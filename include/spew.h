#ifndef _SPEW_H_
#define _SPEW_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>


#define MESSAGE_BUF_SIZE   1024


/* Wrapper for debug utilities. */
#define Spew                         spew_message(__FILE__, __LINE__, __FUNCTION__,
#define Spew0(p0)                    Spew p0)
#define Spew1(p0, p1)                Spew p0, p1)
#define Spew2(p0, p1, p2)            Spew p0, p1, p2)
#define Spew3(p0, p1, p2, p3)        Spew p0, p1, p2, p3)


void spew_message(const char*, const int, const char*, const char*, ...);


#endif
