/*
 * WS281x driver for STM32F030
 * E. Brombaugh 04-01-2015
 */

#include "stm32f0xx.h"
#include "adc.h"
#include "systick.h"
#include "ws281x.h"
#include "ir.h"

/* WS281x RGB buffer */
uint8_t rgb_data[3*WS_MAX_LEDS];

/* linear interpolate shift to one of five */
void shift5(uint8_t shift, uint8_t *out_array)
{
	uint16_t ll, lh, fl, fh, i;
	
	/* which outputs are active? */
	ll = shift/52;
	lh = (ll+1)%5;
	
	/* what fractions at each */
	fh = (shift%52)*5;
	fh = (fh>255)?255:fh;
	fl = 255-fh;
	
	/* clear the output array */
	for(i=0;i<5;i++)
		out_array[i]=0;
	
	/* set the active elements */
	out_array[ll] = fl;
	out_array[lh] = fh;
}

/* main */
int main(void)
{
	uint8_t algo, bright, speed, ldr, temp;
	uint8_t hsv[3], rgb_temp[3], shift_array[5];
	uint8_t shift, i, j;
	uint8_t code, power_state = 1;
	
	/* initialize the hardware */
	systick_init();
	adc_init();
	ws281x_init();
	ir_init();
	
	/* init the local state */
	hsv[0] = 0;
	hsv[1] = 0;
	hsv[2] = 0;
	shift = 0;
	
	/* Loop waiting for button push to speak */
	while(1)
	{
		/* get pot parameters */
		algo = (adc_get_data(0)>>9)&7;	// for 8 modes
		bright = adc_get_data(1)>>4;
		speed = 1+(64/(64-(adc_get_data(2)>>6)));	// linearizes speed
		ldr = adc_get_data(3)>>4;
		
		/* compute new colors by algorithm */
		switch(algo)
		{
			case 0:
				/* 5 different colors chase around */
				hsv[0] = (hsv[0] + 1)&0xff;		// next hue
				hsv[1] = 0xff;					// Saturation is max
				hsv[2] = bright;				// Brightness is from pot
			
				/* Assign colors */
				ws281x_hsv2rgb(&rgb_data[0],  hsv);
				temp = hsv[0];
				hsv[0] = (hsv[0] + 51)&0xff;		// advance hue 256/5
				ws281x_hsv2rgb(&rgb_data[3],  hsv);
				hsv[0] = (hsv[0] + 51)&0xff;		// advance hue 256/5
				ws281x_hsv2rgb(&rgb_data[6],  hsv);
				hsv[0] = (hsv[0] + 51)&0xff;		// advance hue 256/5
				ws281x_hsv2rgb(&rgb_data[9],  hsv);
				hsv[0] = (hsv[0] + 51)&0xff;		// advance hue 256/5
				ws281x_hsv2rgb(&rgb_data[12], hsv);		
				hsv[0] = temp;						// reset hue
				break;
			
			case 1:
				/* one color cycles */
				hsv[0] = (hsv[0] + 1)&0xff;		// next hue
				hsv[1] = 0xff;					// Saturation is max
				hsv[2] = bright;				// Brightness is from pot
			
				/* all 5 are the same */
				ws281x_hsv2rgb(&rgb_data[0],  hsv);
				ws281x_hsv2rgb(&rgb_data[3],  hsv);
				ws281x_hsv2rgb(&rgb_data[6],  hsv);
				ws281x_hsv2rgb(&rgb_data[9],  hsv);
				ws281x_hsv2rgb(&rgb_data[12], hsv);
				break;

			case 2:
				/* one color selected by bright pot */
				hsv[0] = bright;		// Hue is from pot
				hsv[1] = 0xff;			// Saturation is max
				hsv[2] = 0xff;			// Brightness is max
			
				/* all 5 are the same */
				ws281x_hsv2rgb(&rgb_data[0],  hsv);
				ws281x_hsv2rgb(&rgb_data[3],  hsv);
				ws281x_hsv2rgb(&rgb_data[6],  hsv);
				ws281x_hsv2rgb(&rgb_data[9],  hsv);
				ws281x_hsv2rgb(&rgb_data[12], hsv);
				break;
				
			case 3:
				/* one of 5 interpolated */
				shift5(shift, shift_array);
				hsv[0] = bright;		// Hue is from pot
				hsv[1] = 0xff;			// Saturation is max
				for(i=0;i<5;i++)
				{
					hsv[2] = shift_array[i];	// Brightness is from shift
					ws281x_hsv2rgb(&rgb_data[i*3],  hsv);
				}
				shift++;
				break;
				
			case 4:
				/* two of 5 interpolated, offset by 180, complementary */
				hsv[1] = 0xff;			// Saturation is max
				
				/* first overrides */
				shift5(shift, shift_array);
				hsv[0] = bright;		// Hue is from pot
				for(i=0;i<5;i++)
				{
					hsv[2] = shift_array[i];	// Brightness is from shift
					ws281x_hsv2rgb(&rgb_data[i*3],  hsv);
				}
				
				/* second adds */
				shift5(shift+128, shift_array);	// opposite LED
				hsv[0]+=128;			// complementary color
				for(i=0;i<5;i++)
				{
					hsv[2] = shift_array[i];	// Brightness is from shift
					ws281x_hsv2rgb(&rgb_temp,  hsv);
					rgb_data[i*3+0] += rgb_temp[0];
					rgb_data[i*3+1] += rgb_temp[1];
					rgb_data[i*3+2] += rgb_temp[2];
				}
				shift++;
				break;
		}
		
		/* check IR */
		if((code = ir_check_key()))
		{
			/* toggle power state */
			if(code == IR_RM_PWR)
				power_state = 1-power_state;
		}
		
		if(power_state)
		{
			/* send new colors to to the LED chain */
			ws281x_send(rgb_data, 5);
		}
		else
		{
			/* send all off */
			for(i=0;i<15;i++)
				rgb_data[i] = 0;
			ws281x_send(rgb_data, 5);
		}
		
		/* wait a bit depending on speed setting */
		systick_delayms(speed);
	}
}
