#pragma once

#include "main.h"

typedef struct {
	unsigned int uiPrescaller;
	unsigned int uiPeriod;
} TimerConfig_t;


#define timerMAX_PRESCALLER 65535
#define timerMAX_PERIOD 65535

#define timerTIM_CFG_FROM(prescaller, period) \
	(TimerConfig_t) {.uiPrescaller = prescaller, .uiPeriod = period}

void TIMER_UpdateValue(TIM_HandleTypeDef *const pxHtim, DMA_HandleTypeDef * const pxDmaChannel,
		const TimerConfig_t *const pxTimerConfig);
