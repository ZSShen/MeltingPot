#include "spew.h"


void
SpewMessage(const char* szPathFile, const int32_t iLineFile, const char* szNameFunc,
            const char* szFormat, ...) {
    int32_t iLenMsg;
    time_t tTime;
    char *szTime; 
    struct tm *tmTime;
    va_list varArgument;
    char bufMsg[MSG_BUF_SIZE];

    memset(bufMsg, 0, sizeof(char) * MSG_BUF_SIZE);
    va_start(varArgument, szFormat);
    iLenMsg = vsnprintf(bufMsg, sizeof(bufMsg), szFormat, varArgument);
    va_end(varArgument);

    if((iLenMsg == -1) || (iLenMsg >= (int)sizeof(bufMsg))) {
	    iLenMsg = sizeof(bufMsg) - 1;
	    bufMsg[iLenMsg] = 0;
    } else if(iLenMsg == 0) {
	    iLenMsg = 0;
	    bufMsg[0] = 0;
    }

    time(&tTime);
    tmTime = localtime(&tTime);
    szTime = asctime(tmTime);

    printf("[%s, %d, %s] %s%s\n", szPathFile, iLineFile, szNameFunc, szTime, bufMsg);

    return;
}
