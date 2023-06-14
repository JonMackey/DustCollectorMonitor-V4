/*
*	DispTestSTM32Config.h, Copyright Jonathan Mackey 2022
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
#ifndef Config_h
#define Config_h

#include <inttypes.h>
#include "PlatformDefs.h"

// This is coded for a STM32F103

#define BAUD_RATE	19200
#define DEBOUNCE_DELAY		20		// ms, for buttons

#define DISPLAY_CONTROLLER	TFT_ILI9488
namespace Config
{
	const pin_t		kDownBtnPin			= PA0;
	const pin_t		kEnterBtnPin		= PA1;
	const pin_t		kRightBtnPin		= PA2;
	const pin_t		kUpBtnPin			= PA3;
	const pin_t		kLeftBtnPin			= PA4;
	const pin_t		kSCK				= PA5;
	const pin_t		kMISO				= PA6;
	const pin_t		kMOSI				= PA7;
	const pin_t		kBacklightPin		= PA8;	// PWM
//	const pin_t		kTxPin				= PA9;
//	const pin_t		kRxPin				= PA10;
// PA11	USB-
// PA12 USB+
// PA13 SWDIO
// PA14 SWCLK
	const pin_t		kTouchCSPin			= PA15;
// PB0
	const pin_t		kMotorSensePin		= PB1;
// PB2	BOOT1
	const pin_t		kSDDetectPin		= PB3;
	const pin_t		kBMP0CSPin			= PB4;
	const pin_t		kBMP1CSPin			= PB5;
	const pin_t		kSDSelectPin		= PB6;
	const pin_t		kMotorControlPin	= PB7;
	const pin_t		kFlasherControlPin	= PB8;
// PB9
	const pin_t		kSCLPin				= PB10;
	const pin_t		kSDAPin				= PB11;
	const pin_t		kTouchIRQPin		= PB12;
	const pin_t		kDispResetPin		= PB13;
	const pin_t		kDispCSPin			= PB14;
	const pin_t		kDispDCPin			= PB15;
// PC13	Fixed LED
// PC14 RTC OSC
// PC15 RTC OSC

	const uint8_t	kDisplayRotation	= 3;	// 270
	const uint32_t	kEnterBtn			= _BV(1); //digitalPinToBitMask(PA1);
	const uint32_t	kUpBtn 				= _BV(3); //digitalPinToBitMask(PA3);
	const uint32_t	kRightBtn			= _BV(2); //digitalPinToBitMask(PA2);
	const uint32_t	kLeftBtn			= _BV(4); //digitalPinToBitMask(PA4);
	const uint32_t	kDownBtn			= _BV(0); //digitalPinToBitMask(PA0);
	
	const uint32_t	kPINAtnMask = (kEnterBtn | kRightBtn | kLeftBtn | kUpBtn | kDownBtn);


	const uint8_t	kTextInset			= 3; // Makes room for drawing the selection frame
	const uint8_t	kTextVOffset		= 6; // Makes room for drawing the selection frame
	// To make room for the selection frame the actual font height in the font
	// file is reduced.  The actual height is kFontHeight.
	const uint8_t	kFontHeight			= 43;

#if 1	
	// Touchscreen min/max for 4" ILI9488 display
	const uint16_t	kDisplayWidth		= 320;
	const uint16_t	kDisplayHeight		= 480;
	const bool		kInvertTouchX		= false;
#else
	// Touchscreen min/max for 3.5" ILI9488 display
	const uint16_t	kDisplayWidth		= 320;
	const uint16_t	kDisplayHeight		= 480;
	const bool		kInvertTouchX		= true;
#endif

	const uint8_t kAT24CDeviceAddr = 0x50;	// Serial EEPROM
	const uint8_t kAT24CDeviceCapacity = 8;	// Value at end of AT24Cxxx xxx/8
	
	/*
	*	The SDCPreferences occupy the first 24 bytes of the AT24C64 EEPROM
	*	[24 to 8192]		unused
	*/
	struct SDCPreferences
	{
		uint8_t		unused[2];
		uint8_t		clockFormat : 1,// The time format (0=24, 1=12)
					tempUnit : 1,	// Temperature display unit (0=C, 1=F)
					presUnit : 1,	// Pressure display unit (0=hPa, 1=Inches)
					unused5 : 5;
		uint8_t		binThreshold; // Dust bin motor trigger threshold.
								  // The value at which the motor will be
								  // stopped and the bin considered full.
		bool		binMotorEnabled;
		bool		displayPressure; // Display pressure in filter status view.
		bool		showInfoViewOnStartup;
		uint8_t		unused2;
		uint16_t	tsMinMax[4]; // Touch screen alignment values
		uint32_t	cleanPressure;
		uint32_t	dirtyPressure;
	}; // __attribute__ ((packed)); (was getting compiler warnings)
}

#endif // Config_h

