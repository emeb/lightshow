/*
 * WS281x driver for STM32F030
 * E. Brombaugh 04-01-2015
 */

#include "stm32f0xx.h"
#include "adc.h"
#include "systick.h"
#include "ws281x.h"

/* WS281x RGB buffer */
uint8_t rgb_data[3*WS_MAX_LEDS];

int main(void)
{
	uint8_t hsv[3], temp;
	
	/* initialize the hardware */
	systick_init();
	adc_init();
	ws281x_init();
	
	/* Loop waiting for button push to speak */
	while(1)
	{
		/* send new colors to to the LED chain */
#if 0
		/* HSV based on ADC readings */
		hsv[0] = adc_get_data(0)>>4;
		hsv[1] = adc_get_data(1)>>4;
		hsv[2] = adc_get_data(2)>>4;
		ws281x_hsv2rgb(&rgb_data[0],  hsv);
		ws281x_hsv2rgb(&rgb_data[3],  hsv);
		ws281x_hsv2rgb(&rgb_data[6],  hsv);
		ws281x_hsv2rgb(&rgb_data[9],  hsv);
		ws281x_hsv2rgb(&rgb_data[12], hsv);
#else
		hsv[0] = (hsv[0] + 1)&0xff;		// hue cycles
		hsv[1] = 0xff;					// Saturation is max
		hsv[2] = adc_get_data(2)>>4;	// Brightness is from pot
#if 0
		/* all same color */
		ws281x_hsv2rgb(&rgb_data[0],  hsv);
		ws281x_hsv2rgb(&rgb_data[3],  hsv);
		ws281x_hsv2rgb(&rgb_data[6],  hsv);
		ws281x_hsv2rgb(&rgb_data[9],  hsv);
		ws281x_hsv2rgb(&rgb_data[12], hsv);
#else
		/* 5 different colors */
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
#endif
#endif
		ws281x_send(rgb_data, 5);

		/* wait a bit depending on pot */
		systick_delayms(adc_get_data(1)>>4);
	}
}
