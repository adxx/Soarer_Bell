/*
 * Bell.c
 *
 * Зуммер "Колокольчик"
 * Attiny13a, 9.6Mhz
 * 
 * Author: Погребняк Дмитрий, г. Самара, 2014
 *
 * Помещённый здесь код является свободным. Т.е. допускается его свободное использование для любых целей,
 * включая коммерческие, при условии указания ссылки на автора (Погребняк Дмитрий, http://aterlux.ru/).
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define Flasher PINB3
#define Doors	PINB4

#define WAVE1_FADE_STEP 200	//120 - good for turn signal
#define WAVE1_FADE_STEP_SHORT 120	
#define WAVE2_FADE_STEP 150		//80 - good for turn signal
#define WAVE2_FADE_STEP_SHORT 80
 

#define WAVE1_INITIAL_AMP 180
#define WAVE2_INITIAL_AMP 200


volatile uint8_t status	= 0;
volatile uint8_t wave1_fade	= 200;
volatile uint8_t wave1_fade_short = 120;
volatile uint8_t wave2_fade	= 150;
volatile uint8_t wave2_fade_short = 80;


volatile uint8_t wave_pos = 0;
volatile uint8_t w1amp = 0;
volatile uint8_t w2amp = 0;
volatile uint8_t w1step = 0;
volatile uint8_t w2step = 0;
volatile uint8_t a = 128;


// возвращает знаковое произведение знакового и беззнакового параметров, сдвинутое вправо на 8 разрядов
extern int8_t mul_s_u_shr8(int8_t, uint8_t); 

// Частота тона = 9600000 (частота МК) / 256 (период таймера) / размер_массива = 37500 / размер_массива
// Колокольчик 585 Гц, размер массива = 64
const int8_t wave3[] PROGMEM = { // Форманта (синусоида)
  0, 6, 12, 19, 24, 30, 36, 41, 45, 49, 53, 56, 59, 61, 63, 64, 64, 
   64, 63, 61, 59, 56, 53, 49, 45, 41, 36, 30, 24, 19, 12, 6, 0, 
   -6, -12, -19, -24, -30, -36, -41, -45, -49, -53, -56, -59, -61, -63, -64, -64, 
   -64, -63, -61, -59, -56, -53, -49, -45, -41, -36, -30, -24, -19, -12, -6
};

const int8_t wave4[] PROGMEM = { // Обертоны
  0, 27, 47, 59, 62, 59, 53, 45, 37, 28, 20, 12, 6, -1, -6, -11, -14, 
   -18, -20, -22, -23, -23, -23, -22, -21, -19, -17, -15, -12, -10, -6, -3, 0, 
   3, 6, 10, 12, 15, 17, 19, 21, 22, 23, 23, 23, 22, 20, 18, 14, 
   11, 6, 1, -6, -12, -20, -28, -37, -45, -53, -59, -62, -59, -47, -27
};


// Колокольчик 893 Гц, размер массива = 42
const int8_t wave1[] PROGMEM = { // Форманта (синусоида)
  0, 10, 19, 28, 36, 44, 50, 55, 60, 62, 64, 64, 62, 60, 55, 50, 44, 
   36, 28, 19, 10, 0, -10, -19, -28, -36, -44, -50, -55, -60, -62, -64, -64, 
   -62, -60, -55, -50, -44, -36, -28, -19, -10
};

const int8_t wave2[] PROGMEM = { // Обертоны
  0, 12, 23, 32, 38, 41, 42, 42, 42, 40, 38, 36, 34, 32, 30, 28, 26, 
   24, 22, 20, 18, 16, 15, 13, 12, 10, 9, 7, 6, 4, 3, 1, 0, 
   -1, -3, -4, -6, -7, -9, -10, -12, -13
};




int main(void)
{
  OSCCAL = 0x5D; //калибровка кривого генератора китайской тиньки. 
  DDRB = (1 << PORTB0); //|(1 << PORTB4);
  //PORTB = (1 << PINB3)|(1 << PINB4);
  
  
  
  OCR0A = 128;
  //TCCR0A = 0b11000011;
  TCCR0B = 0b00000001;
  
  PORTB &= ~(1 << PINB0); 
  
  while(1)
  {
	
	if(status == 0) {
		if((PINB & (1<<Flasher)) != 0) {
			TCCR0A = 0b11000011;
			status = 1;
		}
		
		if((PINB & (1<<Doors)) == 0) {
			TCCR0A = 0b11000011;
			status = 2;
		}
			
	}  
	
	
	
		if (status != 0){
		  if (status == 1) {
			if (!w1step) {
				w1step = wave1_fade_short;
				if (!w1amp) {
					w1amp = WAVE1_INITIAL_AMP;
					w2amp = WAVE2_INITIAL_AMP;
					w2step = wave2_fade_short;
					wave_pos = 0;
					} else {
					w1amp--;
				}
				} else {
				w1step--;
			}
			if (!w2step) {
				w2step = wave2_fade_short;
				if (w2amp) {
					w2amp--;
				}
				} else {
				w2step--;
			}	
		  
				a = 128 +
				mul_s_u_shr8((int8_t)pgm_read_byte(&wave1[wave_pos]), w1amp) +
				mul_s_u_shr8((int8_t)pgm_read_byte(&wave2[wave_pos]), w2amp);
				wave_pos++;
				if (wave_pos >= sizeof(wave1)){
					wave_pos = 0;
				}
		  }
			
		  if (status == 2){
			    if (!w1step) {
				    w1step = wave1_fade;
				    if (!w1amp) {
					    w1amp = WAVE1_INITIAL_AMP;
					    w2amp = WAVE2_INITIAL_AMP;
					    w2step = wave2_fade;
					    wave_pos = 0;
					    } else {
					    w1amp--;
				    }
				    } else {
				    w1step--;
			    }
			    if (!w2step) {
				    w2step = wave2_fade;
				    if (w2amp) {
					    w2amp--;
				    }
				    } else {
				    w2step--;
			    }
			    
				a = 128 +
				mul_s_u_shr8((int8_t)pgm_read_byte(&wave1[wave_pos]), w1amp) +
				mul_s_u_shr8((int8_t)pgm_read_byte(&wave2[wave_pos]), w2amp);
				wave_pos++;
				if (wave_pos >= sizeof(wave1)){
					wave_pos = 0;
				}
		  }
			while (!(TIFR0 & (1 << TOV0)));
			OCR0A = a;
			TIFR0 |= (1 << TOV0);
			//TODO:: Please write your application code
			if (!w1amp){
				status = 0;
				TCCR0A = 0b00000011;
				PORTB &= ~(1 << PINB0); // PB1 goes low to remove noise
			}
			
			//PORTB ^= 1 << PINB4;
		}	
	
	

	
  }
}
