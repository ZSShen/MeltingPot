#ifndef _H_DS_
#define _H_DS_

#include <stdint.h>

/* Define the buffer size. */
#define BUF_SIZE_SMALL      64
#define BUF_SIZE_MEDIUM     BUF_SIZE_SMALL  << 3
#define BUF_SIZE_LARGE      BUF_SIZE_MEDIUM << 3

/* The data structures to record section information. */
typedef struct _SECTION {
    uint32_t  rawOffset;
    uint32_t  rawSize;
    char      *hash;
} SECTION;

typedef struct _SAMPLE {
    uint16_t countSection;
    char     *nameSample;
    SECTION  *arraySection;
} SAMPLE;

#endif
