/*
 * ir.c - stmf030 IR receiver
 * Some parts of code courtesy of jpa
 *       https://svn.kapsi.fi/jpa/led-controller/sw/src/remote.c
 * Rest by
 * E. Brombaugh 05-30-15
 */

#include "ir.h"

uint16_t ir_rise_timer, ir_fall_timer; /* rise and fall time indices */
uint16_t ir_num_bits; /* Number of bits received */
volatile uint32_t ir_remote_code; /* The code received */
uint8_t ir_prev_received;	/* time of previous code & check */
volatile uint8_t ir_remote_button; /* decoded button */

/*
 * Initialize the IR receiver
 */
void ir_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	
	/* init state */
	ir_rise_timer = 0;
	ir_fall_timer = 0;
	ir_num_bits = 99;
	ir_remote_code = 0;
	ir_prev_received = 0;
	ir_remote_button = 0;
	
	/* turn on clock for IR input GPIO */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	/* Enable PA4 for input to TIM14 */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Connect PA4 to TIM14_CH1 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_4);

	/* TIM14 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14 , ENABLE);
	
	/* TIM14 Time Base configuration for 1MHz -> microsecond resolution */
	TIM_TimeBaseStructure.TIM_Prescaler = 47;	// 1MHz clock rate
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);

	/* TIM14 Input Capture setup */
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInit(TIM14, &TIM_ICInitStructure);

	/* TIM14 enable counter */
	TIM_Cmd(TIM14, ENABLE);

	/* Enable the CC1 Interrupt Request */
	TIM_ITConfig(TIM14, TIM_IT_CC1, ENABLE);

	/* Enable the TIM14 global Interrupt */
	NVIC_EnableIRQ(TIM14_IRQn);
}

/*
 * Check for new keypress
 */
uint8_t ir_check_key(void)
{
	if(ir_prev_received)
	{
		/* new code has arrived */
		ir_prev_received = 0;
		return ir_remote_button;
	}
	
	return 0;
}

/*
 * Decode the codes into button indexes
 */
uint8_t ir_decode_code(uint32_t code)
{
    switch (code)
    {
        case 0xed127f80: return  IR_RM_PWR;
        case 0xe51a7f80: return  IR_RM_VOLUP;
        case 0xe11e7f80: return  IR_RM_CHUP;

        case 0xfe017f80: return  IR_RM_MUTE;
        case 0xfd027f80: return  IR_RM_VOLDN;
        case 0xfc037f80: return  IR_RM_CHDN;

        case 0xfb047f80: return  IR_RM_1;
        case 0xfa057f80: return  IR_RM_2;
        case 0xf9067f80: return  IR_RM_3;

        case 0xf8077f80: return  IR_RM_4;
        case 0xf7087f80: return  IR_RM_5;
        case 0xf6097f80: return  IR_RM_6;

        case 0xf50a7f80: return  IR_RM_7;
        case 0xe41b7f80: return  IR_RM_8;
        case 0xe01f7f80: return  IR_RM_9;

        case 0xf30c7f80: return  IR_RM_ZOOM;
        case 0xf20d7f80: return  IR_RM_0;
        case 0xf10e7f80: return  IR_RM_JUMP;
        
        default: return IR_RM_IDLE;
    }
}

/*
 * IR decoder - called by ISR
 */
void ir_state_mchn(uint16_t width)
{
    if (width > 4000 && width < 5000)
    {
        // Start bit
        ir_num_bits = 0;
        ir_remote_code = 0;
    }
    else if (width > 2000 && width < 3000)
    {
        /* only repeat on some keys */
		if((ir_remote_button == IR_RM_VOLUP) |
			(ir_remote_button == IR_RM_CHUP) |
			(ir_remote_button == IR_RM_VOLDN) |
			(ir_remote_button == IR_RM_CHDN))
		{
			ir_num_bits = 99; // Repeat
			ir_prev_received = 1;
		}
    }
    else if (width > 1000 && width < 2000)
    {
        // '1' bit
        ir_remote_code |= (1 << ir_num_bits);
        ir_num_bits++;
    }
    else if (width > 300 && width < 800)
    {
        // '0' bit
        ir_num_bits++;
    }
    else
    {
        ir_num_bits = 99; // Invalid pulse
    }
    
    if (ir_num_bits == 32)
    {
		// last bit
        ir_remote_button = ir_decode_code(ir_remote_code);
        ir_prev_received = 1;
    }
}

/*
 * Timer 14 ISR
 */
void TIM14_IRQHandler(void)
{
	uint16_t width;
	
	if(TIM_GetITStatus(TIM14, TIM_IT_CC1) == SET) 
	{
		/*
		 * No need to explicitly clear TIM14 Capture compare
		 * interrupt pending bit since reading CCR1 does it implicitly
		 */
		//TIM_ClearITPendingBit(TIM14, TIM_IT_CC1);
	  
		/* check state of PA4 to determine if this is rising or falling edge */
		if(GPIOA->IDR & 0x10)
		{
			/* rising edge */
			ir_rise_timer = TIM_GetCapture1(TIM14);
		}
		else
		{
			/* falling edge */
 			ir_fall_timer = TIM_GetCapture1(TIM14);
			
			/* high pulsewidth computation */
			if (ir_fall_timer > ir_rise_timer)
			{
				width = (ir_fall_timer - ir_rise_timer); 
			}
			else if (ir_fall_timer < ir_rise_timer)
			{
				width = ((0xFFFF - ir_rise_timer) + ir_fall_timer); 
			}
			else
			{
				width = 0;
			}
			
			/* send the result to the state machine */
			ir_state_mchn(width);
		}
	}
}

