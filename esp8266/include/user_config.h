/***************************************************************************
* 
* File              : user_config.h
*
* Author			: Kurt E. Clothier
* Date				: June 1, 2015
* Modified			: June 22, 2015
*
* Description       : Header File for all Projects
*
* Compiler			: Xtensa Tools - GCC
* Hardware			: ESP8266-x
*
* More Information	: http://www.projectsbykec.com/
*					: http://www.pubnub.com/
*					: http://www.esp8266.com/
*
****************************************************************************/
#include "pubnub_ssid.h"

/**************************************************************************
	Definitions 
***************************************************************************/
// Define to print debug statements using DEBUG_PRINT()
#define DEBUG_PRINT_	1

/***************************************************************************
	Configuration Options
****************************************************************************/
// Networking
#ifndef SSID
#	define SSID "MY_SSID"
#	define SSID_PW "MY_PASSWORD"
#endif
#define SSID_PASSWORD SSID_PW

#define STATION_MODE	0x01
#define SSID_LENGTH		32
#define PW_LENGTH		32

#define IP_CHECK_DELAY	2000	// Time (s) to wait before (re)checking IP

// Serial Baudrate
#define	BITRATE			9600

// ATMEGA328p TWI Slave Address
#define ATMEGA_SLA		0x1C

/***************************************************************************
	Constants & Status Flags
****************************************************************************/
// Status Flags
#define FIRST_BYTE		0x10
#define REQUEST_PUBLISH	0x20
#define REQUEST_RESET	0x40
#define TRANSMIT		0x80


#define TRANSMITTING	(stat_flag & TRANSMIT)
#define TIME_TO_PUBLISH (stat_flag & REQUEST_PUBLISH)

#define TIME_CHECK_SCALE	2500	// Check scale every 2.5 seconds
#define TIME_PUBLISH		48		// Always publish once every so many checks (2 min)

#define OPCODE_REQUEST_RESET	0xF0
#define OPCODE_REQUEST_DATA		0xC0


/***************************************************************************
	Hardware Definitions and Associated Macros
****************************************************************************/
