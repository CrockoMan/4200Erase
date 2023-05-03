//************************************************************************************
// Ver 0.3
//************************************************************************************

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "twi_drv.h"

#define MS_ADDR 0xA0
#define SetBit(x)    |= (1<<x) 
#define ClearBit(x)  &=~(1<<x) 

/*void Blink(void)
{		
	TWCR = ~(1 << TWEN);
	PORTC = 0xFF;
	DDRC |= (1 << PC5);
		
	while(1)
		{
			PORTC = (1 << PC5);
			_delay_ms(150);
			PORTC = (0 << PC5);
			_delay_ms(150);
		}
}*/

void Blink(uint8_t n)
{		
	int i = 0, j=0;
	TWCR = (0 << TWEN);
	PORTC = 0xFF;
	DDRC = (1 << PC3) | (1 << PC2);
	
	PORTC ClearBit(3);			// Красный светодиод
	PORTC ClearBit(2);			// Зеленый светодиод
	
//	if (n == 8) {PORTC = (1 << PC3); while(1);}
	if (n == 8)
	{
		for(j=0; j<3; j++)
		{
		PORTC = (1 << PC2);
		_delay_ms(1000);
		PORTC = (0 << PC2);
		}
	}
	else
	{
	
		if(bit_is_set(PINC, PC0)) n = 5;

		PORTC = (0 << PC3);
		_delay_ms(200);
	
//	while(1)
		for(j=0; j<3; j++)
		{
			while(1)
			{
				PORTC = (1 << PC3);
				_delay_ms(200);
				PORTC = (0 << PC3);
				_delay_ms(200);
				i++;
				if(i == n) break;
			}
			i = 0;
			_delay_ms(750);
		}
	}

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();
}



int i2c_delay(void)
{  
	for(uint16_t i = 0; i < 500; i++)
		{
		  if(TWCR & (1 << TWINT)) return 0;
		  _delay_us(10);
		}
	err = 1;
	return 1;
}


void i2c_init(void)
{
	TWSR = 0;
	TWBR = 0x10;
	TWCR = (1 << TWEN);
	err = 0;
}


void i2c_start(void)
{
	if(err) return;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	if(i2c_delay()) return;
	switch (TWSR)
		{
			case TW_START:
			case TW_REP_START:
			return;
			default:
			err = 6;
		}
}

void i2c_stop(void)
{
	TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
}

void i2c_send_byte(uint16_t byte)
{
	if(err) return;
	TWDR = byte;
	TWCR = (1 << TWINT) | (1 << TWEN);
	i2c_delay();
	switch (TWSR)
		{
			case TW_MT_DATA_ACK:
			return;
			default:
			err = 1;
		}                
}

void i2c_send_addr(uint8_t adr)
{
	if(err) return;
	//uint8_t i = 255;
	//rep:
	TWDR = adr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	if(i2c_delay()) return;
	switch (TWSR)
		{
			case TW_MT_SLA_ACK:
			case TW_MR_SLA_ACK:
			return;
			default:
			//if(i--) {i2c_start(); _delay_us(20); goto rep;} else // if use it, rem _delay_ms(5.0);
			err = 3;
		}       
}

uint8_t i2c_read_byte(void)
{
	if(err) return 0;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	if(i2c_delay()) return 0;
	switch (TWSR)
		{
			case TW_MR_DATA_ACK:
			case TW_MR_DATA_NACK:
			break;
			default:
			err = 1;
		}       
 
	return TWDR;
}

uint8_t i2c_read_single_byte(void)
{
	if(err) return 0;
	TWCR = (1 << TWINT) | (1 << TWEN);
	if(i2c_delay()) return 0;
	switch (TWSR)
		{
			case TW_MR_DATA_ACK:
			case TW_MR_DATA_NACK:
			break;
			default:
			err = 1;
		}       
 
	return TWDR;
}

void chip_wait(void)
{
	int rep = 1;
	err = 0;
	
		while(rep < 40)
		{
			TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
			if(i2c_delay()) return;

			switch (TWSR)
				{
					case TW_START:
					case TW_REP_START:
					break;
					default:
					err = 6;
					return;
				}     
			
			TWDR = MS_ADDR;
			TWCR = (1 << TWINT) | (1 << TWEN);
			for(uint16_t i = 0; i < 500; i++)
				{
					if(TWCR & (1 << TWINT)) break;
					_delay_us(10);
				}

			switch (TWSR)
				{
					case TW_MT_SLA_ACK:
					case TW_MR_SLA_ACK:
					i2c_stop();
					_delay_ms(50);
					return;
					default:
					rep++;
				}    
		
			i2c_stop();
			_delay_ms(100);
		}
	Blink(3);
}


uint8_t read_byte(uint16_t eeaddr) 
{
	i2c_start();
	if (eeaddr > 0xFF) i2c_send_addr(MS_ADDR | 2); else i2c_send_addr(MS_ADDR);
	i2c_send_byte(eeaddr);
	i2c_start();
	i2c_send_addr(MS_ADDR | 1);
	uint8_t i = i2c_read_single_byte();
	i2c_stop();
	return(i);
}


void read_str(uint16_t eeaddr, uint8_t *Buf)
{
	uint16_t i;
	
	i2c_start();
	if (eeaddr > 0xFF) i2c_send_addr(MS_ADDR | 2); else i2c_send_addr(MS_ADDR);
	i2c_send_byte(eeaddr);
	i2c_start();
	i2c_send_addr(MS_ADDR | 1);
	for(i=0; i < 7; i++)
	Buf[i] = i2c_read_byte();
	Buf[i] = i2c_read_single_byte();
	i2c_stop();
}

void write_byte(uint16_t eeaddr, uint8_t buf)
{
	i2c_start();
	if (eeaddr > 0xFF) i2c_send_addr(MS_ADDR | 2); else i2c_send_addr(MS_ADDR);
	i2c_send_byte(eeaddr);
	i2c_send_byte(buf);
	i2c_stop();
	_delay_ms(5.0);
	
	if(err) return;
	if(read_byte(eeaddr) != buf) err = 4;
}

void write_4byte(uint16_t eeaddr, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
	i2c_start();
	if (eeaddr > 0xFF) i2c_send_addr(MS_ADDR | 2); else i2c_send_addr(MS_ADDR);
	i2c_send_byte(eeaddr);
	i2c_send_byte(b1);
	i2c_send_byte(b2);
	i2c_send_byte(b3);
	i2c_send_byte(b4);
	i2c_stop();
	_delay_ms(5.0);
	
	if(err) return;
	uint8_t Buf[8];
	read_str(eeaddr, Buf);
	if(b1 != Buf[0]) err = 4;
	if(b2 != Buf[1]) err = 4;
	if(b3 != Buf[2]) err = 4;
	if(b4 != Buf[3]) err = 4;
}

void write_str(uint16_t eeaddr, uint8_t *buf)
{	
	i2c_start();
	if (eeaddr > 0xFF) i2c_send_addr(MS_ADDR | 2); else i2c_send_addr(MS_ADDR);
	i2c_send_byte(eeaddr);
	for(uint8_t i = 0; i < 8; i++) i2c_send_byte(buf[i]);
	i2c_stop();
	_delay_ms(5.0);
	
	if(err) return;
	uint8_t Buf[8];
	read_str(eeaddr, Buf);
	for(uint8_t i = 0; i < 8; i++)
	if(buf[i] != Buf[i]) {err = 4;	break;}// :-)
}



