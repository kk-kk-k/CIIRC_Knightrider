#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "tokenator.h"
#include "commander.h"

/**
 * @brief Run command stored in command table
 *
 * @param pcCommand 	Command to run
 * @param pcReply		Command reply destination
 * @param uiReplySize	Command reply size
 * @param pxCommands	Command table
 */
void COMMANDER_RunCommand(const char *const pcCommand, char *const pcReply,
		const unsigned int uiReplySize, const COMMANDER_CommandTable pxCommands) {
	TOKENATOR_Token ppcTokens[4] = { };

	// parameter check
	assert(pcCommand != NULL);
	assert(pcReply != NULL);
	assert(uiReplySize >= 1);
	assert(pxCommands != NULL);

	// split command into tokens
	const unsigned int uiTokenCount = TOKENATOR_IntoTokens(pcCommand, ppcTokens,
			4, ' ');

	// check if empty line was passed
	if (uiTokenCount == 0) {
		strcpy(pcReply, "No command available\r\n");
		return;
	}

	// go through command table until reached NULL callback
	for (unsigned int uiCommandIndex = 0; ; uiCommandIndex++) {
		if (pxCommands[uiCommandIndex].xCallback == NULL) {
			snprintf(pcReply, uiReplySize, "Error: Command '%s' not found \r\n"
					"Try typing 'help' to see available commands\r\n",
					ppcTokens[0]);
			return;
		}

		// compare command and command table key
		else if (strcmp(pxCommands[uiCommandIndex].pcName, ppcTokens[0]) == 0) {
			// call command callback
			pxCommands[uiCommandIndex].xCallback(ppcTokens, uiTokenCount,
					pcReply, uiReplySize);
			return;
		}
	}

}
