
#include <util/twi.h>



extern void i2c_init(void);
//extern void i2c_start(void);
extern void i2c_stop(void);

extern void Blink(uint8_t);						// мигалка светодиодом
extern uint8_t read_byte(uint16_t);
extern void read_str(uint16_t, uint8_t *);
extern void write_byte(uint16_t, uint8_t);		// Операции записи 
extern void write_str(uint16_t, uint8_t *);		// завершаются
extern void write_4byte(uint16_t, uint8_t,		// проверкой.
				uint8_t, uint8_t, uint8_t);		//
extern void chip_wait(void);

uint8_t err;



