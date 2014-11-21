#include "spew.h"


/**
 * The log utility for precise error message.
 *
 * @param pathFile          The source file path of the invoking site.
 * @param numLine           The source file line number of the invoking site.
 * @param strFunc           The function name of the invoking site.
 * @param strFormat         The string formatter.
 * @param ...               The parameters of the string formatter.
 */
void spew_message(const char* pathFile, int numLine, const char* strFunc, const char* strFormat, ...) {
    char      bufMsg[MESSAGE_BUF_SIZE];
    int       lenMsg;
    time_t    nTime;        
    va_list   varArgument;
    char      *strTime;    
    struct tm *tmTime;

    memset(bufMsg, 0, sizeof(char) * MESSAGE_BUF_SIZE);
    va_start(varArgument, strFormat);
    lenMsg = vsnprintf(bufMsg, sizeof(bufMsg), strFormat, varArgument);
    va_end(varArgument);

    if((lenMsg == -1) || (lenMsg >= (int)sizeof(bufMsg))) {
	    lenMsg = sizeof(bufMsg) - 1;
	    bufMsg[lenMsg] = 0;
    } else if(lenMsg == 0) {
	    lenMsg = 0;
	    bufMsg[0] = 0;
    }

    time(&nTime);
    tmTime = localtime(&nTime);
    strTime = asctime(tmTime);

    printf("[%s, %d, %s] %s%s\n", pathFile, numLine, strFunc, strTime, bufMsg);

    return;
}
