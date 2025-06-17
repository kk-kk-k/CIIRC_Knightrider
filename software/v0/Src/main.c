/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "tokenator.h"
#include "freq_switcher.h"
#include "commander.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define mainUART_RECEIVE_BUFFER_SIZE 50 // UART receive buffer size

// comment/uncomment the following line to toggle standby low-power mode usage
// #define USE_BATERRY
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim6;
DMA_HandleTypeDef hdma_tim6_up;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static char pcUartReceivedBuffer[mainUART_RECEIVE_BUFFER_SIZE]; // UART 2 receive buffer
static unsigned int uiUartReceivedBufferIndex = 0;
static int iUartReceivedBufferComplete = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief HAL GPIO external interrupt callback
 *
 * @param	GPIO_Pin Pin which interrupted MCU
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	// check if interrupt comes from button pin
	if (GPIO_Pin == BUTTON_Pin) {
		// update mode
		FREQ_SWITCHER_NextMode();
#ifdef USE_BATERRY
		// if new mode is 0
		if (FREQ_SWITCHER_GetMode() == 0) {
			// enter standby mode
			__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

			HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
			HAL_PWR_EnterSTANDBYMode();
		}
#endif
	}
}

/**
 * @brief HAL UART interrupt callback
 *
 * @param pxHuart	UART instance which interrupted the MCU
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *pxHuart) {
	// check if interrupt comes from UART2
	if (pxHuart->Instance == USART2) {
		// make character lower
		pcUartReceivedBuffer[uiUartReceivedBufferIndex] = (char) tolower(
				pcUartReceivedBuffer[uiUartReceivedBufferIndex]);

		// if newline received
		if (pcUartReceivedBuffer[uiUartReceivedBufferIndex] == '\n') {
			// store end of string to received buffer
			pcUartReceivedBuffer[uiUartReceivedBufferIndex] = '\0';

			// mark buffer as complete
			iUartReceivedBufferComplete = 1;

			// reset buffer iterator
			uiUartReceivedBufferIndex = 0;
		}
		// ignore carriage return
		else if (pcUartReceivedBuffer[uiUartReceivedBufferIndex] != '\r')
			uiUartReceivedBufferIndex++;

		// receive next character
		HAL_UART_Receive_IT(&huart2,
				(uint8_t*) &pcUartReceivedBuffer[uiUartReceivedBufferIndex], 1);
	}
}

/*
 * 	Start help command
 */

/**
 * @brief Help command function
 *
 * @param				Unused
 * @param 				Unused
 * @param pcReply		Help command reply destination
 * @param uiReplySize	Help command reply destination length
 */
static void COMMANDER_CommandHelp(const TOKENATOR_Token[4], unsigned int,
		char *const pcReply, const unsigned int uiReplySize) {
	// copy help
	strncpy(pcReply, "Help: \r\n"
			"commands: \r\n"
			"help:\t\t show this help\r\n"
			"read <variable>:\t read variable value\r\n"
			"set <variable> <value>:\t set variable to value\r\n"
			"restart:\t\t restart loop\r\n"
			"\r\n"
			"Command line is NOT case sensitive \r\n", uiReplySize);
}
/*
 * End help command
 */

/*
 * Start read command
 */

/**
 * @brief Show read command usage
 *
 * @param pcMessage		Additional message which will be copied to pcReply (NULL for none)
 * @param pcReply		Read command usage reply destination
 * @param uiReplySize	Read command usage reply destination length
 */
static void COMMANDER_ReadShowUsage(const char *const pcMessage,
		char *const pcReply, const unsigned int uiReplySize) {
	char pcBuf[500];

	// if no message provided
	if (pcMessage == NULL)
		// setup reply to have zero length
		pcReply[0] = '\0';
	else
		// else copy message to reply
		strncpy(pcReply, pcMessage, uiReplySize);

	// print usage to pcBuf
	snprintf(pcBuf, sizeof(pcBuf), "Usage: 'read <variable-name>' \r\n"
			"Reads variable value \r\n"
			"Available keywords: \r\n"
			"\tall:\t\tAll variables\r\n"

			"\tmode:\t\tCurrent blinking mode\r\n"

			"\tfreq:\t\tCurrent frequency\r\n"
			"\tprescaller:\t\tCurrent mode prescaller"
			"\tperiod:\t\tCurrent mode period"

			"\tmode<mode-id>.freq:\t\tFrequency in mode\r\n"
			"\tmode<mode-id>.prescaller:\t\tPrescaller value in given mode\r\n"
			"\tmode<mode-id>.period:\t\tPeriod value in given mode\r\n"

			"Range of variables:\r\n"
			"mode:\t\t0..%d\r\n",
	freq_switcherMODE_COUNT);

	// append pcBuf to pcReply
	strncat(pcReply, pcBuf, uiReplySize);
}

/**
 * @brief Read command function
 *
 * @param ppcTokens		Read command arguments
 * @param uiTokenCount	Read command argument count
 * @param pcReply		Read command reply destination
 * @param uiReplySize	Read command reply destination size
 */
static void COMMANDER_CommandRead(const TOKENATOR_Token ppcTokens[4],
		const unsigned int uiTokenCount, char *const pcReply,
		const unsigned int uiReplySize) {

	// check if one argument passed
	if (uiTokenCount == 2) {
		// send mode
		if (strcmp(ppcTokens[1], "mode") == 0) {
			snprintf(pcReply, uiReplySize, "Mode: %d\r\n",
					FREQ_SWITCHER_GetMode());
		}
		// send all variables
		else if (strcmp(ppcTokens[1], "all") == 0) {
			// print mode
			snprintf(pcReply, uiReplySize, "Mode: %d\r\n"
					"\r\n", FREQ_SWITCHER_GetMode());

			// iterate over mode configurations
			for (unsigned int uiMode = 0; uiMode < freq_switcherMODE_COUNT;
					uiMode++) {
				char pcBuf[80];

				// read timer configuration
				const TimerConfig_t xTimerCfg = FREQ_SWITCHER_GetConfig(uiMode);

				// print configuration details
				snprintf(pcBuf, sizeof(pcBuf), "Mode %d\r\n"
						"Frequency: %lg\r\n"
						"Prescaller: %u\r\n"
						"Period: %u\r\n"
						"\r\n", uiMode,
						(double) SystemCoreClock
								/ (double) (xTimerCfg.uiPrescaller + 1)
								/ (double) (xTimerCfg.uiPeriod + 1),
						xTimerCfg.uiPrescaller, xTimerCfg.uiPeriod);

				// append configuration details to reply
				strncat(pcReply, pcBuf, uiReplySize);
			}
		}
		// send prescaller
		else if (strcmp(ppcTokens[1], "prescaller") == 0) {
			// receive current configuration
			const TimerConfig_t xTimerCfg = FREQ_SWITCHER_GetConfig(
					FREQ_SWITCHER_GetMode());

			// send prescaller
			snprintf(pcReply, uiReplySize, "Prescaller: %u\r\n",
					xTimerCfg.uiPrescaller);
		}
		// send period
		else if (strcmp(ppcTokens[1], "period") == 0) {
			// receive current configuration
			const TimerConfig_t xTimerCfg = FREQ_SWITCHER_GetConfig(
					FREQ_SWITCHER_GetMode());

			// send period
			snprintf(pcReply, uiReplySize, "Period: %u\r\n",
					xTimerCfg.uiPeriod);
		}
		// send frequency
		else if (strcmp(ppcTokens[1], "freq") == 0) {
			// receive current configuration
			const TimerConfig_t xTimerCfg = FREQ_SWITCHER_GetConfig(
					FREQ_SWITCHER_GetMode());

			// send frequency
			snprintf(pcReply, uiReplySize, "Frequency: %lg\r\n",
					(float) SystemCoreClock
							/ (double) (xTimerCfg.uiPrescaller + 1)
							/ (double) (xTimerCfg.uiPeriod + 1));
		}
		// send settings in mode
		else {
			TOKENATOR_Token ppcModeTokens[3] = { };

			// split additional arguments to tokens
			const unsigned int uiModeTokenCount = TOKENATOR_IntoTokens(
					ppcTokens[1], ppcModeTokens, 3, '_');

			// if user typed mode_<id>.<something>
			if (uiModeTokenCount == 2
					&& strcmp(ppcModeTokens[0], "mode") == 0) {
				char *pcEnd;

				// split mode_<id> to 'mode' '<id>'
				const unsigned int uiMode = (unsigned int) strtol(
						ppcModeTokens[1], &pcEnd, 10);

				// if user typed only mode
				if (pcEnd == ppcModeTokens[1]) {
					// send error message and usage
					COMMANDER_ReadShowUsage("Variable not found\r\n", pcReply,
							uiReplySize);
				}
				// if mode is not out out of bonds
				else if (uiMode < freq_switcherMODE_COUNT) {
					// receive current configuration
					const TimerConfig_t xTimerCfg = FREQ_SWITCHER_GetConfig(
							uiMode);

					TOKENATOR_Token ppcModeTokens[2] = { };

					// split mode_<id>.<something> to 'mode_<id>' and '<something>'
					const unsigned int uiModeTokenCount = TOKENATOR_IntoTokens(
							ppcTokens[1], ppcModeTokens, 2, '.');

					// if user typed 'all' or just 'mode_<id>'
					if (uiModeTokenCount == 1
							|| strcmp(ppcModeTokens[1], "all") == 0) {
						// print all variables
						snprintf(pcReply, uiReplySize, "Frequency: %lg\r\n"
								"Prescaller: %u\r\n"
								"Period: %u\r\n",
								(double) SystemCoreClock
										/ (double) (xTimerCfg.uiPrescaller + 1)
										/ (double) (xTimerCfg.uiPeriod + 1),
								xTimerCfg.uiPrescaller, xTimerCfg.uiPeriod);
					}
					// if user typed 'mode_<id>.freq'
					else if (strcmp(ppcModeTokens[1], "freq") == 0) {
						// print frequency
						snprintf(pcReply, uiReplySize, "Frequency: %lg\r\n",
								(float) SystemCoreClock
										/ (double) (xTimerCfg.uiPrescaller + 1)
										/ (double) (xTimerCfg.uiPeriod + 1));
					}
					// if user typed 'mode_<id>.period'
					else if (strcmp(ppcModeTokens[1], "period") == 0) {
						// print period
						snprintf(pcReply, uiReplySize, "Period: %u\r\n",
								xTimerCfg.uiPeriod);
					}
					// if user typed 'mode_<id>.prescaller'
					else if (strcmp(ppcModeTokens[1], "prescaller") == 0) {
						// send prescaller
						snprintf(pcReply, uiReplySize, "Prescaller: %u\r\n",
								xTimerCfg.uiPrescaller);
					}
					// if user typed anything else
					else {
						// print error message and usage
						COMMANDER_ReadShowUsage("Variable not found\r\n",
								pcReply, uiReplySize);
					}
				}
			}
			// send error message and usage
			else {
				COMMANDER_ReadShowUsage("Variable not found\r\n", pcReply,
						uiReplySize);
			}
		}
	}
	// send usage if no arguments provided
	else {
		COMMANDER_ReadShowUsage(NULL, pcReply, uiReplySize);
	}
}
/*
 * End read command
 */

/*
 * Start restart command
 */
/**
 * @brief Restart command
 *
 * @param 				Unused
 * @param uiTokenCount	Restart command argument count (used for detection: no arguments received/some arguments received => print help)
 * @param pcReply		Restart command usage reply destination
 * @param uiReplySize	Restart command usage reply destination size
 */
static void COMMANDER_CommandRestart(const TOKENATOR_Token[4],
		unsigned int uiTokenCount, char *const pcReply,
		const unsigned int uiReplySize) {
	// check if no parameters provided
	if (uiTokenCount == 1) {
		// restart timer/counter
		FREQ_SWITCHER_SetMode(FREQ_SWITCHER_GetMode());
		strncpy(pcReply, "Restarted tim6\r\n", uiReplySize);
	}
	// show usage
	else {
		strncpy(pcReply,
				"Usage: 'restart'\r\n"
						"Restarts tim6 and starts blinking with LEDs from beginning.\r\n",
				uiReplySize);
	}
}
/*
 * End restart command
 */

/*
 * Start set command
 */

/**
 * @brief Show set command usage
 *
 * @param pcMessage		Additional message for user
 * @param pcReply		Command set reply destination
 * @param uiReplySize	Command set reply destination size
 */
static void COMMANDER_SetShowUsage(const char *const pcMessage,
		char *const pcReply, const unsigned int uiReplySize) {
	char pcBuf[550];

	// if no message provided
	if (pcMessage == NULL)
		// mark string as empty
		pcReply[0] = '\0';
	else
		// copy message
		strncpy(pcReply, pcMessage, uiReplySize);

	// print usage
	snprintf(pcBuf, sizeof(pcBuf),
			"Usage: 'set <variable-name> <new-value>' \r\n"
					"Sets variable to new value \r\n"
					"Available keywords: \r\n"

					"\tmode <new-mode>:\t\tUpdate current blinking mode to new mode\r\n"

					"\tprescaller <new-value>:\t\tUpdate current mode prescaller to new value\r\n"
					"\tperiod <new-value>:\t\tUpdate current mode period to new value\r\n"

					"\tmode<mode-id>.prescaller <new-value>:\t\tUpdate prescaller value in given mode to new value\r\n"
					"\tmode<mode-id>.period <new-value>:\t\tUpdate period value in given mode to new value\r\n"

					"Range of variables:\r\n"
					"mode:\t\t0..%d\r\n"
					"prescaller:\t\t0..=%d\r\n"
					"period:\t\t0..=%d\r\n",
			freq_switcherMODE_COUNT,
			timerMAX_PRESCALLER,
			timerMAX_PERIOD);

	// append usage to reply
	strncat(pcReply, pcBuf, uiReplySize);
}

/**
 * @brief Set command
 *
 * @param ppcTokens		Set command arguments
 * @param uiTokenCount	Set command argument count
 * @param pcReply		Set command reply destination
 * @param uiReplySize	Set command reply destination size
 */
static void COMMANDER_CommandSet(const TOKENATOR_Token ppcTokens[4],
		const unsigned int uiTokenCount, char *const pcReply,
		const unsigned int uiReplySize) {

	// check if user typed '<variable-name> <new-value>'
	if (uiTokenCount == 3) {
		// if user typed 'mode <new-mode>'
		if (strcmp(ppcTokens[1], "mode") == 0) {
			char *pcEnd = NULL;

			// get user input
			const long lInput = strtol(ppcTokens[2], &pcEnd, 10);

			// if user typed value in allowed range
			if (lInput
					>= 0&&
					pcEnd[0] == '\0' && (unsigned int) lInput <= freq_switcherMODE_COUNT) {
				// update mode
				FREQ_SWITCHER_SetMode((unsigned int) lInput);

				// send reply
				strncpy(pcReply, "Mode updated successfully\r\n", uiReplySize);
			} else {
				// send error message
				snprintf(pcReply, uiReplySize,
						"Mode out of bonds (0 <= mode < %d)\r\n",
						freq_switcherMODE_COUNT);
			}
		}
		// if user typed 'prescaller <new-value>'
		else if (strcmp(ppcTokens[1], "prescaller") == 0) {
			char *pcEnd = NULL;

			// get user input
			const long lInput = strtol(ppcTokens[2], &pcEnd, 10);

			// if input is in allowed range
			if (lInput >= 0&& pcEnd[0] == '\0'
			&& (unsigned int) lInput <= timerMAX_PRESCALLER) {
				// obtain current mode configuration
				TimerConfig_t xTimerCfg = FREQ_SWITCHER_GetConfig(
						FREQ_SWITCHER_GetMode());

				// update current mode configuration
				xTimerCfg.uiPrescaller = (unsigned int) lInput;
				FREQ_SWITCHER_SetConfig(FREQ_SWITCHER_GetMode(), xTimerCfg);

				// send reply
				strncpy(pcReply, "Prescaller updated successfully\r\n",
						uiReplySize);
			} else {
				// send error message
				snprintf(pcReply, uiReplySize,
						"Prescaller out of bonds (0 <= mode < %d)\r\n",
						timerMAX_PRESCALLER);
			}
		}
		// if user typed 'period <new-value>'
		else if (strcmp(ppcTokens[1], "period") == 0) {
			char *pcEnd = NULL;

			// read user input
			const long lInput = strtol(ppcTokens[2], &pcEnd, 10);

			// if input is in allowed range
			if (lInput >= 0&& pcEnd[0] == '\0'
			&& (unsigned int) lInput < timerMAX_PRESCALLER) {
				// get current configuration
				TimerConfig_t xTimerCfg = FREQ_SWITCHER_GetConfig(
						FREQ_SWITCHER_GetMode());

				// update current configuration
				xTimerCfg.uiPeriod = (unsigned int) lInput;
				FREQ_SWITCHER_SetConfig(FREQ_SWITCHER_GetMode(), xTimerCfg);

				// send reply
				strncpy(pcReply, "Period updated successfully\r\n",
						uiReplySize);
			} else {
				// send error message
				snprintf(pcReply, uiReplySize,
						"Period out of bonds (0 <= mode < %d)\r\n",
						timerMAX_PERIOD);
			}
		} else {
			TOKENATOR_Token ppcModeTokens[2] = { };

			// split variable name to tokens by '_'
			const unsigned int uiModeTokenCount = TOKENATOR_IntoTokens(
					ppcTokens[1], ppcModeTokens, 2, '_');

			// if variable name is 'mode_<something>'
			if (uiModeTokenCount == 2
					&& strcmp(ppcModeTokens[0], "mode") == 0) {
				char *pcEnd;

				// get mode id from variable name
				const unsigned int uiMode = (unsigned int) strtol(
						ppcModeTokens[1], &pcEnd, 10);

				// if mode is not out of bonds
				if (uiMode < freq_switcherMODE_COUNT
						&& pcEnd != ppcModeTokens[1]) {
					TOKENATOR_Token ppcModeTokens[2] = { };

					// extract '<something>' from 'mode_<mode-id>.<something>'
					const unsigned int uiModeTokenCount = TOKENATOR_IntoTokens(
							ppcTokens[1], ppcModeTokens, 2, '.');

					// if succeeded extraction '<something>' from 'mode_<mode-id>.<something>'
					if (uiModeTokenCount == 2) {
						// if user typed 'mode<mode-id>.period <new-period>'
						if (strcmp(ppcModeTokens[1], "period") == 0) {
							char *pcEnd = NULL;

							// obtain new period value
							const long lInput = strtol(ppcTokens[2], &pcEnd,
									10);

							// check if period value is not out of bonds
							if (lInput >= 0&& pcEnd[0] == '\0'
							&& (unsigned int) lInput <= timerMAX_PERIOD) {
								// get mode configuration
								TimerConfig_t xTimerCfg =
										FREQ_SWITCHER_GetConfig(uiMode);

								// update mode configuration
								xTimerCfg.uiPeriod = (unsigned int) lInput;
								FREQ_SWITCHER_SetConfig(uiMode, xTimerCfg);

								// send reply
								strncpy(pcReply,
										"Period updated successfully\r\n",
										uiReplySize);
							} else {
								// send error message
								snprintf(pcReply, uiReplySize,
										"Period out of bonds (0 <= mode < %d)\r\n",
										timerMAX_PERIOD);
							}
						}
						// if user typed 'mode<mode-id>.prescaller <new-period>'
						else if (strcmp(ppcModeTokens[1], "prescaller") == 0) {
							char *pcEnd = NULL;

							// extract new prescaller value
							const long lInput = strtol(ppcTokens[2], &pcEnd,
									10);

							// check if new prescaller value is not out of bonds
							if (lInput
									>= 0&& pcEnd[0] == '\0'
									&& (unsigned int) lInput <= timerMAX_PRESCALLER) {
								// get mode configuration
								TimerConfig_t xTimerCfg =
										FREQ_SWITCHER_GetConfig(uiMode);

								// update mode configuration
								xTimerCfg.uiPrescaller = (unsigned int) lInput;
								FREQ_SWITCHER_SetConfig(uiMode, xTimerCfg);

								// send reply
								strncpy(pcReply,
										"Period updated successfully\r\n",
										uiReplySize);
							} else {
								snprintf(pcReply, uiReplySize,
										"Period out of bonds (0 <= mode < %d)\r\n",
										timerMAX_PERIOD);
							}
						}
						// no matching variable found
						else {
							// send error message
							COMMANDER_SetShowUsage("Variable not found\r\n",
									pcReply, uiReplySize);
						}
					}
					// if extraction '<something>' from 'mode_<mode-id>.<something>' failed
					else {
						COMMANDER_SetShowUsage("Variable not found\r\n",
								pcReply, uiReplySize);
					}
				}
				// if extraction '<something>' from 'mode_<mode-id>.<something>' failed
				else {
					char pcBuf[50];

					snprintf(pcBuf, uiReplySize,
							"Error: Mode index out of bonds (allowed range: 0 <= freq index < %d)\r\n",
							freq_switcherMODE_COUNT);
					COMMANDER_SetShowUsage(pcBuf, pcReply, uiReplySize);
				}
			}
			// check if user did not type '<variable-name> <new-value>'
			else {
				// send error message
				COMMANDER_SetShowUsage("Variable not found\r\n", pcReply,
						uiReplySize);
			}
		}
	} else {
		COMMANDER_SetShowUsage(NULL, pcReply, uiReplySize);
	}
}
/*
 * End set command
 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	// set blinking mode to 0
	FREQ_SWITCHER_SetMode(0);

	// receive characters from uart2
	HAL_UART_Receive_IT(&huart2, (uint8_t*) &pcUartReceivedBuffer[0], 1);

	// Disable SysTick interrupt
	// When enabled, this causes signal flickering
	HAL_SuspendTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		// check if uart2 buffer is complete
		if (iUartReceivedBufferComplete) {
			char pcCommand[mainUART_RECEIVE_BUFFER_SIZE];
			char pcReply[600];

			// copy received buffer (what if another character arrived...)
			strncpy(pcCommand, pcUartReceivedBuffer,
			mainUART_RECEIVE_BUFFER_SIZE);

			// reset status of uart2 buffer
			iUartReceivedBufferComplete = 0;

			static const COMMANDER_CommandTable pxCommands = { {
					COMMANDER_CommandHelp, "help" }, { COMMANDER_CommandSet,
					"set" }, { COMMANDER_CommandRead, "read", }, {
					COMMANDER_CommandRestart, "restart" }, {
					(COMMANDER_Func) NULL, "" } };

			// run command
			COMMANDER_RunCommand(pcCommand, pcReply, sizeof(pcReply),
					pxCommands);

			// send command reply
			HAL_UART_Transmit_IT(&huart2, (uint8_t*) pcReply,
					(unsigned short int) strlen(pcReply));

			HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON,
			PWR_SLEEPENTRY_WFI);
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 5;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin
                          |LED4_Pin|LED5_Pin|LED6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED0_Pin LED1_Pin LED2_Pin LED3_Pin
                           LED4_Pin LED5_Pin LED6_Pin */
  GPIO_InitStruct.Pin = LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin
                          |LED4_Pin|LED5_Pin|LED6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
