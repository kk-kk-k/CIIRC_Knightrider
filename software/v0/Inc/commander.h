#pragma once

/**
 * @brief Command function
 *
 * @param ppcTokens		Command tokens
 * @param uiTokenCount	Token count
 * @param pcReply		Command reply destination
 * @param uiReplySize	Reply destination max size
 */
typedef void (*COMMANDER_Func)(const TOKENATOR_Token ppcTokens[4],
		unsigned int uiTokenCount, char *const pcReply,
		const unsigned int uiReplySize);

#define commanderMAX_COMMAND_NAME_LEN 20

/**
 * @brief Command typedef
 *
 */
typedef struct {
	COMMANDER_Func xCallback; // Command callback function
	char pcName[commanderMAX_COMMAND_NAME_LEN];	// Command name
} COMMANDER_Callback;

/**
 * Command table with possible commands
 */
typedef COMMANDER_Callback COMMANDER_CommandTable[];

void COMMANDER_RunCommand(const char *const pcCommand, char *const pcReply,
		const unsigned int uiReplySize, const COMMANDER_CommandTable pxCommand);
