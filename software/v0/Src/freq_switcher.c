#include <stdio.h>
#include <assert.h>
#include "freq_switcher.h"
#include "timer.h"

extern DMA_HandleTypeDef hdma_tim6_up;
static DMA_HandleTypeDef *const pxDmaChannel = &hdma_tim6_up;
extern TIM_HandleTypeDef htim6;
static TIM_HandleTypeDef *const pxHtim = &htim6;

static TimerConfig_t pxModeConfig[freq_switcherMODE_COUNT] =
freq_switcherDEFAULT_FREQS;
static unsigned int uiCurrentMode = 0;

void vFreqSwitcherSetMode(const unsigned int uiMode) {
	assert(uiMode < freq_switcherMODE_COUNT);

	uiCurrentMode = uiMode;
	vTimerUpdateValue(pxHtim, pxDmaChannel, &pxModeConfig[uiCurrentMode]);
}

void vFreqSwitcherNextMode(void) {
	if (++uiCurrentMode == freq_switcherMODE_COUNT)
		uiCurrentMode = 0;

	vFreqSwitcherSetMode(uiCurrentMode);
}
