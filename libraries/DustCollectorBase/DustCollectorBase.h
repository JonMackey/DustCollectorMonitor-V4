/*
*	DustCollectorBase.h, Copyright Jonathan Mackey 2023
*	- Monitors the dust collector to determine when the drum or filter is full.
*	- Controls the drum motor and warning flasher.
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
#ifndef DustCollectorBase_h
#define DustCollectorBase_h

#include <inttypes.h>
#include "MSPeriod.h"
#include "BMP280SPI.h"
#include "PlatformDefs.h"

class DustCollectorBase
{
public:
							DustCollectorBase(
								pin_t					inBMP280AmbientPin,
								pin_t					inBMP280DuctPin,
								pin_t					inFlasherControlPin,
								pin_t					inMotorControlPin,
								pin_t					inMotorSensePin);
		
	virtual void			begin(void);

	enum EStatus
	{
		eNotRunning,
		eRunning,
		eBinFull,
		eFilterFull
	};

	enum EConfig
	{
		eNumDeltas = 4,		// Number of deltas contained in mDeltaSum.
		eNumDeltaAvgs = 8,	// Number of Delta Averages representing
							// averages over the period eNumDeltaAvgs*kPressureUpdatePeriod
		eBinMotorSampleSize = 8
	};
	
	uint8_t					Status(void) const
								{return(mStatus);}
	virtual void			CancelFault(void);
	int32_t					AmbientTemperature(void) const
								{return(mAmbientTemperature);}
	uint32_t				AmbientPressure(void) const
								{return(mAmbientPressure);}
	uint32_t				DuctPressure(void) const
								{return(mDuctPressure);}
	inline int32_t			DeltaAverage(void) const
								{return(mDeltaSum/eNumDeltas);}
	bool					DeltaAveragesLoaded(void) const
								{return(mDeltaAveragesLoaded);}
	bool					DCIsRunning(void) const
								{return(mDCIsRunning);}
	bool					BinMotorIsRunning(void) const
								{return(mMotorSensePeriod.Get() != 0);}
	virtual void			ToggleBinMotor(void); // Start/Stop motor from UI
	uint8_t					GetBinMotorReading(void) const
								{return(mBinMotorAverage);}
	int32_t					Baseline(void) const; // Returns the oldest average
	inline int32_t			AdjustedDeltaAverage(void) const
								{return(DeltaAverage() - Baseline());}
	virtual bool			Update(void);
	virtual void			DustBinFull(void);
	virtual void			FilterFull(void);
	uint8_t					GetTriggerThreshold(void) const
								{return(mTriggerThreshold);}
	void					SetTriggerThreshold(
								uint8_t					inTriggerThreshold);
	virtual void			SaveTriggerThreshold(void) = 0;
	virtual void			StartFlasher(void);
	virtual void			StopFlasher(void);
								
	// Dust filter
	static const uint32_t	kPressureUpdatePeriod;	// in milliseconds
	
	// Dust bin motor
	static const uint32_t	kMotorSensePeriod;	// in milliseconds
	static const uint8_t	kThresholdLowerLimit;
	static const uint8_t	kDefaultTriggerThreshold;
	static const uint8_t	kThresholdUpperLimit;
	
protected:
	uint8_t		mStatus;
	pin_t		mFlasherControlPin;
	pin_t		mMotorControlPin;
	pin_t		mMotorSensePin;
	MSPeriod	mPressureUpdatePeriod;
	int32_t		mDeltaSum;
	int32_t		mDelta[eNumDeltas];
	int16_t		mDeltaAverage[eNumDeltaAvgs];
	uint8_t		mDeltaIndex;
	uint8_t		mDeltaAverageIndex;
	bool		mDeltaSumLoaded;
	bool		mDeltaAveragesLoaded;
	bool		mDCIsRunning;
	bool		mFaultAcknowledged;
	BMP280SPI	mBMP280Ambient;
	BMP280SPI	mBMP280Duct;
	int32_t		mAmbientTemperature;
	uint32_t	mDuctPressure;
	uint32_t	mAmbientPressure;

	MSPeriod	mMotorSensePeriod;

	uint16_t	mSampleCount;
	uint16_t	mRingBuf[eBinMotorSampleSize];
	uint8_t		mRingBufIndex;
	uint16_t	mSampleAccumulator;

	bool		mMotorEnabled;
	uint8_t		mTriggerThreshold;
	uint8_t		mBinMotorAverage;
	
	void					CheckFilter(void);
	void					CheckDustBinMotor(void);
	virtual void			DustCollectorJustStarted(void);
	virtual void			DustCollectorJustStopped(void);
	void					StartDustBinMotor(void);
	virtual void			StopDustBinMotor(void);
	virtual void			InitializeMotorThresholdVars(void) = 0;
	virtual int32_t			CurrentDirtyPressure(void) = 0;
	virtual void			LoadingDeltas(void){}
};

#endif // DustCollectorBase_h
