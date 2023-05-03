#include "twi_drv.h"
#include "reset.h"


uint8_t ID_CR_1[] = "CRUM";
uint8_t ID_CR_2[] = "crum";

uint8_t ID_STR_1[] = "SAMSUNG"; // 2150, 2250, 2550
uint8_t ID_STR_2[] = "SCX-";    // ML4520, 4720, 6320, R
uint8_t ID_STR_3[] = "C4200";	// R
uint8_t ID_STR_4[] = "XEROX";   // 3420, 3450
uint8_t ID_STR_5[] = "CLP510";
uint8_t ID_STR_6[] = "CLP600";
uint8_t ID_STR_7[] = "гXerox";  // 3450(addr60=00), 3150
uint8_t ID_STR_8[] = " XEROX "; // m20i, PE220, WC3119
uint8_t ID_STR_9[] = "ЁПXEROX"; // PE120, m20i ~~ sam4520 
uint8_t ID_STR_10[] = " XEROX2 ";// WC4118
uint8_t ID_STR_11[] = "Type103"; // Gestetner DSm 520, - MB OfficeCenter 220, Ricoh FX220 ~~ Xerox PE120, 
uint8_t ID_STR_12[] = "Tally";   // ?MB521?, Tally Genicom 9022 
uint8_t ID_STR_13[] = "P3560";		// R
uint8_t ID_STR_14[] = "XEROXг";
uint8_t ID_STR_15[] = "TOSHIBA";
uint8_t ID_STR_16[] = "DELL";		// 1600

uint8_t ID_REG[4][4] = {"EXP", "EUR", "CHN", "KOR"};

uint8_t Empty[8];

uint8_t ID_V[] = "(Ver 0.3)";


uint8_t cmp(uint8_t *Buf1, uint8_t *Buf2)
{ 
	while(*Buf2)
		if(*Buf1++ != *Buf2++)	return 0;
	return 1;
}

void ChangeMaxPages(uint16_t addr)	//максимальный ресурс
{
	uint8_t max_pages = read_byte(addr);
	if(((set_max_pages) && (max_pages) && (max_pages < 0x0D)) || ((set_max_pages) && (addr == 0x20))) write_byte(addr, set_max_pages);
	else if((max_pages || addr == 0x20) && (max_pages < 3)) write_byte(addr, 0x03);
}

void ChangeCrum(uint16_t addr)	 //здесь меняем серийный номер - до 2х цифр, если они вдруг почему-то испорчены, исправляем.
{
	uint8_t Buf[8];
	
	read_str(addr, Buf);
	if(!(cmp(Buf, ID_CR_1) || cmp(Buf, ID_CR_2))) Blink(5);
		
	uint8_t cr1 = read_byte(addr + 0x0E);
	uint8_t cr2 = read_byte(addr + 0x0F);
				
	cr2++;
	if((cr2 > 0x39) || (cr2 < 0x30))
		{
			cr2 = 0x30;
			cr1++;
		}
		
	if((cr1 > 0x39) || (cr1 < 0x30)) cr1 = 0x30;
	
	write_byte(addr + 0x0E, cr1);
	write_byte(addr + 0x0F, cr2);
	
//return 1;
}

void SetCounter(void)
{
	uint8_t max_pages = read_byte(0x20);

	if(max_pages == 0x01) write_4byte(0x40, 0x1A, 0xAA, 0x02, 0x3C); //0x0C, 0x8F, 0xC9, 0xB2
	if(max_pages == 0x03) write_4byte(0x40, 0x1A, 0xAA, 0x02, 0x3C);
	if(max_pages == 0x05) write_4byte(0x40, 0x2C, 0xB4, 0x17, 0x80); //0x2E, 0xB4, 0x9C, 0xDB
	if(max_pages == 0x06) write_4byte(0x40, 0x3E, 0x2A, 0xCA, 0xE0);
	if(max_pages == 0x08) write_4byte(0x40, 0x4A, 0xBA, 0x94, 0x92);
	if(max_pages == 0x0A) write_4byte(0x40, 0x5D, 0x69, 0x39, 0xB6);
	if(max_pages == 0x0C) write_4byte(0x40, 0x7D, 0x2B, 0x75, 0x00);
}

void SetRegID(uint16_t addr)
{
	for(uint16_t i = 0; i < 3; i++)
	write_byte(addr++, ID_REG[region - 1][i]);
}

void SetRegion(void)
{
	uint16_t addr;
	uint8_t Buf[8];

	read_str(0x00, Buf);
	
	if(cmp(Buf, ID_STR_2)) 
		{
			SetRegID(0x04);
					
			read_str(0x58, Buf);
			if(cmp(Buf, ID_STR_2)) 
				{
					SetRegID(0x5C);
										
					addr = 0x80;
					for(uint8_t p = 0; p < 3; p++)
						{
							read_str(addr, Buf);
							if(cmp(Buf, ID_STR_2)) 	SetRegID(addr + 0x04);
							read_str(addr + 0x58, Buf);
							if(cmp(Buf, ID_STR_2)) 	SetRegID(addr + 0x5C);
							addr += 0x80;
						}
					
					return;
				}
		}

	read_str(0x00, Buf);
	
	if(cmp(Buf, ID_STR_3)) 
		{
			SetRegID(0x05);
					
			read_str(0x58, Buf);
			if(cmp(Buf, ID_STR_3))
				{
					SetRegID(0x5D);
					return;
				}
		}
		
	read_str(0x00, Buf);
	
	if(cmp(Buf, ID_STR_13)) 
		{
			SetRegID(0x05);
					
			read_str(0x10, Buf);
			if(cmp(Buf, ID_STR_13))
				{
					SetRegID(0x15);
					return;
				}
		}
		
	if(!err) Blink(5);	
}
	
	

void reset_chip(void)
{
	uint16_t addr;
	uint8_t Buf[8];
	
	
	
					//Samsung 4200,  WC3119//
					
	/*read_str(0x00, Buf);
					
	if(cmp(Buf, ID_STR_3) || cmp(Buf, ID_STR_8))		// проверка соответствия моделей
		{	
			ChangeMaxPages(0x20);						// меняем число копий по необходимости
			ChangeCrum(0x30);							// есть "crum" -> пробуем сменить SN
			
			write_str(0x48, Empty);						//
			write_str(0x50, Empty);						// обнуляем
			write_str(0x60, Empty);						// счетчики
			write_byte(0x78, 0x00);						//
							
			if(!err) Blink(8);							// сигнал "OK"
		}*/



	if(region) SetRegion();
					
					//Samsung 2150, 2250, 2550, 3560,  XEROX 3150, 3420, 3450, 3500//
					
	read_str(0x00, Buf);
					
	if(cmp(Buf, ID_STR_1) || cmp(Buf, ID_STR_4) || cmp(Buf, ID_STR_7) || cmp(Buf, ID_STR_13))
		{
			read_str(0x10, Buf);
			if(cmp(Buf, ID_STR_1) || cmp(Buf, ID_STR_4) || cmp(Buf, ID_STR_7) || cmp(Buf, ID_STR_13))
				{	
					if(read_byte(0x60) == 0x00)
						{
							ChangeMaxPages(0x20);
							ChangeCrum(0x30);
							SetCounter();
							
							for(addr = 0x50; addr < 0x54; addr++)
							write_byte(addr, 0x00);
							
							if(read_byte(0x70) != 0xFF)	// Xerox 3500, Samsung 3560
								{
									for(addr = 0x70; addr < 0x74; addr++)
									write_byte(addr, 0x00);
															
									for(addr = 0x58; addr < 0x5C; addr++)
									write_byte(addr, 0x00);
								}
								
							for(addr = 0x80; addr < 0x84; addr++)
							write_byte(addr, 0x00);
							
							if(!err) Blink(8);
						}
										
								//Xerox 3150//
								
					if(read_byte(0x60) == 0xFF)
						{
							ChangeMaxPages(0x20);
							ChangeCrum(0x30);
							
							for(addr = 0x40; addr < 0x44; addr++)
							write_byte(addr, 0x00);
							
							for(addr = 0x50; addr < 0x54; addr++)
							write_byte(addr, 0x00);

							if(!err) Blink(8);
						}
					
				}

		}
		
		
					

		
				// Tally 9022, MB521-???//
				
	read_str(0x00, Buf);
				
	if(cmp(Buf, ID_STR_12))
		{
			read_str(0x10, Buf);
			if(cmp(Buf, ID_STR_12))
				{
					ChangeMaxPages(0x20);
					ChangeCrum(0x30);
					SetCounter();
					
					for(addr = 0x50; addr < 0x54; addr++)
					write_byte(addr, 0x00);
					
					//for(addr = 0x80; addr < 0x84; addr++)	// ??? Возможно, не требуется.
					//write_byte(addr, 0xFF);
					
					if(!err) Blink(8);
				}
			
		}


		
	
		
					//Samsung 4200(+addr 60), 4520, 4720, 6320, Xerox M20i, PE120, PE220, wc3119(+addr 60), Type103, 4118, DELL 1600//
					
	read_str(0x00, Buf);
	
	if(cmp(Buf, ID_STR_2) || cmp(Buf, ID_STR_3) || cmp(Buf, ID_STR_8)  || cmp(Buf, ID_STR_9) || cmp(Buf, ID_STR_10) || cmp(Buf, ID_STR_11) || cmp(Buf, ID_STR_15)  || cmp(Buf, ID_STR_16))
		{
			ChangeCrum(0x30);
			ChangeMaxPages(0x20);
			
			uint8_t cr1 = read_byte(0x3E);
			uint8_t cr2 = read_byte(0x3F);
					
			read_str(0xB0, Buf);
			if(cmp(Buf, ID_CR_1) || cmp(Buf, ID_CR_2)) 
				{
					ChangeMaxPages(0xA0);
					
					write_byte(0xBE, cr1);
					write_byte(0xBF, cr2);
					write_byte(0xD7, 0x00);
				}
				
			read_str(0x130, Buf);
			if(cmp(Buf, ID_CR_1) || cmp(Buf, ID_CR_2)) 
				{
					write_byte(0x13E, cr1);
					write_byte(0x13F, cr2);
				}
								
			read_str(0x1B0, Buf);
			if(cmp(Buf, ID_CR_1) || cmp(Buf, ID_CR_2)) 
				{
					write_byte(0x1BE, cr1);
					write_byte(0x1BF, cr2);
				}
					
			write_str(0x48, Empty);
			write_str(0x50, Empty);
			write_str(0x60, Empty); // S4200, X3119, PE220
			for(addr = 0x74; addr < 0x79; addr++)
			write_byte(addr, 0x00);
			
			if(!err) Blink(8);
		}
		
		
		
	if (!err) Blink(5);

						//---//
}
