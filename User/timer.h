#ifndef __TIMER_H
#define __TIMER_H
 #define START_TIME5 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , ENABLE);TIM_Cmd(TIM5, ENABLE)
#define STOP_TIME5  TIM_Cmd(TIM5, DISABLE);RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , DISABLE)


void time2_init(void);
void TIM2_IRQHandler(void);
void time3_init(void);
void TIM5_NVIC_Configuration(void);
void TIM5_Configuration(void);
void time4_init(void);
#endif
