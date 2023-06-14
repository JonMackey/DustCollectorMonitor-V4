/*
*	DustCollectorSTM32.cpp, Copyright Jonathan Mackey 2020-2023
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
#include "DustCollectorSTM32.h"
#include "BMP280SPI.h"
#include "SdFat.h"

XFont	xFont;
#if 0
// Using 1-bit fonts saves about 11.5KB over 8-bit. (MyriadPro 7.7KB + Avenir 3.8KB)
// 1-bit draws slightly faster.
// Use 1-bit when developing, uploads are faster.
#define UI20ptFont	MyriadPro_Regular_20_1b::font
#include "MyriadPro-Regular_20_1b.h"
#define UI64ptFont	Avenir_64_1b::font
#include "Avenir_64_1b.h"
#else
// 8-bit fonts (antialiased)
#define UI20ptFont	MyriadPro_Regular_20::font
#include "MyriadPro-Regular_20.h"
#define UI64ptFont	Avenir_64::font
#include "Avenir_64.h"
#endif
#include "DC_Icons.h"
#include "DCSettings.h"
#include "DCXViews.h"

void ButtonISR(void);

bool DustCollectorSTM32::sButtonPressed;
static const char kDCSettingsPath[] = "DCSettings.txt";

/***************************** DustCollectorSTM32 *****************************/
DustCollectorSTM32::DustCollectorSTM32(void)
  : DustCollectorBase(Config::kBMP1CSPin, Config::kBMP0CSPin,
    Config::kFlasherControlPin, Config::kMotorControlPin, Config::kMotorSensePin),
	mDisplay(Config::kDispDCPin, Config::kDispResetPin,
						Config::kDispCSPin, Config::kBacklightPin),
	mPreferences(Config::kAT24CDeviceAddr, Config::kAT24CDeviceCapacity),
    mTouchScreen(Config::kTouchCSPin, Config::kTouchIRQPin,
			Config::kDisplayHeight, Config::kDisplayWidth,
			0, 0, 0, 0, Config::kInvertTouchX),
	mDebouncePeriod(DEBOUNCE_DELAY),
	mStartsPerHourHead(0), mStartsPerHourTail(0)
{
}

/************************************ begin ***********************************/
void DustCollectorSTM32::begin(void)
{
	DustCollectorBase::begin();
	
	pinMode(Config::kUpBtnPin, INPUT_PULLUP);
	pinMode(Config::kLeftBtnPin, INPUT_PULLUP);
	pinMode(Config::kEnterBtnPin, INPUT_PULLUP);
	pinMode(Config::kRightBtnPin, INPUT_PULLUP);
	pinMode(Config::kDownBtnPin, INPUT_PULLUP);

	// Define button pin interrupt routine
	// The buttons are used to setup the touchscreen.
	attachInterrupt(digitalPinToInterrupt(Config::kUpBtnPin), ButtonISR, FALLING);
	attachInterrupt(digitalPinToInterrupt(Config::kLeftBtnPin), ButtonISR, FALLING);
	attachInterrupt(digitalPinToInterrupt(Config::kEnterBtnPin), ButtonISR, FALLING);
	attachInterrupt(digitalPinToInterrupt(Config::kRightBtnPin), ButtonISR, FALLING);
	attachInterrupt(digitalPinToInterrupt(Config::kDownBtnPin), ButtonISR, FALLING);

	STM32UnixRTC::RTCInit();

	pinMode(Config::kSDDetectPin, INPUT_PULLUP);
	pinMode(Config::kSDSelectPin, OUTPUT);
	digitalWrite(Config::kSDSelectPin, HIGH);	// Deselect the SD card.
	
	/*
	*	Load the preferences...
	*/
	Config::SDCPreferences	prefs;
	bool	prefsRead = mPreferences.Read(0, sizeof(Config::SDCPreferences), &prefs.unused[0]);
	if (prefsRead)
	{
		UnixTime::SetFormat24Hour(prefs.clockFormat == 0);
		ValueFormatter::sTemperatureUnit = (ValueFormatter::ETemperatureUnit)prefs.tempUnit;
		ValueFormatter::sPressureUnit = (ValueFormatter::EPressureUnit)prefs.presUnit;
		filterPresValueField.SetVisible(prefs.displayPressure);
		mMotorEnabled = prefs.binMotorEnabled;
		binMotorValueField.OverrideValueString(mMotorEnabled ? kStoppedStr : kDisabledStr, false);

		/*
		*	Initialize the touchscreen alignment values
		*/
		mTouchScreen.SetMinMax(prefs.tsMinMax);

		mCleanPressure = prefs.cleanPressure;
		if (mCleanPressure < 100 ||
			mCleanPressure > 600)
		{
			mCleanPressure = 100;
		}
		
		mDirtyPressure = prefs.dirtyPressure;
		if (mDirtyPressure < mCleanPressure ||
			mDirtyPressure > 1000)
		{
			mDirtyPressure = 800;	// Default to ~3.2" of water or 8 hPa
		}
		
		/*
		*	Sanity check the threshold value.
		*	This value will be out of range when the EEPROM is erased and/or never
		*	been accessed.
		*/
		mTriggerThreshold = prefs.binThreshold;
		if (mTriggerThreshold > kThresholdUpperLimit ||
			mTriggerThreshold < kThresholdLowerLimit)
		{
			mTriggerThreshold = kDefaultTriggerThreshold;
		}
	}
	
	filterStatusGauge.SetMinMax(mCleanPressure, mDirtyPressure);
	mTouchScreen.begin(Config::kDisplayRotation);
	mDisplay.begin(Config::kDisplayRotation);	// Init TFT
	
	filterSettingsDialog.SetValidatorDelegate(this);
	filterPresValueField.SetHeight(20);	// Has no stepper to assign the height.
	rootView.SetSize(Config::kDisplayHeight, Config::kDisplayWidth);
	rootView.SetDisplay(&mDisplay);
	rootView.SetModalView(&mainMenuBtn);
	rootView.SetViewChangedDelegate(this);
	warningDialog.SetViewChangedDelegate(this);
	warningDialog.SetMinDialogSize();
	xFont.SetDisplay(&mDisplay, &UI20ptFont);	// To initialize mDisplay of xFont
	
	if (prefs.showInfoViewOnStartup)
	{
		ShowInfoView(false);
	} else
	{
		ShowFilterStatusGauge(false);
	}
}

/************************ InitializeMotorThresholdVars ************************/
void DustCollectorSTM32::InitializeMotorThresholdVars(void)
{
	// Legacy routine.  The threshold is set in begin() above.
}

/**************************** CurrentDirtyPressure ****************************/
int32_t DustCollectorSTM32::CurrentDirtyPressure(void)
{
	return(mDirtyPressure);
}

/**************************** SaveTriggerThreshold ****************************/
void DustCollectorSTM32::SaveTriggerThreshold(void)
{
	mPreferences.Write(offsetof(Config::SDCPreferences, binThreshold), 1, &mTriggerThreshold);
}

/*************************** NoModalDialogDisplayed ***************************/
bool DustCollectorSTM32::NoModalDialogDisplayed(void) const
{
	return(rootView.ModalView() == &mainMenuBtn);
}

/*********************************** Update ***********************************/
/*
*	Called from loop()
*/
bool DustCollectorSTM32::Update(void)
{
	if (mTouchScreen.PenStateChanged())
	{
		if (mTouchScreen.PenIsDown())
		{
			if (!mDisplaySleeping)
			{
			//	Serial.println('D');
				UnixTime::ResetSleepTime();
				uint16_t	z;
				mTouchScreen.Read(mX, mY, z);
				mHitView = rootView.HitTest(mX, mY);
				if (mHitView)
				{
					mHitView->MouseDown(mX,mY);
				}
			} else
			{
				WakeUp();
			}
			
		} else	// Else, pen is up
		{
		//	Serial.println('U');
			if (mHitView)
			{
				mHitView->MouseUp(mX, mY);
				mHitView = nullptr;
			}
		}
	}

	DustCollectorBase::Update();
	CheckButtons();	// Buttons are currently only used to setup the screen.
	
	/*
	*	If the display isn't sleeping...
	*/
	if (!mDisplaySleeping)
	{
		bool	noModalDialogDisplayed = NoModalDialogDisplayed();
		/*
		*	If there are no modal dialogs visible THEN
		*	update either the filter status gauge or the info view.
		*/
		if (noModalDialogDisplayed &&
			!mainMenu.IsVisible())
		{
			int32_t	adjustedDeltaAverage = DeltaAveragesLoaded() ? AdjustedDeltaAverage() : 0;
			filterStatusGauge.SetValue(abs(adjustedDeltaAverage));
			if (filterStatusGauge.IsVisible())
			{
				filterStatusGauge.Update();
				if (DeltaAveragesLoaded())
				{
					filterPresValueField.SetValue(adjustedDeltaAverage);
				}
			} else
			{
				UpdateInfoView();
			}
		}
		dcStatusIcon.Update();
		UpdateInfoViewBinMotor();
		
		if (UnixTime::TimeToSleep())
		{
			if (noModalDialogDisplayed &&
				!mDCIsRunning)
			{
				GoToSleep();
			} else
			{
				UnixTime::ResetSleepTime();
			}
		}
	} else if (mDCIsRunning)
	{
		WakeUp();
	}

#if 1
	/*
	*	Serial commands
	*/
	if (Serial.available())
	{
		switch (Serial.read())
		{
			case '>':	// Set the time.  A hexadecimal ASCII UNIX time follows
						// Use >65920071 for all-fields-change test (15s delay)
				UnixTime::SetUnixTimeFromSerial();
				STM32UnixRTC::SyncRTCToTime();
				break;
		}
	}
#endif	

	return(false);
}

/*********************************** WakeUp ***********************************/
/*
*	Wakup the display from sleep and/or keep it awake if not sleeping.
*/
void DustCollectorSTM32::WakeUp(void)
{
	if (mDisplaySleeping)
	{
		mDisplaySleeping = false;
		mDisplay.WakeUp();
		rootView.Draw(0, 0, 999, 999);
	}
	UnixTime::ResetSleepTime();
}

/********************************* GoToSleep **********************************/
/*
*	Puts the display to sleep.
*/
void DustCollectorSTM32::GoToSleep(void)
{
	if (!mDisplaySleeping)
	{
		mDisplay.Fill();
		mDisplay.Sleep();
		mDisplaySleeping = true;
	}
}

/******************************** CheckButtons ********************************/
/*
*	CheckButtons is used to setup the display orientation and the touch screen
*	alignment values. 
*/
void DustCollectorSTM32::CheckButtons(void)
{
	if (sButtonPressed)
	{
		uint32_t	pinsState = (~GPIOA->IDR) & Config::kPINAtnMask;
		if (pinsState)
		{
			/*
			*	If debounced
			*/
			if (mStartPinState == pinsState)
			{
				/*
				*	Wakeup the display when any key is pressed.
				*/
				WakeUp();
				/*
				*	If a debounce period has passed
				*/
				if (mDebouncePeriod.Passed())
				{
					UnixTime::ResetSleepTime();
					sButtonPressed = false;
					mStartPinState = 0xFF;
					switch (pinsState)
					{
						case Config::kUpBtn:	// Up button pressed
							//Serial.println('U');
							if (touchScreenAlignment.IsVisible())
							{
								// Stop and cancel/quit (restore old alignment)
								Serial.println("Cancel Align");
								infoView.SetVisible(true);
								filterStatusGauge.SetVisible(false);
								touchScreenAlignment.Stop(true);
							}
							break;
						case Config::kEnterBtn:	// Enter button pressed
							//Serial.println('E');
							if (touchScreenAlignment.IsVisible())
							{
								if (touchScreenAlignment.OKToSave())
								{
									// Stop and save
									infoView.SetVisible(true);
									filterStatusGauge.SetVisible(false);
									touchScreenAlignment.Stop(false);
									uint16_t	minMax[4];
									mTouchScreen.GetMinMax(minMax);
									mPreferences.Write(offsetof(Config::SDCPreferences, tsMinMax), sizeof(minMax), (const uint8_t*)minMax);
								}
							} else if (NoModalDialogDisplayed() &&
								!mDCIsRunning)
							{
								infoView.SetVisible(false);
								filterStatusGauge.SetVisible(false);
								touchScreenAlignment.Start(&mTouchScreen);
							}
							break;
					#if 0
						case Config::kLeftBtn:	// Left button pressed
							break;
						case Config::kDownBtn:	// Down button pressed
							if (touchScreenAlignment.IsVisible())
							{
								touchScreenAlignment.ToggleInvertX();
							}
							break;
						case Config::kRightBtn:	// Right button pressed
							if (touchScreenAlignment.IsVisible())
							{
								touchScreenAlignment.ToggleInvertY();
							}
							break;
					#else
						case Config::kLeftBtn:	// Left button pressed
							break;
						case Config::kDownBtn:	// Down button pressed
						{
					#if 0
							Serial.print("ModalView = ");
							if (rootView.ModalView())
							{
								Serial.println(rootView.ModalView()->Tag());
							} else
							{
								Serial.println("nullptr");
							}
					#endif
							break;
						}
						case Config::kRightBtn:	// Right button pressed
							break;
					#endif
						default:
							mDebouncePeriod.Start();
							break;
					}
				}
			} else
			{
				mStartPinState = pinsState;
				mDebouncePeriod.Start();
			}
		}
	}
}

/******************************* UpdateInfoView *******************************/
/*
*	Called from Update() when no modal dialogs are displayed.
*/
void DustCollectorSTM32::UpdateInfoView(void)
{
	if (infoView.IsVisible())
	{
		if (UnixTime::TimeChanged())
		{
			UnixTime::ResetTimeChanged();
			infoDateValueField.SetValue(UnixTime::Time());
			temperatureValueField.SetValue(mAmbientTemperature);
			ductPresValueField.SetValue(mDuctPressure);
			ambientPresValueField.SetValue(mAmbientPressure);
			if (DeltaAveragesLoaded())
			{
				basePresValueField.SetValue(Baseline());
				staticPresValueField.SetValue(AdjustedDeltaAverage());
			}
			// Remove any entries in the mStartsPerHourRingBuffer that are more
			// than 1 hour old.
			UpdateStartsPerHour(UnixTime::Time());
			startsPerHourValueField.SetValue(StartsPerHour());
		}
	}
}

/*************************** UpdateInfoViewBinMotor ***************************/
/*
*	This was separated from UpdateInfoView to be able to see the bin motor
*	value when the utilities dialog is showing.
*/
void DustCollectorSTM32::UpdateInfoViewBinMotor(void)
{
	if (infoView.IsVisible() &&
		filterSettingsDialog.IsVisible() == false)
	{
		/*
		*	If the motor is running THEN
		*	update the bin motor value as it changes, not every second.
		*/
		if (mMotorSensePeriod.Get() &&
			(uint32_t)mBinMotorAverage != binMotorValueField.Value())
		{
			binMotorValueField.SetValue(mBinMotorAverage);
		}
	}
}

/******************************** StartsPerHour *******************************/

uint32_t DustCollectorSTM32::StartsPerHour(void) const
{
	return((mStartsPerHourTail + eMaxStartsPerHour - mStartsPerHourHead) % eMaxStartsPerHour);
}

/**************************** AddStartToRingBuffer ****************************/
void DustCollectorSTM32::AddStartToRingBuffer(
	time32_t	inStartTime)
{
	// inStartTime expires in one hour (+3600)
	mStartsPerHourRingBuffer[mStartsPerHourTail] = inStartTime+3600;
	mStartsPerHourTail = (mStartsPerHourTail + 1) % eMaxStartsPerHour;
	/*
	*	If the tail caught up with the head THEN
	*	trash the oldest entry.
	*/
	if (mStartsPerHourTail == mStartsPerHourHead)
	{
		mStartsPerHourHead = (mStartsPerHourHead + 1) % eMaxStartsPerHour;
	}
}

/**************************** UpdateStartsPerHour *****************************/
void DustCollectorSTM32::UpdateStartsPerHour(
	time32_t	inTime)
{
	while (mStartsPerHourHead != mStartsPerHourTail &&
		mStartsPerHourRingBuffer[mStartsPerHourHead] < inTime)
	{
		mStartsPerHourHead = (mStartsPerHourHead + 1) % eMaxStartsPerHour;
	}
}

/************************** DustCollectorJustStarted **************************/
void DustCollectorSTM32::DustCollectorJustStarted(void)
{
	DustCollectorBase::DustCollectorJustStarted();
	dcStatusIcon.SetAnimationPeriod(750);
	AddStartToRingBuffer(UnixTime::Time());
}

/************************** DustCollectorJustStopped **************************/
void DustCollectorSTM32::DustCollectorJustStopped(void)
{
	DustCollectorBase::DustCollectorJustStopped();
	dcStatusIcon.SetAnimationPeriod(0);
	if (!mDeltaAveragesLoaded)
	{
		staticPresValueField.OverrideValueString(kStoppingStr, infoView.IsVisible() && filterSettingsDialog.IsVisible() == false);
		filterPresValueField.OverrideValueString(kStoppingStr, filterStatusGauge.IsVisible());
	}
}

/******************************** LoadingDeltas *******************************/
void DustCollectorSTM32::LoadingDeltas(void)
{
	staticPresValueField.OverrideValueString(kWaitStr, infoView.IsVisible() && filterSettingsDialog.IsVisible() == false);
	filterPresValueField.OverrideValueString(kWaitStr, filterStatusGauge.IsVisible());
}

/******************************** CancelFault *********************************/
void DustCollectorSTM32::CancelFault(void)
{
	DustCollectorBase::CancelFault();
	warningDialog.DoCancel();
}

/********************************* FilterFull *********************************/
void DustCollectorSTM32::FilterFull(void)
{
	DustCollectorBase::FilterFull();
	/*
	*	If no modal dialogs are showing THEN
	*	display the "filter loaded" message.
	*/
	if (NoModalDialogDisplayed())
	{
		warningDialog.DoMessage(kFilterLoadedStr, kFilterLoadedMessageTag);
	}
}

/******************************** DustBinFull *********************************/
void DustCollectorSTM32::DustBinFull(void)
{
	//Serial.print("mBinMotorAverage = ");
	//Serial.println(mBinMotorAverage);
	DustCollectorBase::DustBinFull();
	if (NoModalDialogDisplayed() ||
		utilitiesDialog.IsVisible())
	{
		warningDialog.DoMessage(kDustBinFullStr, kDustBinFullMessageTag);
	}
}

/****************************** StopDustBinMotor ******************************/
void DustCollectorSTM32::StopDustBinMotor(void)
{
	DustCollectorBase::StopDustBinMotor();
	binMotorValueField.OverrideValueString(kStoppedStr, infoView.IsVisible() && filterSettingsDialog.IsVisible() == false);
}

/********************************** ButtonISR *********************************/
void ButtonISR(void)
{
	DustCollectorSTM32::sButtonPressed = true;
}

/******************************* ValuesAreValid *******************************/
/*
*	ValuesAreValid is a member of the XValidatorDelegate mixin class.
*	It's called when a dialog's OK button is pressed.
*	True is returned if the values are valid and the dialog can be dismissed,
*	otherwise the dialog either just stays visible (no action) or the dialog
*	displays an error explaining why the dialog can't be dismissed.
*/
bool DustCollectorSTM32::ValuesAreValid(
	XDialogBox*	inDialog)
{
	bool	valuesAreValid = false;
	if (inDialog)
	{
		switch (inDialog->Tag())
		{
			case kFilterSettingsDialogTag:
				valuesAreValid = cleanPresValueField.Value() < dirtyPresValueField.Value();
				if (!valuesAreValid)
				{
					warningDialog.DoMessage(kCleanPresInvalidMessageStr);
				}
				break;
		}
	}
	return(valuesAreValid);
}

/****************************** HandleViewChange ******************************/
void DustCollectorSTM32::HandleViewChange(
	XView*		inView,
	uint16_t	inAction)
{
	if (inView)
	{
	#if 0
		Serial.print("Change: Tag = ");
		Serial.print(inView->Tag());
		Serial.print(", Action = ");
		Serial.println(inAction);
	#endif
		switch (inView->Tag())
		{
			case kMainMenuTag:
				switch (inAction)
				{
					case kInfoMenuItem:
						ShowInfoView();
						break;
					case kFilterStatusMenuItem:
						ShowFilterStatusGauge();
						break;
					case kFilterSettingsMenuItem:
						ShowFilterSettingsDialog();
						break;
					case kBinSettingsMenuItem:
						ShowBinSettingsDialog();
						break;
					case kSetClockMenuItem:
						ShowSetClockDialog();
						break;
					case kUtilitiesMenuItem:
						ShowUtilitiesDialog();
						break;
					case kAboutMenuItem:
						aboutBox.Show();
						break;
				}
				break;
			case kSetCleanPresBtnTag:
				if (inAction == 0)
				{
					if (mDCIsRunning)
					{
						alertDialog.DoMessage(kUseCurrentPresMessageStr, kSetCleanPresBtnTag);
					} else
					{
						warningDialog.DoMessage(kDCMustBeRunningMessageStr);
					}
				}
				break;
			case kSetDirtyPresBtnTag:
				if (inAction == 0)
				{
					if (mDCIsRunning)
					{
						alertDialog.DoMessage(kUseCurrentPresMessageStr2, kSetDirtyPresBtnTag);
					} else
					{
						warningDialog.DoMessage(kDCMustBeRunningMessageStr);
					}
				}
				break;
			case kPresUnitMenuTag:
			{	// Update the pressure units value strings if a new unit
				// has been selected.
				ValueFormatter::EPressureUnit prevUnit = ValueFormatter::sPressureUnit;
				ValueFormatter::sPressureUnit = (ValueFormatter::EPressureUnit)(inAction == kInchesMenuItem);
				if (prevUnit != ValueFormatter::sPressureUnit)
				{
					dirtyPresValueField.ValueChanged();
					cleanPresValueField.ValueChanged();
				}
				break;
			}
			case kFilterSettingsDialogTag+XDialogBox::eCancelTagOffset:
			{
				Config::SDCPreferences	prefs;
				// Revert back to original pressure unit when Cancel is selected.
				if (mPreferences.Read(0, 3, &prefs.unused[0]))
				{
					ValueFormatter::sPressureUnit = (ValueFormatter::EPressureUnit)prefs.presUnit;
				}
				break;
			}
			case kFilterSettingsDialogTag+XDialogBox::eOKTagOffset:
			{
				if (inAction == 1)
				{
					SaveFilterSettingsDialogChanges();
				}
				break;
			}
			case kBinSettingsDialogTag+XDialogBox::eOKTagOffset:
			{
				if (inAction == 1)
				{
					SaveBinSettingsDialogChanges();
				}
				break;
			}
			case kSetClockDialogTag+XDialogBox::eOKTagOffset:
			{
				if (inAction == 1)
				{
					UnixTime::SetTime(dateValueField.Value());
					STM32UnixRTC::SyncRTCToTime();
				}
				break;
			}
			case kWarningDialogTag+XDialogBox::eOKTagOffset:
			{
				if (warningDialog.MessageTag() == kFilterLoadedMessageTag ||
					warningDialog.MessageTag() == kDustBinFullMessageTag)
				{
					DustCollectorBase::CancelFault();
					if (utilitiesDialog.IsVisible() &&
						motorStartStopBtn.IsEnabled())
					{
						motorStartStopBtn.SetString(kStartStr, false);
					}
					if (!mDCIsRunning)
					{
						dcStatusIcon.SetAnimationPeriod(0);
					}
				}
				break;
			}
			case kAlertDialogTag+XDialogBox::eOKTagOffset:
				switch(alertDialog.MessageTag())
				{
					case kLoadSettingsBtnTag:
						LoadDCSettingsFromSD();
						break;
					case kSetCleanPresBtnTag:
						cleanPresValueField.SetValue(AdjustedDeltaAverage());
						break;
					case kSetDirtyPresBtnTag:
						dirtyPresValueField.SetValue(AdjustedDeltaAverage());
						break;
				}
				break;
			case kMotorStartStopBtnTag:
			{
				if (inAction == 1)
				{
					if (mMotorSensePeriod.Get())
					{
						DustCollectorJustStopped();
						motorStartStopBtn.SetString(kStartStr);
					} else
					{
						DustCollectorJustStarted();
						motorStartStopBtn.SetString(kStopStr);
					}
				}
				break;
			}
			case kSaveSettingsBtnTag:
				if (inAction == 0)
				{
					SaveDCSettingsToSD();
				}
				break;
			case kLoadSettingsBtnTag:
				if (inAction == 0)
				{
					if (digitalRead(Config::kSDDetectPin))
					{
						warningDialog.DoMessage(kNoSDCardFoundStr);
					} else
					{
						alertDialog.DoMessage(kLoadDCSettingsStr, kLoadSettingsBtnTag);
					}
				}
				break;			
		}
	}
}

/************************** UpdateInfoPressureValues **************************/
void DustCollectorSTM32::UpdateInfoPressureValues(void)
{
	ductPresValueField.ValueChanged(false);
	ambientPresValueField.ValueChanged(false);
	basePresValueField.ValueChanged(false);
	if (DeltaAveragesLoaded())
	{
		staticPresValueField.ValueChanged(false);
	}
}

/******************************** ShowInfoView ********************************/
void DustCollectorSTM32::ShowInfoView(
	bool	inUpdatePref)
{
	if (!infoView.IsVisible())
	{
		infoView.SetVisible(true);
		filterStatusGauge.SetVisible(false);
		rootView.Draw(0, 0, 480, 320);
		if (inUpdatePref)
		{
			UpdateShowInfoViewrOnStartupPref();
		}
	}
}

/**************************** ShowFilterStatusGauge ***************************/
void DustCollectorSTM32::ShowFilterStatusGauge(
	bool	inUpdatePref)
{
	if (!filterStatusGauge.IsVisible())
	{
		infoView.SetVisible(false);
		filterStatusGauge.SetVisible(true);
		rootView.Draw(0, 0, 480, 320);
		if (inUpdatePref)
		{
			UpdateShowInfoViewrOnStartupPref();
		}
	}
}

/********************** UpdateShowInfoViewrOnStartupPref **********************/
void DustCollectorSTM32::UpdateShowInfoViewrOnStartupPref(void)
{
	uint8_t	infoViewIsVisible = infoView.IsVisible();
	mPreferences.Write(offsetof(Config::SDCPreferences, showInfoViewOnStartup), 1, &infoViewIsVisible);
}

/************************** ShowFilterSettingsDialog **************************/
void DustCollectorSTM32::ShowFilterSettingsDialog(void)
{
	presUnitPopUp.SelectMenuItem(
		ValueFormatter::sPressureUnit == ValueFormatter::eHectopascal ?
			kHPaMenuItem : kInchesMenuItem);
	dirtyPresValueField.SetValue(mDirtyPressure, false);
	cleanPresValueField.SetValue(mCleanPressure, false);
	dirtyPresValueField.ValueChanged(false);
	cleanPresValueField.ValueChanged(false);
	dispPresCheckbox.SetState(filterPresValueField.IsVisible() ? XControl::eOn : XControl::eOff, false);
	filterSettingsDialog.Show();
}

/********************** SaveFilterSettingsDialogChanges ***********************/
void DustCollectorSTM32::SaveFilterSettingsDialogChanges(void)
{
	/*
	*	If it gets here then ValuesAreValid() has been called
	*	(and passed) for this dialog.
	*/
	uint8_t showPressure = dispPresCheckbox.GetState();
	uint8_t	prefShowPressure;
	if (mPreferences.Read(offsetof(Config::SDCPreferences, displayPressure), 1, &prefShowPressure) &&
		prefShowPressure != showPressure)
	{
		mPreferences.Write(offsetof(Config::SDCPreferences, displayPressure), 1, &showPressure);
		if (filterStatusGauge.IsVisible())
		{
			if (showPressure)
			{
				filterPresValueField.Show();
			} else
			{
				filterPresValueField.Hide();
			}
		} else
		{
			filterPresValueField.SetVisible(showPressure != 0);
		}
	}

	{
		Config::SDCPreferences	prefs;
		if (mPreferences.Read(0, 3, &prefs.unused[0]))
		{
			if (prefs.presUnit != ValueFormatter::sPressureUnit)
			{
				prefs.presUnit = ValueFormatter::sPressureUnit;
				mPreferences.Write(0, 3, &prefs.unused[0]);
				UpdateInfoPressureValues();
				if (DeltaAveragesLoaded())
				{
					filterPresValueField.ValueChanged(filterStatusGauge.IsVisible());
				}
			}
		}
	}
	mDirtyPressure = dirtyPresValueField.Value();
	mCleanPressure = cleanPresValueField.Value();
	mPreferences.Write(offsetof(Config::SDCPreferences, dirtyPressure), 4, (uint8_t*)&mDirtyPressure);
	mPreferences.Write(offsetof(Config::SDCPreferences, cleanPressure), 4, (uint8_t*)&mCleanPressure);
	filterStatusGauge.SetMinMax(mCleanPressure, mDirtyPressure);
}

/**************************** ShowBinSettingsDialog ***************************/
void DustCollectorSTM32::ShowBinSettingsDialog(void)
{
	disableMotorCheckbox.SetState(mMotorEnabled ? XControl::eOff : XControl::eOn, false);
	binWrnThresValueField.SetValue(mTriggerThreshold, false);
	binSettingsDialog.Show();
}

/**************************** ShowUtilitiesDialog *****************************/
void DustCollectorSTM32::ShowUtilitiesDialog(void)
{
	motorStartStopBtn.SetString(mMotorSensePeriod.Get() ? kStopStr : kStartStr);
	motorStartStopBtn.Enable(mMotorEnabled, false);
	utilitiesDialog.Show();
}

/************************ SaveBinSettingsDialogChanges ************************/
void DustCollectorSTM32::SaveBinSettingsDialogChanges(void)
{
	bool	newMotorEnabled = disableMotorCheckbox.GetState() == XControl::eOff;
	if (newMotorEnabled != mMotorEnabled)
	{
		mMotorEnabled = newMotorEnabled;
		mPreferences.Write(offsetof(Config::SDCPreferences, binMotorEnabled), 1, (uint8_t*)&mMotorEnabled);
		if (!mMotorEnabled)
		{
			binMotorValueField.SetValue(0, false);
			binMotorValueField.OverrideValueString(kDisabledStr, infoView.IsVisible());
			StopDustBinMotor();
		} else
		{
			if (mDCIsRunning)
			{
				StartDustBinMotor();
			}
		}
	}
	mTriggerThreshold = binWrnThresValueField.Value();
	SaveTriggerThreshold();
}

/***************************** ShowSetClockDialog *****************************/
void DustCollectorSTM32::ShowSetClockDialog(void)
{
	dateValueField.SetValue(UnixTime::Time(), false);
	setClockDialog.Show();
}

/***************************** SaveDCSettingsToSD *****************************/
void DustCollectorSTM32::SaveDCSettingsToSD(void)
{
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		DCSettings	dcSettings;
		SDCSettings	settings;
		settings.hourFormat = UnixTime::Format24Hour() ? 24:12,
		settings.temperatureUnitCelsius = ValueFormatter::sTemperatureUnit == ValueFormatter::eCelsius,
		settings.pressureUnit_hPa = ValueFormatter::sPressureUnit == ValueFormatter::eHectopascal,
		settings.binThreshold = mTriggerThreshold,
		settings.binMotorEnabled = mMotorEnabled,
		settings.displayPressure = filterPresValueField.IsVisible(),
		settings.currentView = infoView.IsVisible() ? kInfoViewTag : kFilterStatusGaugeTag,
		mTouchScreen.GetMinMax(settings.tsMinMax);
		settings.cleanPressure = mCleanPressure;
		settings.dirtyPressure = mDirtyPressure;

		SdFat sd;
		bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			success = dcSettings.WriteFile(kDCSettingsPath, settings);
		} else
		{
			sd.initErrorHalt();
		}
		warningDialog.DoMessage(success ? kSavedDCSettingsStr : kSaveToSDFailedStr);
	} else
	{
		warningDialog.DoMessage(kNoSDCardFoundStr);
	}
}

/**************************** LoadDCSettingsFromSD ****************************/
void DustCollectorSTM32::LoadDCSettingsFromSD(void)
{
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		SdFat sd;
		DCSettings	dcSettings;
		bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			success = dcSettings.ReadFile(kDCSettingsPath);
		} else
		{
			sd.initErrorHalt();
		}
		if (success)
		{
			const SDCSettings&	settings = dcSettings.Settings();
			Config::SDCPreferences	prefs = {0};
			prefs.clockFormat = settings.hourFormat == 12;
			prefs.tempUnit = settings.temperatureUnitCelsius == false;
			prefs.presUnit = settings.pressureUnit_hPa == false;
			prefs.binThreshold = settings.binThreshold;
			prefs.binMotorEnabled = settings.binMotorEnabled;
			prefs.displayPressure = settings.displayPressure;
			prefs.showInfoViewOnStartup = settings.currentView == kInfoViewTag;
			memcpy(prefs.tsMinMax, settings.tsMinMax, sizeof(prefs.tsMinMax));
			prefs.cleanPressure = mCleanPressure;
			prefs.dirtyPressure = mDirtyPressure;
			mPreferences.Write(0, sizeof(Config::SDCPreferences), &prefs.unused[0]);
			/*
			*	Restart the board.
			*/
			HAL_NVIC_SystemReset();
		} else
		{
			warningDialog.DoMessage(kLoadFromSDFailedStr);
		}
	} else
	{
		warningDialog.DoMessage(kNoSDCardFoundStr);
	}
}