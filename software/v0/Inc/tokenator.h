#pragma once

#define tokenatorMAX_TOKEN_LEN 100

typedef char TOKENATOR_Token[tokenatorMAX_TOKEN_LEN];

unsigned int TOKENATOR_IntoTokens(const char * pcSource, TOKENATOR_Token ppcParts[], const unsigned int uiMaxTokenCount, const char cSplitter);
