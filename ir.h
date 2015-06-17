/*
 * ir.h - stmf030 IR receiver
 * Some parts of code courtesy of jpa
 *       https://svn.kapsi.fi/jpa/led-controller/sw/src/remote.c
 * Rest by
 * E. Brombaugh 05-30-15
 */

#ifndef __ir__
#define __ir__

#include "stm32f0xx.h"

#define IR_RM_IDLE 0

#define IR_RM_PWR 11
#define IR_RM_VOLUP 12
#define IR_RM_CHUP 13

#define IR_RM_MUTE 21
#define IR_RM_VOLDN 22
#define IR_RM_CHDN 23

#define IR_RM_1 31
#define IR_RM_2 32
#define IR_RM_3 33

#define IR_RM_4 41
#define IR_RM_5 42
#define IR_RM_6 43

#define IR_RM_7 51
#define IR_RM_8 52
#define IR_RM_9 53

#define IR_RM_ZOOM 61
#define IR_RM_0 62
#define IR_RM_JUMP 63

/* unique codes for 2nd remote */
#define IR_RM_FULLSCRN 70
#define IR_RM_RCL 71
#define IR_RM_REC 72
#define IR_RM_TSHFT 73
#define IR_RM_SRC 74

void ir_init(void);
uint8_t ir_check_key(void);

#endif
