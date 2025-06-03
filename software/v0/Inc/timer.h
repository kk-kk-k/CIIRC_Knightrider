#pragma once
#include "main.h"

typedef struct {
	unsigned int uiPrescaller;
	unsigned int uiPeriod;
} TimerConfig_t;

#define timerTIM_CFG_FROM(prescaller, period) \
	(TimerConfig_t) {.uiPrescaller = prescaller, .uiPeriod = period}

void vTimerUpdateValue(TIM_HandleTypeDef *const pxHtim, DMA_HandleTypeDef * const pxDmaChannel,
		const TimerConfig_t *const pxTimerConfig);
