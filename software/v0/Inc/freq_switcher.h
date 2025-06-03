#pragma once

#define freq_switcherMODE_COUNT 3
#define freq_switcherDEFAULT_FREQS (TimerConfig_t[freq_switcherMODE_COUNT]) \
	{timerTIM_CFG_FROM(64000, 2250),\
	timerTIM_CFG_FROM(64000, 1125),\
	timerTIM_CFG_FROM(32000, 1125)}

void vFreqSwitcherSetMode(const unsigned int uiMode);
void vFreqSwitcherNextMode(void);
