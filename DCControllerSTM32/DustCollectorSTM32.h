/*
*	DustCollectorSTM32.h, Copyright Jonathan Mackey 2023
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
#ifndef DustCollectorSTM32_h
#define DustCollectorSTM32_h

#include "DustCollectorBase.h"
#include "Config.h"
#include "AT24C.h"
#include "TFT_ILI9488.h"
#include "XPT2046.h"
#include "XDialogBox.h"
#include "MSPeriod.h"
#include "STM32UnixRTC.h"

class DustCollectorSTM32 : public DustCollectorBase,
							public XViewChangedDelegate,
								public XValidatorDelegate
{
public:
							DustCollectorSTM32(void);
		
	virtual void			begin(void);
	virtual void			CancelFault(void);
	virtual void			DustBinFull(void);
	virtual void			FilterFull(void);
	virtual void			SaveTriggerThreshold(void);	// Save to EEPROM
	virtual void			InitializeMotorThresholdVars(void);
	virtual int32_t			CurrentDirtyPressure(void);
	virtual bool			Update(void);

	virtual void			HandleViewChange(
								XView*					inView,
								uint16_t				inAction);
	virtual bool			ValuesAreValid(
								XDialogBox*				inDialog);
								
protected:
	XView*			mHitView;
	TFT_ILI9488		mDisplay;
	XPT2046			mTouchScreen;
	AT24C			mPreferences;
	bool			mDisplaySleeping;
	MSPeriod		mDebouncePeriod;	// For buttons
	uint32_t		mStartPinState;		// For buttons
	int32_t			mDirtyPressure;
	int32_t			mCleanPressure;
	uint16_t		mX, mY;
	enum EStartsPerHour
	{
		eMaxStartsPerHour = 20
	};
	uint16_t		mStartsPerHourHead;
	uint16_t		mStartsPerHourTail;
	time32_t		mStartsPerHourRingBuffer[eMaxStartsPerHour];

public:
	static bool		sButtonPressed;
	
protected:
	
	bool					NoModalDialogDisplayed(void) const;
	void					ShowInfoView(
								bool					inUpdatePref = true);
	void					ShowFilterStatusGauge(
								bool					inUpdatePref = true);
	void					UpdateShowInfoViewrOnStartupPref(void);
	virtual void			DustCollectorJustStarted(void);
	virtual void			DustCollectorJustStopped(void);
	virtual void			StopDustBinMotor(void);
	virtual void			LoadingDeltas(void);
	void					ShowFilterSettingsDialog(void);
	void					SaveFilterSettingsDialogChanges(void);
	void					ShowBinSettingsDialog(void);
	void					SaveBinSettingsDialogChanges(void);
	void					ShowUtilitiesDialog(void);
	void					ShowSetClockDialog(void);
	void					SaveDCSettingsToSD(void);
	void					LoadDCSettingsFromSD(void);
	void					UpdateInfoPressureValues(void);
	void					UpdateInfoView(void);
	void					UpdateInfoViewBinMotor(void);
	void					WakeUp(void);
	void					GoToSleep(void);
	void					CheckButtons(void);
	void					AddStartToRingBuffer(
								time32_t				inStartTime);
	void					UpdateStartsPerHour(
								time32_t				inTime);
	uint32_t				StartsPerHour(void) const;
};

#endif // DustCollectorSTM32_h
