
#include <avr/io.h>
#include <util/delay.h>

#include "twi_drv.h"
#include "eeprom_save.h"

uint8_t Empt[] = "Empty!!!";
uint8_t en = 0;	// в "1" - считано в буфер

void ee_write_block(uint8_t *buf, uint16_t uiAddress)
{
	for(uint8_t i = 0; i < 8; i++)
	{
		while(EECR & (1<<EEWE));
		EEAR = uiAddress++;
		EEDR = buf[i];
		EECR |= (1<<EEMWE);
		EECR |= (1<<EEWE);
	}
}

void ee_write(uint8_t buf, uint16_t uiAddress)
{
	while(EECR & (1<<EEWE));
	EEAR = uiAddress++;
	EEDR = buf;
	EECR |= (1<<EEMWE);
	EECR |= (1<<EEWE);
}


uint8_t ee_read(uint16_t uiAddress)
{
	while(EECR & (1<<EEWE));
	EEAR = uiAddress;
	EECR |= (1<<EERE);
	return EEDR;
}


void M_Save(void)
{
	uint8_t Buf[8];
	uint8_t Buf2[512];
		
	for(uint16_t i = 0; i < 512; i += 8)
	{
		read_str(i, Buf);
		for(uint8_t j = 0; j < 8; j++) Buf2[i+j] = Buf[j];
	}
	
	for(uint16_t i = 0; i < 512; i += 8)
	{
		read_str(i, Buf);
		if(err) return;
		for(uint8_t j = 0; j < 8; j++) if(Buf2[i+j] != Buf[j]) Blink(4); // проверка считывания из i2c
	}
	
	if(err) return;
	
	ee_write_block(Empt, 0x1F8);
	
	DDRC = (1 << PC3);
	PORTC = (1 << PC3);
	_delay_ms(100);
	PORTC = (0 << PC3);
	
	for(uint16_t i = 0; i < 512; i += 8)
	{
		for(uint8_t j = 0; j < 8; j++) Buf[j] = Buf2[i+j];
		ee_write_block(Buf, i);
		
	}
	
	for(uint16_t i = 0; i < 512; i++)	if(Buf2[i] != ee_read(i)) Blink(4); // проверка архивации данных 
	
	Blink(8);
}

void M_Restore(void)
{
	uint8_t Buf[8], Buf2[8];
		
	uint8_t x = 0;
	for(uint8_t i = 0; i < 8; i++)	if(Empt[i] == ee_read(i + 0x1F8)) x++;
	if(x == 8) Blink(2);
	
	x = 0;
	read_str(0x00, Buf);
	for(uint16_t i = 0; i < 8; i++) if(Buf[i] == ee_read(i)) x++;
	if((x != 8) && (!en))
		{
			DDRC = (1 << PC3);
			PORTC = (1 << PC3);
			_delay_ms(250);
			PORTC = (0 << PC3);
			_delay_ms(250);
			DDRC = (1 << PC3);
			PORTC = (1 << PC3);
			_delay_ms(250);
			PORTC = (0 << PC3);
			_delay_ms(2000);
			en = 1;
		}
		
	for(uint16_t i = 0; i < 512; i += 8)
	{
		for(uint8_t j = 0; j < 8; j++) Buf[j] = ee_read(i+j);
		write_str(i, Buf);
		if(err) return;
	}

	
	for(uint16_t i = 0; i < 512; i += 8)
	{
		read_str(i, Buf2);
		if(err) return;
		for(uint8_t j = 0; j < 8; j++) if(Buf2[j] != ee_read(i+j)) Blink(4); // проверка восстановления данных 
	}
	
	if(!err) Blink(8);
}


	
