#pragma once

#include "timer.h"

#define freq_switcherMODE_COUNT 3
#define freq_switcherDEFAULT_FREQS (TimerConfig_t[freq_switcherMODE_COUNT]) \
	{timerTIM_CFG_FROM(64000 - 1, 2250 - 1),\
	timerTIM_CFG_FROM(64000 - 1, 1125 - 1),\
	timerTIM_CFG_FROM(32000 - 1, 1125 - 1)}

void FREQ_SWITCHER_SetMode(const unsigned int uiMode);
unsigned int FREQ_SWITCHER_GetMode();
void FREQ_SWITCHER_NextMode(void);
TimerConfig_t FREQ_SWITCHER_GetConfig(const unsigned int uiMode);
void FREQ_SWITCHER_SetConfig(const unsigned int uiMode, const TimerConfig_t xNewConfig);
