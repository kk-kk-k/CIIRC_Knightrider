#include <stdio.h>
#include <assert.h>
#include "freq_switcher.h"
#include "timer.h"

/**
 * Select DMA channel here
 * Just setup pxDmaChannel to custom channel
 */
extern DMA_HandleTypeDef hdma_tim6_up;
static DMA_HandleTypeDef *const pxDmaChannel = &hdma_tim6_up;

/**
 * Select timer/counter peripheral here
 * Just setup pxHtim to custom channel
 */
extern TIM_HandleTypeDef htim6;
static TIM_HandleTypeDef *const pxHtim = &htim6;

/**
 * Timer configurations array
 */
static TimerConfig_t pxModeConfig[freq_switcherMODE_COUNT] =
freq_switcherDEFAULT_FREQS;
/**
 * Current mode
 */
static unsigned int uiCurrentMode = 0;

/**
 * @brief Set blinking mode
 *
 * @param uiMode New blinking mode
 */
void FREQ_SWITCHER_SetMode(const unsigned int uiMode) {
	// parameter check
	assert(uiMode < freq_switcherMODE_COUNT);

	// update mode
	uiCurrentMode = uiMode;

	// update peripherals
	TIMER_UpdateValue(pxHtim, pxDmaChannel, &pxModeConfig[uiCurrentMode]);
}

/**
 * @brief Obtain current blinking mode
 *
 * @return Current blinking mode
 */
unsigned int FREQ_SWITCHER_GetMode() {
	// return current mode
	return uiCurrentMode;
}

/**
 * @brief Jump to next blinking mode
 *
 */
void FREQ_SWITCHER_NextMode(void) {
	// update mode and check for overflow
	if (++uiCurrentMode == freq_switcherMODE_COUNT)
		uiCurrentMode = 0;

	// update peripherals
	FREQ_SWITCHER_SetMode(uiCurrentMode);
}

/**
 * @brief Obtain blinking configuration in given mode
 *
 * @param	uiMode Mode of configuration
 * @return	Blinking configuration in given mode
 */
TimerConfig_t FREQ_SWITCHER_GetConfig(const unsigned int uiMode) {
	// parameter check
	assert(uiMode < freq_switcherMODE_COUNT);

	// return copy of blinking mode
	return pxModeConfig[uiMode];
}

/**
 * @brief Set blinking configuration in given mode
 *
 * @param	uiMode Mode which is updated
 * @param	xNewConfig New mode configuration
 */
void FREQ_SWITCHER_SetConfig(const unsigned int uiMode, const TimerConfig_t xNewConfig) {
	// parameter check
	assert(uiMode < freq_switcherMODE_COUNT);

	// update mode configuration
	pxModeConfig[uiMode] = xNewConfig;

	// Update peripherials
	FREQ_SWITCHER_SetMode(uiCurrentMode);
}
