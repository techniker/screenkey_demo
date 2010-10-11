//# Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
//# This code is licensed under GNU/ GPL

#include <avr/io.h>
#include <avr/interrupt.h>


volatile uint8_t phase;
volatile uint16_t bits;
volatile uint8_t cnt;
volatile uint16_t fcnt=0;

// This file is designed to be used on an atmega 328, running @ 16mhz,
// but should be portable to other processors
// Pin assignments are:
// 	PB7 - Button input [button drives pin low]
// 	PB1 - Data - Arduino PIN 10 - Arduino PIN 9
// 	PB0 - Clock - Arduino PIN 10

//IMAGES

uint8_t start_img[108] = {
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 
	0x3f, 0x7c, 0x78, 0xf8, 
	0xff, 0x3f, 0x7f, 0x7e, 0xfe, 
	0xff, 0xf7, 0xef, 0xef, 
	0xff, 0xff, 0xfe, 0xfd, 0xfd, 
	0xff, 0xef, 0xdf, 0xdf, 
	0xff, 0xff, 0xfe, 0xfd, 0xfd, 
	0xff, 0xef, 0xdf, 0xdf, 
	0xff, 0x7f, 0xff, 0xfe, 0xfe, 
	0xff, 0xf7, 0xef, 0xef, 
	0xff, 0x83, 0x7, 0x7, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0x0, 0x0, 0x0,
};

uint8_t stop_img[108] = {
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0x3f, 0x0, 0xc0, 
	0xff, 0xff, 0xbb, 0xfb, 
	0xfd, 0xff, 0xbf, 0x9b, 0xdf, 
	0xff, 0x7e, 0x93, 0xfd, 
	0xfd, 0xb, 0x80, 0xe4, 0xdf, 
	0x5f, 0xa0, 0xa0, 0xff, 
	0xfd, 0x0, 0x41, 0xf0, 0xdf, 
	0xf, 0x8, 0xc0, 0xfc, 
	0xfd, 0x4, 0x7a, 0xd3, 0xdf, 
	0x7, 0xfd, 0xb9, 0xf9, 
	0xfd, 0xca, 0xbf, 0x3b, 0xdf, 
	0xf, 0xfe, 0x73, 0xfd, 
	0xfc, 0xd0, 0x3f, 0x0, 0xc0, 
	0xe7, 0xff, 0xff, 0xff, 
	0x2f, 0xf9, 0xff, 0xff, 0xff, 
	0x10, 0xff, 0xff, 0xff, 
	0x7f, 0xf7, 0xff, 0xff, 0xff, 
	0x57, 0xfe, 0xff, 0xff, 
	0x7f, 0xed, 0xff, 0xff, 0xff, 
	0x8f, 0xff, 0xff, 0xff, 
	0xff, 0xe5, 0xff, 0xff, 0xff, 
	0xbf, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0x0, 0x0, 0x0,
};


//---------------------------------------------

//Interrupt service routine
ISR(TIMER1_OVF_vect){
  fcnt++;
  TCNT1 = 65536-80;

  phase = ~phase;

   PORTB = (PORTB & 0XFE) | (phase?1:0 ); //0xFE
   
  if (phase)
  {
  	if (!cnt)
	{
		PORTB |= 0x2;
		return;
	}

	PORTB = (PORTB & 0xFD) | ((bits & 0x1) ? 2:0);
  	cnt --;
	bits = bits >> 1;
  }
  
}

/* Prepares a word to be sent to the lcd */
/* Parity is 1 for odd parity, 0 for even */
void screenkey_write(uint8_t data, uint8_t parity)
{
	/* Block while theres still a word being sent */
	while (cnt);

	/* Calculate parity */
	uint8_t pb = data ^ (data >> 1) ^ (data >> 2) ^ (data >> 3) ^
		(data >> 4) ^ (data >> 5) ^ (data >> 6) ^ (data >> 7)
		^ parity;

	/* Setup the bits to send */
	bits = 0x0C00 | (data << 1) | ((pb & 0x1) << 9);

	/* queue it up */
	cnt = 12;
}

/* Start / Stop characters */
void screenkey_start()
{
	screenkey_write(0x00, 0);
}

void screenkey_stop()
{
	screenkey_write(0xAA, 0);
}

// Write a 1 byte register
void screenkey_reg_1(uint8_t reg, uint8_t val)
{
	screenkey_start();
	screenkey_write(reg,1);
	screenkey_write(val,1);
	screenkey_stop();

}

// Write a 2 byte register
void screenkey_reg_2(uint8_t reg, uint8_t val1, uint8_t val2)
{
	screenkey_start();
	screenkey_write(reg,1);
	screenkey_write(val1,1);
	screenkey_write(val2,1);
	screenkey_stop();
}


// Write a 108 byte image to the screen
// Data is a pointer to 108 bytes of image data to display
void screenkey_write_img(uint8_t * data)
{
	
	uint8_t i;

	screenkey_start();
	screenkey_write(0x80,1);
	for (i=0; i<108; i++)
		screenkey_write(data[i], 1);

	screenkey_stop();

}

// Color constants Bright and Dark -Register 0xED
#define OFF      0x00
#define DK_GREEN 0x04
#define BR_GREEN 0x44
#define DK_RED   0x02
#define BR_RED	 0x22 //22
#define DX_BLUE  0x01
#define BR_BLUE  0x11
#define BR_YELW  0x26
#define DX_MAGN  0x03
#define BR_MAGN  0x33
#define BR_PNK   0x23
#define BR_CYN   0x05
#define BR_WHT   0x27

// Call with constants above, use | to make composite colors
// aka Blueish purple: BR_BLUE | DK_RED
void screenkey_set_color(uint8_t color)
{
	screenkey_reg_1(0xED, color); //color register
}


// Application specific
void show_start()
{
	screenkey_set_color(BR_GREEN);
	screenkey_write_img(start_img);
}

void show_stop()
{
	screenkey_set_color(BR_RED);
	screenkey_write_img(stop_img);
}

int main()
{
	enum {
		stopped,
		running,
		valve,
		x
	} state;

	uint8_t btn_db;
	
	uint8_t br_dim;

	cnt = 0;
	DDRB = 0x3; //3
	PORTB = 0x83; //83
	TIMSK1=0x01;

	TCCR1A = 0x00;

	TCCR1B = 0x01;
	TCCR1C = 0x0;

	sei(); //Global interrupt enable

	// Setup the control registers
	screenkey_reg_1(0xEE, 0x00);
	screenkey_reg_2(0xEF, 0x07, 0x00);
	
	state = stopped;
	show_start();

	while (1)
	{
		/* Button Debounce */
		uint8_t btn = PINB & 0x80;
		if (!btn)
		{
			if (btn_db < 0x4)
				btn_db ++;

			if (btn_db == 0x4)
			{
				btn_db = 0xFF;
				/* Button State */
				if (state == running)
				{
					state = stopped;
					show_start();
				}
				else if (state == stopped)
				{
					fcnt = 0;
					state = running;
					show_stop();
				}

			}	

		}
		else
			btn_db = 0;

		/* Blink animation for stop display */
		if (state==running)
		{
			if (fcnt & 0x8000) //8
				if (!br_dim)
				{
                                
					screenkey_set_color(BR_RED);
					//screenkey_write_img(start_img);
                                        br_dim = 1;
				}
			if (! (fcnt & 0x8000)) //8
				if (br_dim)
				{
					screenkey_set_color(BR_RED);
					//screenkey_write_img(stop_img);
					br_dim = 0;
				}
		}
	}
}
