/***************************************************************************
* 
* File              : twi.h
*
* Author			: Kurt E. Clothier
* Date				: July 9, 2015
* Modified			: June 9, 2015
*
* Description       : Using I2C
*
* Compiler			: Xtensa Tools - GCC
* Hardware			: ESP8266-x
*
* More Information	: http://www.projectsbykec.com/
*					: http://www.pubnub.com/
*					: http://www.esp8266.com/
*
****************************************************************************/

/**************************************************************************
	Definitions 
***************************************************************************/

/***************************************************************************
	Constants & Status Flags
****************************************************************************/
// TWI State Flags
#define TWI_IDLE		0x00
#define TWI_START		0x01
#define TWI_SLA			0x08
#define TWI_SLA_READ	0x09
#define TWI_SLA_WRITE	0x0A
#define TWI_ACK			0x10
#define TWI_STOP		0x40

#define TWI_RECV_ACK	0xF0
#define TWI_RECV_NACK	0xE0

#define TWI_READ		0x01
#define TWI_WRITE		0x00

/***************************************************************************
	Hardware Definitions and Associated Macros
****************************************************************************/
// Reading State of SDA - Input Pin
#define SDA_IS_HI		(GPIO_INPUT_GET(GPIO_ID_PIN(0)))
#define SDA_IS_LO		!SDA_IS_HI

// Reading State of SCL - Input Pin
#define SCL_IS_HI		(GPIO_INPUT_GET(GPIO_ID_PIN(2)))
#define SCL_IS_LO		!SCL_IS_HI

// Release I2C Lines
#define RELEASE_SCL()	SET_AS_INPUT(2)
#define RELEASE_SDA()	SET_AS_INPUT(0)

// Driving SDA
#define SDA_SET_HI()		RELEASE_SDA()
#define SDA_SET_LO()		SET_PIN_LO(0)
#define SDA_SEND_ACK()		SET_PIN_LO(0)
#define SDA_SEND_NACK()		RELEASE_SDA()

// Driving SCL
#define SCL_SET_HI()		RELEASE_SCL()
#define SCL_SET_LO()		SET_PIN_LO(2)

