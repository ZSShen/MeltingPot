/**
 *   The MIT License (MIT)
 *   Copyright (C) 2014-2017 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */


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

