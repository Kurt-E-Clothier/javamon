/***************************************************************************
* 
* File              : javamon_main-vxx.c
*
* Author			: Kurt E. Clothier
* Date				: July 1, 2015
* Modified			: July 13, 2015
*
* Description       : IoT Coffee Pot Monitor
*
* Compiler			: AVR-GCC
* Hardware			: ATMEGA328p; Ext 8MHz Osc
* Fuse Settings		: lfuse: 0xF7 ; hfuse: 0xD9 ; efuse: 0x07
*
* More Information	: http://www.projectsbykec.com/
*
****************************************************************************/
 #define VERSION	0x30// Firmware Version 3.0 - Basic Functionality
 /***************************************************************************
  Completed
	- Correctly reading Scale LCD Screen
	- Mapping LCD data to useable information
	- Decode data for each digit 
	- Control Scale using buttons
	- TWI Communication with ESP8266
	- Remote ESP Reset (on command)
  
  Working On
	- Send 2 bytes back: 1 for dig[1,0] and 1 for dig[3,2]
	- This removes the need to calculate the level here!
	- Changes to the weight:level ratio will be handled in the subscribing client

  Additional Notes
	- Empty pot weight is static...
	- If a different type of pot is used, this will not work!

***************************************************************************/
 
/**************************************************************************
	Definitions for Conditional Code
***************************************************************************/

/**************************************************************************
	Included Header Files
***************************************************************************/
#include "modules/avr.h"
#include "definitions.h"
#include "modules/twi/twi.h"
#include <util/delay.h>

/**************************************************************************
	Definitions for Testing Purposes Only
***************************************************************************/


/**************************************************************************
	Global Variables
***************************************************************************/
#define stat_flag	GPIOR0	// Status Flags
#define TWI_msgBuf0	GPIOR1	// Buffer for TWI data
#define TWI_msgBuf1	GPIOR2	// Buffer for TWI data

static volatile bool TWI_isBusy = false;
static volatile uint8_t lastLevels[2];	// Keep track of last levels

// LCD readings
static volatile unsigned char lcdPins[COMS][2];		// Container for LCD Pin data
static volatile unsigned char lcdPinsBuf[COMS][2];	// Buffer for LCD Pin data

// For Handling COM Signals
static volatile uint8_t		com_cnt = 0;			// counter for timer 0 COM cycle
static volatile uint8_t		com_line = 0;			// Which COM line is active
static volatile bool		com_isHI = false;		// True when COM line is HI

/**************************************************************************
    Local Function Prototypes
***************************************************************************/
static void initialize_AVR(void);
static void readLCD(void);
static void updateLevel(void);
static void ESP_reset(void);
static uint8_t decodeDigit(uint8_t digit);


/**************************************************************************
    Main
***************************************************************************/
int main (void) 
{
	uint8_t liveTimer = 0;

  //- - - - - - - - - - -
  // Set up this device
  //- - - - - - - - - - -
	initialize_AVR();

	DISABLE_INTERRUPTS();
  
  //- - - - - - - - - - -
  // Turn on the ESP8266
  //- - - - - - - - - - -
	ENABLE_ESP();
	_delay_ms(100);

  //- - - - - - - - - - -
  // Turn on Scale
  // Get out of Clock Mode
  //- - - - - - - - - - -
	ENABLE_SCALE();
	_delay_ms(500);
	PUSH_UNIT_BUTTON();
	PUSH_UNIT_BUTTON();
	_delay_ms(250);

	ENABLE_INTERRUPTS();

  //- - - - - - - - - - -
  // Check Scale Mode
  //- - - - - - - - - - -
	_delay_ms(500);
	readLCD();

	// Should be in Weight mode
	if(!MODE_W) {
		// Enter Cycle Mode
		HOLD_ZERO_BUTTON();
		_delay_ms(3000);
		RELEASE_ZERO_BUTTON();
		// Cycle through modes
		do {
			PUSH_ZERO_BUTTON();
			_delay_ms(250);
			readLCD();
		} while(!MODE_W);
	}

	// Make sure weighing in Grams
	while(!MODE_G) {
		PUSH_UNIT_BUTTON();
		_delay_ms(250);
		readLCD();
	}
	
  //- - - - - - - - - - -
  // Turn On TWI
  //- - - - - - - - - - -
	lastLevels[0] = 0;
	lastLevels[1] = 0;
	TWI_RESET();

	/**********************************************
	 *	MAIN LOOP
	 **********************************************/
	for(;;) // Enter Main Loop
	{
		// Read the Scale LCD
		readLCD();

		// Update Pot Level data for TWI
		updateLevel();

		// Repeat every second
		_delay_ms(1000);

		// Reset ESP8266 if requested
		if(stat_flag & ESP_RESET_REQ)
			ESP_reset();

		// Keep the scale awake, every 45 or so seconds
		else if (++liveTimer == 45) {
			liveTimer = 0;
			KEEP_SCALE_AWAKE();
			_delay_ms(500);
		}

	}	// End of Main Loop
  
}	// End of Main


/**************************************************************************
	INTERRUPT HANDLERS
***************************************************************************/
// Catch-All Default for Unexpected Interrupts
//ISR(BADISR_vect){}

/***************************************************************
 * Timer/Counter0 Compare Match A - 0.2 ms timer
 *
 *	This ISR is the core timing mechanism of the LCD Reader.
 *	It is turned on after the rising edge of an analog comapre
 * 	event of LCD COM1. The timer then estimates the timing of
 *	LCD COM lines 2, 3, and 4. Better accuracy is achieved 
 *	with a 0.2ms timer than a 1ms timer.
 
 *	The Pins are read while the com line is HI. The LCD line is
 *	active if it is out of phase with the COM line (that is, 
 *	HI when COM is LO, and LO when COM is HI.
 ***************************************************************/
ISR(TIMER0_COMPA_vect)
{

	if (++com_cnt ==  TIME_COM_PULSE)
	{
		switch (com_line) {
		
	//------------------------
	//	COM 1
	// -----------------------
		case 1:	
			// COM 1 - Negative Edge
			if (com_isHI) {
				com_isHI = false;
			}
			// COM 2 - Positive Edge
			else {
				com_line = 2;
				com_isHI = true;
			}
			break;
	
	// -----------------------
	//	COM 2
	// -----------------------
		case 2:
			// COM 2 - Negative Edge
			if (com_isHI) {
				com_isHI = false;
			}
			// COM 3 - Positive Edge
			else {
				com_line = 3;
				com_isHI = true;
			}
			break;
		
	//------------------------	
	//	COM 3
	//------------------------
		case 3:
			// COM 3 - Negative Edge
			if (com_isHI) {
				com_isHI = false;
			}
			// COM 4 - Positive Edge
			else {
				com_line = 4;
				com_isHI = true;
			}
			break;
		
	//------------------------	
	//	COM 4
	//------------------------
		case 4:
			// COM 4 - Negative Edge
		default:
			com_isHI = false;
			com_line = 0;					// End of this COM Cycle
			DISABLE_TIM0A();				// Disable this timer ISR
			CLEAR_BIT(stat_flag, READ_LCD);	// Done Reading
			
			// Copy data buffer - this prevents interrupted data sets
			do {
				lcdPins[com_line][PINSA] = lcdPinsBuf[com_line][PINSA]; 
				lcdPins[com_line][PINSB] = lcdPinsBuf[com_line][PINSB]; 
			} while (++com_line < COMS);
			com_line = 0;

			// COM 1 - Positive Edge handled in AC-ISR
			break;
		}
		com_cnt = 0;
	}

	// Pins should be stable, so read them!
	// Read happens a little more than 1ms after the COM rising edge
	else if((com_cnt == TIME_READ_LCD) && (com_isHI))
	{
		_delay_us(100);
		
		// Read LCD Pins
		lcdPinsBuf[com_line - 1][PINSA] = LCD_PINS_A;
		lcdPinsBuf[com_line - 1][PINSB] = LCD_PINS_B;
	}
}


/***************************************************************
 * Analog Compare
 *
 * This is used to detect the rising LCD COM1 Line, 
 *	and to synchronize Timer0 in order to estimate 
 *	the other 3 COM lines.
 ***************************************************************/
ISR(ANALOG_COMP_vect)
{
	com_isHI = true;
	com_line = 1;
	ENABLE_TIM0A();			// Enable Timer 0A ISR
	DISABLE_ACI();			// Disable Analog Comp ISR
}

/***************************************************************
 * TWI Interrupt Service Routine - Communication with ESP8266
 *
 *	This ISR will control the communication between this chip 
 *	and the ESP8266. The next action is decided based on the
 *	code in the Status Register. 
 ***************************************************************/
ISR(TWI_vect)
{
	switch (TWSR)
	{
		//--------------------------------------
		// Received: SLA + R; ACK returned
		// - Transmit Data to Master
		//--------------------------------------
		case TWI_STX_ADR_ACK:
			CLEAR_BIT(stat_flag, TWI_DONE);
			TWDR = TWI_msgBuf0;
			TWI_isBusy = true;
			TWI_RESET();
			break;

		//--------------------------------------
		// Transmitted TWDR; ACK received
		// - Transmit next byte if there is one.
		//--------------------------------------
		// Transmitted: Last data byte in TWDR
		case TWI_STX_DATA_ACK_LAST_BYTE:
		case TWI_STX_DATA_ACK:          
		case TWI_STX_DATA_NACK:  
			if (TWI_NOT_DONE) {
				TWDR = TWI_msgBuf1;
				SET_BIT(stat_flag, TWI_DONE);
				TWI_RESET();
				break;
			}

		//--------------------------------------
		// Transmitted TWDR; NACK received
		// - Need to resend?
		//--------------------------------------
		//case TWI_STX_DATA_NACK:  
    
		//--------------------------------------
		// STOP or Repeated START
		//--------------------------------------
		case TWI_SRX_STOP_RESTART:
			TWI_isBusy = false;
			TWI_RECOVER();
			TWI_RESET();
			break;

		//--------------------------------------
		// Received: SLA + W; ACK returned
		// - ESP8266 Requests a reset!
		//--------------------------------------
		case TWI_SRX_ADR_ACK:
			SET_BIT(stat_flag, ESP_RESET_REQ);
		//--------------------------------------
		// Received: General Call; ACK returned
		// - Nothing special to do...
		//--------------------------------------
		case TWI_SRX_GEN_ACK:
		//--------------------------------------
		// States that should never happen...
		//--------------------------------------
		// Received: Data after SLA+W; ACK returned
		case TWI_SRX_ADR_DATA_ACK:
		// Received: Data after Gen Call; ACK returned
		case TWI_SRX_GEN_DATA_ACK:
		// Received: Data after SLA+W; NACK returned
		case TWI_SRX_ADR_DATA_NACK:
		// Received: Data after SLA+W; NACK returned
		case TWI_SRX_GEN_DATA_NACK:

		// Bus Error: Illegal START or STOP
		case TWI_BUS_ERROR:
		// And anything else...
		default:     
			TWI_isBusy = false;
			TWI_RECOVER();
			TWI_RESET();
			break;
	}
}

/**************************************************************************
	LCD READING UTILITIES
***************************************************************************/
// Read LCD Screen
static void readLCD(void)
{
	SET_BIT(stat_flag, READ_LCD);
	ENABLE_ACI();
	LOOP_UNTIL_BIT_LO(stat_flag, READ_LCD);
	DISABLE_ACI();


}

// Get Pot Level based on Scale LCD reading
static void updateLevel(void)
{
	uint8_t weight0 = LVL_ERROR;
	uint8_t weight1 = LVL_ERROR;

	// Record each digit in 2 bytes
	// Full weight is weight0 + 100 * weight1
	weight0 = decodeDigit(0);
	weight0 += 10 * decodeDigit(1);
	weight1 = decodeDigit(2);
	weight1 += 10 * decodeDigit(3);

	// Update the TWI Message Buffer
	while(TWI_isBusy);
	TWI_msgBuf0 = weight1;
	TWI_msgBuf1 = weight0;

}


// Decode the bits in a digit
// Return value of digit
static uint8_t decodeDigit(uint8_t digit)
{
	uint8_t pin1 = 0;
	uint8_t pin2 = 0;
	uint8_t pins = 0;
	uint8_t com = 0;
	uint8_t byte = 0x00;

	// Need to know what pins we are using!
	switch (digit)
	{
	case 0:
		pin1 = DIG_01;
		pin2 = DIG_02;
		pins = PINSB;
		break;
	case 1:
		pin1 = DIG_11;
		pin2 = DIG_12;
		pins = PINSB;
		break;
	case 2:
		pin1 = DIG_21;
		pin2 = DIG_22;
		pins = PINSA;
		break;
	case 3:
		pin1 = DIG_31;
		pin2 = DIG_32;
		pins = PINSA;
		break;
	default:
		return 0;
	}
	
	// byte == [MSB .. LSB] = [4-1, 3-1, .... 2-2, 1-2]
	// Loop through all coms for each pin
	for (com = 0; com < COMS; com++)
	{
		if (lcdPins[com][pins] & pin1)
			SET_BIT(byte, (16 << com));	// 16 = 0b00010000
	}
	for (com = 0; com < COMS; com++)
	{
		if (lcdPins[com][pins] & pin2)
			SET_BIT(byte, (1 << com));
	}
	
	byte &= ~0x01;	// Don't care about dot segment

	// Convert byte to Decimal
	switch (byte) 
	{
	case 0xBE:
		byte = 0;
		break;
	case 0x06:
		byte = 1;
		break;
	case 0x7C:
		byte = 2;
		break;
	case 0x5E:
		byte = 3;
		break;
	case 0xC6:
		byte = 4;
		break;
	case 0xDA:
		byte = 5;
		break;
	case 0xFA:
		byte = 6;
		break;
	case 0x8E:
		byte = 7;
		break;
	case 0xFE:
		byte = 8;
		break;
	case 0xDE:
		byte = 9;
		break;
	case 0x00:
	default:
		byte = 0;
		break;
	}
	return byte;
}

/**************************************************************************
	MISCELLANEOUS UTILITIES
***************************************************************************/
// Reset the ESP8266
static void ESP_reset(void)
{
	cli();

	// Reset LCD Read Cycle
	DISABLE_TIM0A();
	DISABLE_ACI();
	com_isHI = false;
	com_line = 0;					
	CLEAR_BIT(stat_flag, READ_LCD);

	// Reset the ESP8266
	DISABLE_ESP();
	_delay_ms(50);
	ENABLE_ESP();
	_delay_ms(100);
	CLEAR_BIT(stat_flag, ESP_RESET_REQ);

	sei();
}

/**************************************************************************
	INITIALIZATION ROUTINES AND POWER MODES
***************************************************************************/

/****** Initialize the ATmega328p microcontroller *****/
void initialize_AVR(void)
{ 
	cli();	// Turn off interrupts

	// Set up AVR I/O Pins
	DDRC = ESP_RST | SCALE_PWR;

	// Power Reduction Register - Enable Modules as Used
	PRR = 
		//_BV(PRTWI) |		// Disable TWI Clock
		_BV(PRSPI) |		// Disable SPI Clock
		_BV(PRTIM2) |		// Disable Timer2 Clock
		_BV(PRTIM1) |		// Disable Timer1 Clock
		//_BV(PRTIM0) |		// Disable Timer0 Clock
		_BV(PRUSART0) ;		// Disable USART0 CLock
		//_BV(PRADC);		// Disable ADC Clock (Needed for Comparator)

	// Timer0 - 0.2ms counter, enabled in Analog Comp ISR
	TCCR0A = _BV(WGM01);	// CTC Mode
	TCCR0B = _BV(CS01);		// Prescaler = 8
	OCR0A = 24;				// (1MHz * 0.2ms / 8)-1

	// Analog Comparator - Detecting LCD COM 1
	ACSR = 
		_BV(ACIS1) |					// Interrupt on Rising Output Edge
		_BV(ACIS0) |
		_BV(ACIE);						// Enable Interrupt
	DIDR1 = _BV(AIN1D) | _BV(AIN0D);	// Disable digital inputso

	// TWI - Communication with ESP8266 (Enabled later)
	TWAR = 
		(TWI_SLAVE_ADDRESS << 1) |	// TWI Slave Address
		_BV(TWGCE);					// Recognize General Call
	TWI_msgBuf0 = 0x00;
	TWI_msgBuf1 = 0x00;

	sei();	// Turn on interrupts
}



