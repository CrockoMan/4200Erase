//************************************************************************************
// Target MCU: Atmega8, default settings.
//************************************************************************************


#include <avr/io.h>
#include <util/delay.h>

#include "twi_drv.h"
#include "eeprom_save.h"
#include "reset.h"

#define SetBit(x)    |= (1<<x) 
#define ClearBit(x)  &=~(1<<x) 
#define InvertBit(x) ^= (1<<x) 


void KeysCtrl(void)
{
	uint8_t KEYS = 0;
	
	KEYS = PIND;
	
	if(KEYS != 0xFF)
	{
		_delay_ms(150);
		if(KEYS == PIND) // from CB
			{
				switch (KEYS)
					{
						case 0xFE:
						set_max_pages = 0x01;
						break;
						
						case 0xFD:
						set_max_pages = 0x03;
						break;
						
						case 0xFC:
						set_max_pages = 0x05;
						break;
						
						case 0xFB:
						set_max_pages = 0x06;
						break;
						
						case 0xFA:
						set_max_pages = 0x08;
						break;
						
						case 0xF9:
						set_max_pages = 0x0A;
						break;
						
						case 0xF8:
						set_max_pages = 0x0C;
						break;
						
						
						case 0xDE:
						region = 1;
						break;
						
						case 0xDD:
						region = 2;
						break;
						
						case 0xDC:
						region = 3;
						break;
						
						case 0xDB:
						region = 4;
						break;
						
						
						default:
						break;
					}
/*					
				switch (KEYS | 0xE7)
					{
						case 0xEF:
						M_Restore();
						break;
						
						case 0xF7:
						M_Save();
						break;
						
						default:
						break;
					}
*/					
			}
	}
}


int main(void)
{	
	PORTB = PORTC = PORTD = 0xFF;
	uint8_t res_counter = 50;		//число попыток связи
	uint8_t resource = 0xFF;		//число оставшихся перепрошивок
	set_max_pages = 0;

	DDRC = (1 << PC3) | (1 << PC2);
	PORTC SetBit(3);				// Красный светодиод
	PORTC ClearBit(2);				// Зеленый светодиод

	resource = ee_read(1);

//	if(resource==0xFF)	resource=100;
	
	if(resource>0)					// Если есть разрешенные перепрошивки
	{
//		ee_write(resource-1, 1);
		
		_delay_ms(750);
		i2c_init();
	
		while(res_counter)
		{
			chip_wait();			//ожидание соединения с чипом
			KeysCtrl();				//проверка переключателей
			reset_chip();
			res_counter--;
			PORTC InvertBit(3);		// Красный светодиод
		}
		
		Blink(err);
	}
		
}









