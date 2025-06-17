#include <assert.h>
#include <ctype.h>

#include "tokenator.h"

/**
 * @brief Split string to tokens by given character
 *
 * @param pcSource			Parsed string
 * @param pcTokensLoaded	Destination of parsed data
 * @param uiMaxTokenCount	Maximal token count. When reached, no more tokens are parsed and function returns
 * @param cSplitter			Splitting ASCII character
 * @return					Number of tokens parsed
 */
unsigned int TOKENATOR_IntoTokens(const char *pcSource,
		TOKENATOR_Token pcTokensLoaded[], const unsigned int uiMaxTokenCount,
		const char cSplitter) {
	// parameter check
	assert(pcSource != NULL);
	assert(uiMaxTokenCount >= 1);
	assert(isprint(cSplitter));

	/**
	 * Remember current part length
	 */
	unsigned int uiCurrentPartLen = 0;
	/**
	 * Remember pointer to current token
	 */
	char (*ppcCurrentToken)[tokenatorMAX_TOKEN_LEN] = &pcTokensLoaded[0];
	/**
	 * Remember current character
	 */
	char *pcCurrentTokenCharacter = ppcCurrentToken[0];

	// prepare string - remove all splitters before the actual parsed string
	while (1) {
		// ignore splitting character
		if (pcSource[0] == cSplitter)
			pcSource++;
		// if end of string reached, return zero token count loaded
		else if (pcSource[0] == '\0') {
			*pcCurrentTokenCharacter = '\0';
			return 0;
		}
		// if reached any other character, end loop
		else
			break;
	}

	// iterate over parsed string
	while (1) {
		// check of end of string
		if (pcSource[0] == '\0') {
			// store end of string
			*pcCurrentTokenCharacter = '\0';
			// return loaded token count (ppcCurrentToken iterates over pcTokensLoaded, so we can substitute them to obtain current index)
			return (unsigned int) (ppcCurrentToken - pcTokensLoaded) + 1;
		}

		// check if splitting character reached
		if (pcSource[0] == cSplitter) {
			// iterate over pcSource
			while (1) {
				// skip splitting character
				pcSource++;

				// if reached splitting character, continue in loop
				if (pcSource[0] == cSplitter)
					continue;
				// if reached end of string, store end of string and return
				else if (pcSource[0] == '\0') {
					*pcCurrentTokenCharacter = '\0';
					// return loaded token count (ppcCurrentToken iterates over pcTokensLoaded, so we can substitute them to obtain current index)
					return (unsigned int) (ppcCurrentToken - pcTokensLoaded) + 1;
				}
				// if reached non-splitting character, break the loop
				else
					break;
			}

			// store end of string to current string
			*pcCurrentTokenCharacter = '\0';

			// update current part and check max token count
			ppcCurrentToken += 1;
			if ((unsigned int) (ppcCurrentToken - pcTokensLoaded) == uiMaxTokenCount)
				return uiMaxTokenCount;

			// update current string pointer
			pcCurrentTokenCharacter = ppcCurrentToken[0];

			// reset current part length
			uiCurrentPartLen = 0;

			// start loop from beginning
			continue;
		}

		// update current part length
		uiCurrentPartLen++;

		// copy source to current token
		*pcCurrentTokenCharacter = pcSource[0];

		// update current string pointer (move to next character)
		pcCurrentTokenCharacter++;

		// update source string index
		pcSource++;
	}

	return 0;
}
