/*
*	DCXViews.h, Copyright Jonathan Mackey 2023
*	UI and constant strings.
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
#ifndef DCXViews_h
#define DCXViews_h

#include "FilterStatusGauge.h"
#include "ST77XXToXPT2046Alignment.h"
#include "ValueFormatter.h"
#include "XAlert.h"
#include "XAnimatedFontIcon.h"
#include "XCheckboxButton.h"
#include "XColoredView.h"
#include "XDateValueField.h"
#include "XDialogBox.h"
#include "XMenuButton.h"
#include "XMenuItem.h"
#include "XNumberValueField.h"
#include "XPopUpButton.h"
#include "XPushButton.h"
#include "XRadioButton.h"
#include "XRootView.h"
#include "XStepper.h"

static const char kVerticalEllipsisStr[] = ".";
static const char kSeparatorStr[] = "-";
static const char kOKStr[] = "OK";
static const char kCancelStr[] = "Cancel";
static const char kCloseStr[] = "Close";
static const char kInfoStr[] = "Info";
static const char kFilterSettingsStr[] = "Filter Settings";
static const char kBinSettingsStr[] = "Bin Settings";
static const char kSetClockStr[] = "Set Clock";
static const char kUtilitiesStr[] = "Utilities";
static const char kFilterStatusStr[] = "Filter Status";
static const char kAboutStr[] = "About";

// Filter settings dialog
static const char kSetStr[] = "Set";
static const char kPresUnitStr[] = "Pressure unit:";
static const char kPresUnitInchesStr[] = "Inches";
static const char kPresUnitHPaStr[] = "hPa";

static const char kDirtyPresStr[] = "\"Dirty\" pressure:";
static const char kCleanPresStr[] = "\"Clean\" pressure:";
static const char kDisplayPresStr[] = "Display pressure in filter status pane";

static const char kUseCurrentPresMessageStr[] = "Use current pressure\nas clean pressure?";
static const char kUseCurrentPresMessageStr2[] = "Use current pressure as\ndirty pressure?";
static const char kCleanPresInvalidMessageStr[] = "The clean pressure must be\nless than the dirty pressure.";
static const char kDCMustBeRunningMessageStr[] = "The pressure cannot be read\nwhen the dust collector isn't\nrunning.";

// Bin settings dialog
static const char kDisableMotStr[] = "Disable motor";
static const char kBinWarnThresStr[] = "Bin warning threshold:";
static const char kDisabledStr[] = "Disabled";
static const char kStoppedStr[] = "Stopped";
static const char kStoppingStr[] = "Stopping…";
static const char kWaitStr[] = "Waiting…";

// Utilities dialog
static const char kDustBinMotorStr[] = "Dust bin motor:";
static const char kStartStr[] = "Start";
static const char kStopStr[] = "Stop";
static const char kSaveSettingsStr[] = "Save settings to SD card:";
static const char kSaveStr[] = "Save";
static const char kLoadSettingsStr[] = "Load settings from SD card:";
static const char kLoadStr[] = "Load";
static const char kSaveToSDFailedStr[] = "Save to SD failed.";
static const char kLoadFromSDFailedStr[] = "Load from SD failed.";
static const char kNoSDCardFoundStr[] = "No SD card found.";
static const char kSavedDCSettingsStr[] = "Saved to file DCSettings.txt";
static const char kLoadDCSettingsStr[] =	"Press OK to load file\n"
											"DCSettings.txt.  The board\n"
											"will restart if successful.";

// About Box
static const char kSoftwareNameStr[] = "Dust Collector Monitor";
static const char kVersionStr[] = "STM32 version 1.0";
static const char kCopyrightStr[] = "Copyright Jonathan Mackey 2023";

// Info view
static const char kStartsPerHourStr[] = "Starts per hour:";
static const char kTemperatureStr[] = "Temperature:";
static const char kDuctPresStr[] = "Duct:";
static const char kAmbientPresStr[] = "Ambient:";
static const char kBasePresStr[] = "Base:";
static const char kStaticPresStr[] = "Static:";
static const char kBinMotorValueStr[] = "Bin motor reading:";
static const char kBinWarnAtReadingStr[] = "Warn at reading:";

// Dust Collector Fault Status
static const char kDustBinFullStr[] = "Dust bin full";
static const uint16_t	kFilterLoadedMessageTag = 100;
static const char kFilterLoadedStr[] = "Filter loaded";
static const uint16_t	kDustBinFullMessageTag = 101;

// Touch screen alignment instructions.
static const char	kAlignmentInstMsg[] =
									"Precisely touch the tiny white dot\n"
									"within each circle. A magenta dot\n"
									"will reflect the alignment accuracy.\n"
									"The red circle at the top of the display\n"
									"will change to green when enough points\n"
									"have been entered to save. When satisfied\n"
									"with the alignment, press Enter to save.\n"
									"Press Up to quit alignment at any time.";

static const uint16_t	kFilterSettingsDialogTag = 100;
static const uint16_t	kPresUnitLabelTag = 102;
static const uint16_t	kPresUnitPopUpTag = 103;
static const uint16_t	kPresUnitMenuTag = 104;
static const uint16_t	kDirtyPresLabelTag = 105;
static const uint16_t	kDirtyPresValueFieldTag = 106;
static const uint16_t	kDirtyPresStepperTag = 107;
static const uint16_t	kSetDirtyPresBtnTag = 108;
static const uint16_t	kCleanPresLabelTag = 109;
static const uint16_t	kCleanPresValueFieldTag = 110;
static const uint16_t	kCleanPresStepperTag = 111;
static const uint16_t	kSetCleanPresBtnTag = 112;
static const uint16_t	kDispPresCheckboxTag = 113;

static const uint16_t	kBinSettingsDialogTag = 200;
static const uint16_t	kDisableMotorCheckboxTag = 201;
static const uint16_t	kBinWrnThresLabelTag = 202;
static const uint16_t	kBinWrnThresValueFieldTag = 203;
static const uint16_t	kBinWrnThresStepperTag = 204;

static const uint16_t	kInfoViewTag = 300;
static const uint16_t	kInfoDateValueFieldTag = 301;
static const uint16_t	kStartsPerHourLabelTag = 302;
static const uint16_t	kStartsPerHourValueFieldTag = 303;
static const uint16_t	kTemperatureLabelTag = 304;
static const uint16_t	kTemperatureValueFieldTag = 305;
static const uint16_t	kDuctPresLabelTag = 306;
static const uint16_t	kDuctPresValueFieldTag = 307;
static const uint16_t	kAmbientPresLabelTag = 308;
static const uint16_t	kAmbientPresValueFieldTag = 309;
static const uint16_t	kBasePresLabelTag = 310;
static const uint16_t	kBasePresValueFieldTag = 311;
static const uint16_t	kStaticPresLabelTag = 312;
static const uint16_t	kStaticPresValueFieldTag = 313;
static const uint16_t	kBinMotorValueLabelTag = 314;
static const uint16_t	kBinMotorValueFieldTag = 315;
static const uint16_t	kBinWarnAtReadingLabelTag = 316;
static const uint16_t	kBinWarnAtValueFieldTag = 317;

static const uint16_t	kMainMenuBtnTag = 900;
static const uint16_t	kMainMenuTag = 901;

static const uint16_t	kSetClockDialogTag = 400;
static const uint16_t	kDateValueFieldTag = 401;
static const uint16_t	kDateValueStepperTag = 402;

static const uint16_t	kFilterStatusGaugeTag = 500;
static const uint16_t	kFilterPresValueFieldTag = 501;

static const uint16_t	kAlertDialogTag = 600;
static const uint16_t	kWarningDialogTag = 700;
static const uint16_t	kDCStatusIconTag = 800;
static const uint16_t	kTouchScreenAlignmentTag = 1000;

static const uint16_t	kUtilitiesDialogTag = 1100;
static const uint16_t	kLoadSettingsLabelTag = 1101;
static const uint16_t	kLoadSettingsBtnTag = 1102;
static const uint16_t	kSaveSettingsLabelTag = 1103;
static const uint16_t	kSaveSettingsBtnTag = 1104;
static const uint16_t	kDustBinMotorLabelTag = 1105;
static const uint16_t	kMotorStartStopBtnTag = 1106;

static const uint16_t	kAboutBoxTag = 1200;
static const uint16_t	kSoftwareNameLabelTag = 1201;
static const uint16_t	kVersionLabelTag = 1202;
static const uint16_t	kCopyrightLabelTag = 1203;

static const uint16_t	kSpaceBetween = 10;
static const uint16_t	kLabelYAdj = 4;
static const uint16_t	kRowHeight = 36;
static const uint16_t	kDialogBGColor = 0xF77D;

static const uint16_t	kInfoMenuItem = 1;
static const uint16_t	kFilterStatusMenuItem = 2;
static const uint16_t	kSeparatorMenuItem = 3;
static const uint16_t	kFilterSettingsMenuItem = 4;
static const uint16_t	kBinSettingsMenuItem = 5;
static const uint16_t	kSetClockMenuItem = 6;
static const uint16_t	kSeparatorMenuItem1 = 7;
static const uint16_t	kUtilitiesMenuItem = 8;
static const uint16_t	kAboutMenuItem = 9;

/*
*	The touch screen alignment view is displayed when the Enter button of the
*	5 button array is pressed.  This button array is only attached/needed when
*	alignment of the touch screen is performed.  The result of the alignment
*	is written to the EEPROM.
*/
ST77XXToXPT2046Alignment touchScreenAlignment(kTouchScreenAlignmentTag,
				&UI20ptFont, nullptr,
				kAlignmentInstMsg);
				
// The alertDialog is displayed within the filter settings dialog so it has
// the same FG and BG color.
XAlert	alertDialog(kAlertDialogTag, &touchScreenAlignment,
				kOKStr, kCancelStr, nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor, kDialogBGColor);

// The warning dialog is displayed on a black background
XAlert	warningDialog(kWarningDialogTag, &alertDialog,
				kOKStr, nullptr, nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor, kDialogBGColor);

//	Main menu

XMenuItem	aboutMenuItem(kAboutMenuItem, kAboutStr);
XMenuItem	utilitiesMenuItem(kUtilitiesMenuItem, kUtilitiesStr, &aboutMenuItem);
XMenuItem	separatorMenuItem1(kSeparatorMenuItem1, kSeparatorStr, &utilitiesMenuItem);
XMenuItem	setClockMenuItem(kSetClockMenuItem, kSetClockStr, &separatorMenuItem1);
XMenuItem	binSettingsMenuItem(kBinSettingsMenuItem, kBinSettingsStr, &setClockMenuItem);
XMenuItem	filterSettingsMenuItem(kFilterSettingsMenuItem, kFilterSettingsStr, &binSettingsMenuItem);
XMenuItem	separatorMenuItem(kSeparatorMenuItem, kSeparatorStr, &filterSettingsMenuItem);
XMenuItem	filterStatusMenuItem(kFilterStatusMenuItem, kFilterStatusStr, &separatorMenuItem);
XMenuItem	infoMenuItem(kInfoMenuItem, kInfoStr, &filterStatusMenuItem);
XMenu		mainMenu(kMainMenuTag, &UI20ptFont, &infoMenuItem);
XMenuButton mainMenuBtn(480-30, 0, 27, 0,
				kMainMenuBtnTag, &mainMenu, &warningDialog,
				kVerticalEllipsisStr, &UI20ptFont);

XAnimatedFontIcon dcStatusIcon(480-32-2, 320-32-2, 32, 32,
				kDCStatusIconTag, &mainMenuBtn, 'A', 'B', &DC_Icons::font);

// Filter settings dialog
XLabel		presUnitLabel(0, kLabelYAdj, 150, 26,
				kPresUnitLabelTag, nullptr, kPresUnitStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignRight);
				
static const uint16_t	kHPaMenuItem = 1;
static const uint16_t	kInchesMenuItem = 2;
XMenuItem	inchesMenuItem(kInchesMenuItem, kPresUnitInchesStr);
XMenuItem	hPaMenuItem(kHPaMenuItem, kPresUnitHPaStr, &inchesMenuItem);
XMenu		presUnitMenu(kPresUnitMenuTag,
				&UI20ptFont, &hPaMenuItem, nullptr, kDialogBGColor);

XPopUpButton presUnitPopUp(150+kSpaceBetween, 0, 90, 0,
				kPresUnitPopUpTag, &presUnitMenu, &presUnitLabel,
				&UI20ptFont,
				XPopUpButton::eLargePopUpSize,
				XFont::eWhite, kDialogBGColor);
				
XLabel		cleanPresLabel(0, kLabelYAdj + kRowHeight, 150, 26,
				kCleanPresLabelTag, &presUnitPopUp, kCleanPresStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignRight);
XNumberValueField	cleanPresValueField(0,0,60+kSpaceBetween,
				kCleanPresValueFieldTag, &cleanPresLabel,
				&UI20ptFont, 0, 110000, 0, 10, true, true,
				&ValueFormatter::PressureToStringNoUnit,
				XFont::eBlack, kDialogBGColor);
XStepper	cleanPresStepper(225, kRowHeight, 0, 0,
				kCleanPresStepperTag, &cleanPresValueField);
XPushButton setCleanPresBtn(253, kRowHeight, 80, 0,
				kSetCleanPresBtnTag, &cleanPresStepper, kSetStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);

XLabel		dirtyPresLabel(0, kLabelYAdj + (kRowHeight*2), 150, 26,
				kDirtyPresLabelTag, &setCleanPresBtn, kDirtyPresStr,
				&UI20ptFont, nullptr, XFont::eBlack,
				kDialogBGColor, XFont::eAlignRight);
XNumberValueField	dirtyPresValueField(0,0,60+kSpaceBetween,
				kDirtyPresValueFieldTag, &dirtyPresLabel,
				&UI20ptFont, 0, 110000, 0, 10, true, true,
				&ValueFormatter::PressureToStringNoUnit,
				XFont::eBlack, kDialogBGColor);
XStepper	dirtyPresStepper(225, kRowHeight*2, 0, 0,
				kDirtyPresStepperTag, &dirtyPresValueField);
XPushButton setDirtyPresBtn(253, kRowHeight*2, 80, 0,
				kSetDirtyPresBtnTag, &dirtyPresStepper, kSetStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);

XCheckboxButton dispPresCheckbox(0, kRowHeight*3, 340, 0,
				kDispPresCheckboxTag, &setDirtyPresBtn, kDisplayPresStr,
				&UI20ptFont,
				XCheckboxButton::eLargeCheckSize,
				XFont::eBlack, kDialogBGColor);

XDialogBox	filterSettingsDialog(&dispPresCheckbox,
				kFilterSettingsDialogTag, &dcStatusIcon,
				kOKStr, kCancelStr, kFilterSettingsStr,
				&UI20ptFont,
				nullptr, kDialogBGColor);

// Bin settings dialog
XCheckboxButton disableMotorCheckbox(0, 0, 150, 0,
				kDisableMotorCheckboxTag, nullptr, kDisableMotStr,
				&UI20ptFont,
				XCheckboxButton::eLargeCheckSize,
				XFont::eBlack, kDialogBGColor);

XLabel		binWrnThresLabel(0, kLabelYAdj + kRowHeight, 195, 26,
				kBinWrnThresLabelTag, &disableMotorCheckbox, kBinWarnThresStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignRight);
XNumberValueField	binWrnThresValueField(0,0,40+kSpaceBetween,
				kBinWrnThresValueFieldTag, &binWrnThresLabel,
				&UI20ptFont, 0, 125, 5, 1, true, false,
				nullptr,
				XFont::eBlack, kDialogBGColor);
XStepper	binWrnThresStepper(250, kRowHeight, 0, 0,
				kBinWrnThresStepperTag, &binWrnThresValueField);
XDialogBox	binSettingsDialog(&binWrnThresStepper,
				kBinSettingsDialogTag, &filterSettingsDialog,
				kOKStr, kCancelStr, kBinSettingsStr,
				&UI20ptFont,
				nullptr, kDialogBGColor);

// Set clock dialog
XDateValueField	dateValueField(0, 0, 0,
				kDateValueFieldTag, nullptr,
				&UI20ptFont,
				XFont::eBlack, kDialogBGColor);
XStepper	dateValueStepper(210, 0, 0, 0,
				kDateValueStepperTag, &dateValueField);
XDialogBox	setClockDialog(&dateValueStepper,
				kSetClockDialogTag, &binSettingsDialog,
				kOKStr, kCancelStr, kSetClockStr,
				&UI20ptFont,
				nullptr, kDialogBGColor);
				
// Utilities dialog
XPushButton motorStartStopBtn(232+kSpaceBetween, 0, 80, 0,
				kMotorStartStopBtnTag, nullptr, kStartStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);
XLabel		dustBinMotorLabel(0, kLabelYAdj, 232, 26,
				kDustBinMotorLabelTag, &motorStartStopBtn, kDustBinMotorStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignRight);
XPushButton saveSettingsBtn(232+kSpaceBetween, kRowHeight, 80, 0,
				kSaveSettingsBtnTag, &dustBinMotorLabel, kSaveStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);
XLabel		saveSettingsLabel(0, kLabelYAdj + kRowHeight, 232, 26,
				kSaveSettingsLabelTag, &saveSettingsBtn, kSaveSettingsStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignRight);
XPushButton loadSettingsBtn(232+kSpaceBetween, kRowHeight*2, 80, 0,
				kLoadSettingsBtnTag, &saveSettingsLabel, kLoadStr,
				&UI20ptFont,
				XFont::eWhite, kDialogBGColor);
XLabel		loadSettingsLabel(0, kLabelYAdj + (kRowHeight*2), 232, 26,
				kLoadSettingsLabelTag, &loadSettingsBtn, kLoadSettingsStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignRight);
XDialogBox	utilitiesDialog(&loadSettingsLabel,
				kUtilitiesDialogTag, &setClockDialog,
				kCloseStr, nullptr, kUtilitiesStr,
				&UI20ptFont,
				nullptr, kDialogBGColor);

// About box
XLabel		softwareNameLabel(0, 0, 280, 26,
				kSoftwareNameLabelTag, nullptr, kSoftwareNameStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XLabel		versionLabel(0, kRowHeight, 280, 26,
				kVersionLabelTag, &softwareNameLabel, kVersionStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XLabel		copyrightLabel(0, kRowHeight*2, 280, 26,
				kCopyrightLabelTag, &versionLabel, kCopyrightStr,
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XDialogBox	aboutBox(&copyrightLabel,
				kAboutBoxTag, &utilitiesDialog,
				kCloseStr, nullptr, nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor);
				
// Filter status gauge view				
XNumberValueField filterPresValueField(2, 320-22, 100,
				kFilterPresValueFieldTag, nullptr,
				&UI20ptFont, 0, 110000, 0, 0, false, false,
				&ValueFormatter::PressureToString,
				XFont::eYellow, XFont::eBlack, XFont::eAlignRight);
FilterStatusGauge filterStatusGauge(0, 0, 480, 320,
				kFilterStatusGaugeTag, &aboutBox, &filterPresValueField,
				&UI64ptFont);

// Info view
XDateValueField	infoDateValueField(136,3,0,
				kInfoDateValueFieldTag, nullptr,
				&UI20ptFont);
				
XLabel			startsPerHourLabel(0, kRowHeight, 170, 26,
				kStartsPerHourLabelTag, &infoDateValueField, kStartsPerHourStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XNumberValueField startsPerHourValueField(170+kSpaceBetween, kRowHeight, 130,
				kStartsPerHourValueFieldTag, &startsPerHourLabel,
				&UI20ptFont, 1, 100, 0, 1, false, false,
				nullptr,	// nullptr = use default Int32ToString
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);
				
XLabel		temperatureLabel(0, (kRowHeight*2), 170, 26,
				kTemperatureLabelTag, &startsPerHourValueField, kTemperatureStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XNumberValueField temperatureValueField(170+kSpaceBetween, kRowHeight*2, 130,
				kTemperatureValueFieldTag, &temperatureLabel,
				&UI20ptFont, 0, 8500, -4000, 1, true, false,
				&ValueFormatter::TemperatureToString,
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);
				
XLabel		ductPresLabel(0, (kRowHeight*3), 170, 26,
				kDuctPresLabelTag, &temperatureValueField, kDuctPresStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XNumberValueField ductPresValueField(170+kSpaceBetween, kRowHeight*3, 130,
				kDuctPresValueFieldTag, &ductPresLabel,
				&UI20ptFont, 0, 110000, 0, 1, true, false,
				&ValueFormatter::PressureToString,
				XFont::eCyan, XFont::eBlack, XFont::eAlignLeft);

XLabel		ambientPresLabel(0, (kRowHeight*4), 170, 26,
				kAmbientPresLabelTag, &ductPresValueField, kAmbientPresStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XNumberValueField ambientPresValueField(170+kSpaceBetween, kRowHeight*4, 130,
				kAmbientPresValueFieldTag, &ambientPresLabel,
				&UI20ptFont, 0, 110000, 0, 1, true, false,
				&ValueFormatter::PressureToString,
				XFont::eCyan, XFont::eBlack, XFont::eAlignLeft);

XLabel		basePresLabel(0, (kRowHeight*5), 170, 26,
				kBasePresLabelTag, &ambientPresValueField, kBasePresStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XNumberValueField basePresValueField(170+kSpaceBetween, kRowHeight*5, 130,
				kBasePresValueFieldTag, &basePresLabel,
				&UI20ptFont, 0, 100, -100, 1, true, false,
				&ValueFormatter::PressureToString,
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);

XLabel		staticPresLabel(0, (kRowHeight*6), 170, 26,
				kStaticPresLabelTag, &basePresValueField, kStaticPresStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XNumberValueField staticPresValueField(170+kSpaceBetween, kRowHeight*6, 130,
				kStaticPresValueFieldTag, &staticPresLabel,
				&UI20ptFont, 0, 110000, 0, 1, true, false,
				&ValueFormatter::PressureToString,
				XFont::eYellow, XFont::eBlack, XFont::eAlignLeft);
XLabel		binMotorValueLabel(0, (kRowHeight*7), 170, 26,
				kBinMotorValueLabelTag, &staticPresValueField, kBinMotorValueStr,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XNumberValueField binMotorValueField(170+kSpaceBetween, kRowHeight*7, 130,
				kBinMotorValueFieldTag, &binMotorValueLabel,
				&UI20ptFont, 1, 100, 0, 1, true, false,
				nullptr,	// nullptr = use default Int32ToString
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);

XColoredView infoView(0, 0, 480, 320,
				kInfoViewTag, &filterStatusGauge, &binMotorValueField,
				nullptr, false);

XRootView	rootView(&infoView);

#endif // DCXViews_h
