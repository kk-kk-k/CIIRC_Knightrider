#include <assert.h>
#include "timer.h"

/**
 * LED states
 * Just enter LED pins, which should be enabled in each mode
 * The function TIMER_UpdateValue automatically determines how many states are available (see sizeof)
 */
//#define USE_BINARY	// comment/uncomment if you wish to use classical blinking
static unsigned int puiGpioPortAStates[] =
#ifndef USE_BINARY
		{ LED0_Pin, LED1_Pin, LED2_Pin, LED3_Pin, LED4_Pin, LED5_Pin, LED6_Pin };
#else
{ LED0_Pin, LED1_Pin, LED0_Pin | LED1_Pin, LED2_Pin, LED2_Pin | LED0_Pin, LED2_Pin | LED1_Pin, LED2_Pin | LED1_Pin | LED0_Pin, 0};
#endif

/**
 * @brief Update timer/counter value
 *
 * @param pxHtim		Timer/counter peripheral which should be updated
 * @param pxDmaChannel	DMA channel associated with Timer/counter peripheral (DMA is aborted and then run from beggining)
 * @param pxTimerConfig	New Timer/counter peripherial configuration
 */
void TIMER_UpdateValue(TIM_HandleTypeDef *const pxHtim,
		DMA_HandleTypeDef *const pxDmaChannel,
		const TimerConfig_t *const pxTimerConfig) {
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	// parameter check
	assert(pxTimerConfig->uiPeriod <= timerMAX_PERIOD);
	assert(pxTimerConfig->uiPrescaller <= timerMAX_PRESCALLER);
	assert(pxHtim != NULL);
	assert(pxTimerConfig != NULL);

	// stop timer
	HAL_TIM_Base_Stop(pxHtim);

	// stop DMA transfer
	HAL_DMA_Abort(pxDmaChannel);

	// reset all LEDs
	GPIOA->ODR = 0;

	// initialize timer
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

	// start DMA access between puiGpioPortAStates and GPIOA_ODR
	HAL_DMA_Start(pxDmaChannel, (uint32_t) puiGpioPortAStates,
			(uint32_t) &GPIOA->ODR,
			sizeof(puiGpioPortAStates) / sizeof(*puiGpioPortAStates));

	// enable Timer/counter peripheral DMA triggering
	__HAL_TIM_ENABLE_DMA(pxHtim, TIM_DMA_UPDATE);

	// start Timer/counter peripheral
	HAL_TIM_Base_Start(pxHtim);
}
