#include "spew.h"


void
SpewMessage(const char* szPathFile, const int32_t iLineFile, const char* szNameFunc,
            const char* szFormat, ...) {

    char bufMsg[MSG_BUF_SIZE];
    memset(bufMsg, 0, sizeof(char) * MSG_BUF_SIZE);
    va_list varArgument;
    va_start(varArgument, szFormat);
    int32_t iLenMsg = vsnprintf(bufMsg, sizeof(bufMsg), szFormat, varArgument);
    va_end(varArgument);

    if((iLenMsg == -1) || (iLenMsg >= (int)sizeof(bufMsg))) {
    iLenMsg = sizeof(bufMsg) - 1;
        bufMsg[iLenMsg] = 0;
    } else if(iLenMsg == 0) {
        iLenMsg = 0;
        bufMsg[0] = 0;
    }

    time_t tTime;
    time(&tTime);
    struct tm *tmTime = localtime(&tTime);
    char *szTime = asctime(tmTime);
    printf("[%s, %d, %s] %s%s\n", szPathFile, iLineFile, szNameFunc, szTime, bufMsg);

    return;
}

