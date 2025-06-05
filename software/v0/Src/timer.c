#include <assert.h>
#include "timer.h"

static unsigned int puiGpioPortAStates[] = {LED0_Pin, LED1_Pin};//, LED2_Pin, LED3_Pin, LED4_Pin, LED5_Pin, LED6_Pin};

void vTimerUpdateValue(TIM_HandleTypeDef *const pxHtim,
		DMA_HandleTypeDef *const pxDmaChannel,
		const TimerConfig_t *const pxTimerConfig) {
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	assert(pxHtim != NULL);
	assert(pxTimerConfig != NULL);
	assert(pxTimerConfig->uiPeriod <= 65535);
	assert(pxTimerConfig->uiPrescaller <= 65535);

	HAL_TIM_Base_Stop(pxHtim);

	pxHtim->Init.Prescaler = pxTimerConfig->uiPrescaller;
	pxHtim->Init.Period = pxTimerConfig->uiPeriod;
	if (HAL_TIM_Base_Init(pxHtim) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(pxHtim, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}

	HAL_DMA_Abort(pxDmaChannel);

	// start DMA access between puiGpioPortAStates and GPIOA_ODR
	HAL_DMA_Start(pxDmaChannel, (uint32_t) puiGpioPortAStates,
			(uint32_t) &GPIOA->ODR, sizeof(puiGpioPortAStates) / sizeof(*puiGpioPortAStates));

	// enable tim6 DMA event
	__HAL_TIM_ENABLE_DMA(pxHtim, TIM_DMA_UPDATE);

	// start tim6
	HAL_TIM_Base_Start(pxHtim);
}
