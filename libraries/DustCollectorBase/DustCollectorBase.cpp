/*
*	DustCollectorBase.cpp, Copyright Jonathan Mackey 2023
*
*	GNU license:
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*	Please maintain this license information along with authorship and copyright
*	notices in any redistribution of this code.
*
*/
#include <Arduino.h>
#include "DustCollectorBase.h"
#include "BMP280SPI.h"

// Dust filter
const uint32_t	DustCollectorBase::kPressureUpdatePeriod = 1500;	// in milliseconds

// Dust bin motor
const uint32_t	DustCollectorBase::kMotorSensePeriod = 500;	// in milliseconds
const uint8_t	DustCollectorBase::kThresholdLowerLimit = 5;
#ifdef _STM32_DEF_
const uint8_t	DustCollectorBase::kDefaultTriggerThreshold = 100;
const uint8_t	DustCollectorBase::kThresholdUpperLimit = 120;
#else
const uint8_t	DustCollectorBase::kDefaultTriggerThreshold = 15;
const uint8_t	DustCollectorBase::kThresholdUpperLimit = 50;
#endif

/******************************* DustCollectorBase ********************************/
DustCollectorBase::DustCollectorBase(
	pin_t	inBMP280AmbientPin,
	pin_t	inBMP280DuctPin,
	pin_t	inFlasherControlPin,
	pin_t	inMotorControlPin,
	pin_t	inMotorSensePin)
  : mBMP280Ambient(inBMP280AmbientPin), mBMP280Duct(inBMP280DuctPin),
 	mFlasherControlPin(inFlasherControlPin), mMotorControlPin(inMotorControlPin),
 	mMotorSensePin(inMotorSensePin),
	mPressureUpdatePeriod(DustCollectorBase::kPressureUpdatePeriod),
	mMotorSensePeriod(DustCollectorBase::kMotorSensePeriod),
	 mDeltaAveragesLoaded(false), mDeltaAverageIndex(0), mMotorEnabled(true),
	 mDeltaIndex(0), mDeltaSumLoaded(false), mDeltaSum(0)
{
	StopFlasher();
	StopDustBinMotor();	// Stop before setting pinMode
	pinMode(mFlasherControlPin, OUTPUT);
	pinMode(mMotorControlPin, OUTPUT);
}

/*********************************** begin ************************************/
void DustCollectorBase::begin(void)
{	
	/*
	*	BMP280 pressure sensor setup
	*/
	{
		/*
		*	Even though it's not used, turning the temperature oversampling off
		*	causes a larger delta between the pressure readings.  I don't know why.
		*/
		mBMP280Ambient.SetOversampling(1, 3);
		mBMP280Duct.SetOversampling(1, 3);

		/*
		*	The filter coefficient should be left off (default).  DustCollectorBase provides
		*	its own pressure averaging.
		*/
		//mBMP280Ambient.SetFilterCoefficient(3);
		//mBMP280Duct.SetFilterCoefficient(3);

	#if 1
		mBMP280Ambient.begin();
		mBMP280Duct.begin();
	#else
		int8_t status = mBMP280Ambient.begin();
		Serial.print(F("BMP280Ambient status = "));
		Serial.println(status);
		status = mBMP280Duct.begin();
		Serial.print(F("BMP280Duct status = "));
		Serial.println(status);
	#endif
		mPressureUpdatePeriod.Start();	
	}
	
	mFaultAcknowledged = true;
	
	InitializeMotorThresholdVars();
	mStatus = eNotRunning;
}

/******************************** CancelFault *********************************/
void DustCollectorBase::CancelFault(void)
{
	mFaultAcknowledged = true;
	StopFlasher();
}

/******************************** StartFlasher ********************************/
void DustCollectorBase::StartFlasher(void)
{
	digitalWrite(mFlasherControlPin, HIGH);
}

/******************************** StopFlasher *********************************/
void DustCollectorBase::StopFlasher(void)
{
	digitalWrite(mFlasherControlPin, LOW);
}

/************************** DustCollectorJustStarted **************************/
void DustCollectorBase::DustCollectorJustStarted(void)
{
	StartDustBinMotor();
}

/************************** DustCollectorJustStopped **************************/
void DustCollectorBase::DustCollectorJustStopped(void)
{
	StopDustBinMotor();
	CancelFault();
}

/***************************** StartDustBinMotor ******************************/
void DustCollectorBase::StartDustBinMotor(void)
{
	if (mMotorEnabled)
	{
		digitalWrite(mMotorControlPin, HIGH);
		// Give the motor 2 seconds to start before taking any readings.
		mMotorSensePeriod.Set(2000);
		mMotorSensePeriod.Start();
	}
	mSampleCount = 0;
	mSampleAccumulator = 0;
	mRingBufIndex = 0;
	mBinMotorAverage = 0;
}

/****************************** StopDustBinMotor ******************************/
void DustCollectorBase::StopDustBinMotor(void)
{
	// Set the sense period to 0.
	// This will stop sensing the motor.
	mMotorSensePeriod.Set(0);
	mBinMotorAverage = 0;
	digitalWrite(mMotorControlPin, LOW);
}

/******************************* ToggleBinMotor *******************************/
void DustCollectorBase::ToggleBinMotor(void)
{
	StopFlasher();
	if (mMotorSensePeriod.Get())
	{
		StopDustBinMotor();
	} else
	{
		StartDustBinMotor();
	}
}

/**************************** SetTriggerThreshold *****************************/
void DustCollectorBase::SetTriggerThreshold(
	uint8_t	inTriggerThreshold)
{
	mTriggerThreshold = inTriggerThreshold;
}

/**************************** SaveTriggerThreshold ****************************/
//void DustCollectorBase::SaveTriggerThreshold(void)
//{
//	EEPROM.put(Config::kMotorTriggerThresholdAddr, mTriggerThreshold);
//}

/*********************************** Update ***********************************/
/*
*	Called from loop() just after the layout has updated.
*/
bool DustCollectorBase::Update(void)
{
	CheckFilter();
	if (mMotorEnabled)
	{
		CheckDustBinMotor();
	}
	return(true);
}

/********************************** Baseline **********************************/
int32_t DustCollectorBase::Baseline(void) const
{
	return(mDeltaAverage[mDeltaAverageIndex % eNumDeltaAvgs]);
}

/******************************** CheckFilter *********************************/
void DustCollectorBase::CheckFilter(void)
{
	if (mPressureUpdatePeriod.Passed())
	{
		mBMP280Ambient.DoForcedRead(mAmbientTemperature, mAmbientPressure);
		{
			int32_t	temp;
			mBMP280Duct.DoForcedRead(temp, mDuctPressure);
		}
		/*
		*	Calling Set on a MSPeriod isn't generally needed, but because the
		*	mPressureUpdatePeriod is set to a 30 second delay when the collector
		*	stops, it's easier to always reset it to the kPressureUpdatePeriod
		*	on each pass.
		*/
		mPressureUpdatePeriod.Set(kPressureUpdatePeriod);
		mPressureUpdatePeriod.Start();
	
		/*
		*	The mDeltaSum is the sum of the last eNumDeltas deltas.
		*
		*	The delta average is updated every 1.5 seconds.  This is the average
		*	delta between the ambient and duct pressure readings when the dust
		*	collector is off.  Four averages are maintained.  Each is an average
		*	of the delta sum measured 1.5 seconds apart.
		*
		*	The storing of averages stop once the dust collector starts. To
		*	detect when the dust collector starts the current mDeltaSum average
		*	must increase by 25Pa over the oldest stored average.
		*
		*	Note that the adjusted delta average is the delta average minus the
		*	baseline (off state) average between the two pressure sensors. The
		*	adjusted value is used when displaying the value to the user and
		*	when determining when the collector is running.  For comparisons,
		*	like determining when the filter is loaded, the simple delta average
		*	is used because there is no need to subtract the baseline provided
		*	both readings being compared are based on the simple delta average.
		*	If both readings are +10Pa, who cares?  It's only when you need to
		*	display the value that the baseline needs to be subtracted.
		*
		*	Whether to use the adjusted average may become an issue if the
		*	baseline changes dramatically over time.
		*/
		// The expected delta is in the range of an signed 16 bit integer.
		int32_t	thisDelta = (int32_t)mDuctPressure - (int32_t)mAmbientPressure;
		/*
		*	When the pressure sensors start up, the first few deltas can be very
		*	large.  At about the 4th reading the delta value becomes rational
		*	for the expected dust collector off state. (a delta less than 200Pa)
		*/
		if (abs(thisDelta) < 1500)
		{
			/*
			*	Member variables:
			*	- mDelta[] is an array containing the last eNumDeltas delta values.
			*	- mDeltaSum is the sum of the eNumDeltas delta values in mDelta[].  The
			*	mDeltaSum is only valid after mDelta contains all eNumDeltas values.
			*	- mDeltaSumLoaded is a flag indicating that the mDelta array
			*	contains eNumDeltas values.
			*	- mDeltaAverage[] is an array containing the last eNumDeltaAvgs delta
			*	averages over a timespan of eNumDeltaAvgs*kPressureUpdatePeriod milliseconds.
			*	- mDeltaAveragesLoaded is a flag indicating that the
			*	mDeltaAverage array contains eNumDeltaAvgs values.  The mDeltaAverage array
			*	values aren't used to determine if the dust collector is running
			*	till this is true.
			*/
			{
				uint8_t	oldestDeltaIndex = mDeltaIndex % eNumDeltas;
				mDeltaSum = mDeltaSum - mDelta[oldestDeltaIndex] + thisDelta;
				mDelta[oldestDeltaIndex] = thisDelta;
		/*	Serial.print("DA = ");
			Serial.print(thisDelta);
			Serial.print(", DI = ");
			Serial.print(oldestDeltaIndex);
			Serial.print(", Ds = ");
			Serial.println(mDeltaSum);
		*/
			}
			mDeltaIndex++;
			// deltaAverage is the average of the eNumDeltas values in mDelta.
			// This is calculated as mDeltaSum/eNumDeltas
			int32_t	deltaAverage = DeltaAverage();
	
		#ifdef DEBUG_DELTAS
			if (mDebugAverageIndex < 511)
			{
				mDeltaAverageDebug[mDebugAverageIndex] = deltaAverage;
				mDebugAverageIndex++;
				mDeltaAverageDebug[mDebugAverageIndex] = 0;
			}
		#endif
	
			if (mDeltaAveragesLoaded)
			{
				/*
				*	Calculate the adjusted average delta using the oldest delta
				*	average.
				*	(mDeltaAverageIndex % eNumDeltaAvgs) = oldest average
				*/
				int32_t	adjustedDeltaAverage = deltaAverage  -
							mDeltaAverage[mDeltaAverageIndex % eNumDeltaAvgs]; //AdjustedDeltaAverage();
				bool isRunning = adjustedDeltaAverage > 25;	// 100 = 1hPa
				if (isRunning != mDCIsRunning)
				{
					mDCIsRunning = isRunning;
					/*
					*	If the dust collector just started THEN
					*	start the dust bin motor.
					*/
					if (isRunning)
					{
						mStatus = eRunning;
						DustCollectorJustStarted();
					/*
					*	Else the dust collector just stopped.
					*	Reset the delta averages.
					*	It takes about 15 seconds for everything to reload after
					*	pressure measurements resume.
					*/
					} else
					{
						mStatus = eNotRunning;
						mDeltaAveragesLoaded = false;
						mDeltaSumLoaded = false;
						mDeltaIndex = 0;
						mDeltaAverageIndex = 0;
						mDeltaSum = 0;
						/*
						*	Give the collector 30 seconds to stop before
						*	resuming pressure measurements.
						*/
						mPressureUpdatePeriod.Set(30000);
						mPressureUpdatePeriod.Start();
						DustCollectorJustStopped();
					}
				}
			} else if (mDeltaAverageIndex >= eNumDeltaAvgs)
			{
				mDeltaAveragesLoaded = true;
			/*
			*	If the deltas just started loading THEN
			*	Provide notification to the UI 
			*/
			} else if (!mDeltaSumLoaded && mDeltaIndex == 1)
			{
				LoadingDeltas();
			}
			/*
			*	If mDeltaSum contains eNumDeltas...
			*/ 
			if (mDeltaSumLoaded)
			{
				/*
				*	If the dust collector is running THEN
				*	see if the filter is loaded.
				*/
				if (mStatus == eRunning)
				{
					/*
					*	If the adjusted delta average is greater than or equal to the dirty pressure THEN
					*	set the status to filter full, start flasher, send message.
					*/
					if ((deltaAverage - Baseline()) >= CurrentDirtyPressure())
					{
						FilterFull();
					}
				} else if (!mDCIsRunning)
				{
					// Set the oldest average to the newest.
					mDeltaAverage[mDeltaAverageIndex % eNumDeltaAvgs] = (int16_t)deltaAverage;
					mDeltaAverageIndex++;	// The next average is now the oldest
				}
			} else if (mDeltaIndex >= eNumDeltas)
			{
				mDeltaSumLoaded = true;
			}
		}
	}
}

/********************************* FilterFull *********************************/
void DustCollectorBase::FilterFull(void)
{
	mStatus = eFilterFull;
	mFaultAcknowledged = false;
	StartFlasher();
}

/***************************** CheckDustBinMotor ******************************/
void DustCollectorBase::CheckDustBinMotor(void)
{
	/*
	*	If the motor is running AND
	*	its value needs to be read...
	*/
	if (digitalRead(mMotorControlPin) &&
		mMotorSensePeriod.Passed())
	{
	//#ifdef _STM32_DEF_
	#if 0
		uint16_t	reading = adc_read_value(analogInputToPinName(mMotorSensePin), 10);
	#else
		uint16_t	reading = analogRead(mMotorSensePin);
	#endif
		mSampleAccumulator += reading;				// Add the newest reading
		uint16_t	oldestReading = mRingBuf[mRingBufIndex];
		mRingBuf[mRingBufIndex] = reading;			// Save the newest reading
		mRingBufIndex++;
		if (mRingBufIndex >= eBinMotorSampleSize)
		{
			mRingBufIndex = 0;
		}
		/*
		*	If the ring buffer is full THEN
		*	use the paddle motor average voltage drop to determine if the motor
		*	is overloaded due to shavings blocking the paddle path.
		*/
		if (mSampleCount >= eBinMotorSampleSize)
		{
			mSampleAccumulator -= oldestReading;	// Remove the oldest reading
			uint16_t average = mSampleAccumulator/eBinMotorSampleSize;
			mBinMotorAverage = average;
			//Serial.println(reading);
			/*
			*	If the paddle motor doesn't have significant resistance THEN
			*	setup the sense period to check the motor in a few ms.
			*/
			if (average < mTriggerThreshold)
			{
				mMotorSensePeriod.Start();
			/*
			*	Else the bin is considered full.
			*/
			} else
			{
				DustBinFull();
			}
		} else
		{
			if (!mSampleCount)
			{
				mMotorSensePeriod.Set(500);
			}
			mSampleCount++;
			mMotorSensePeriod.Start();
			mBinMotorAverage = eBinMotorSampleSize-mSampleCount;
		}
	}
}

/******************************** DustBinFull *********************************/
void DustCollectorBase::DustBinFull(void)
{
	mStatus = eBinFull;
	mFaultAcknowledged = false;
	StopDustBinMotor();
	StartFlasher();
}

