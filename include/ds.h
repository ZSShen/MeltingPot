#ifndef _H_DS_
#define _H_DS_

#include <stdint.h>

/* Define the buffer size. */
#define BUF_SIZE_SMALL      64
#define BUF_SIZE_MEDIUM     BUF_SIZE_SMALL  << 3
#define BUF_SIZE_LARGE      BUF_SIZE_MEDIUM << 3

/* The data structures to classify similar binary sections. */
typedef struct _BINARY {
    union {
        uint32_t idBinary;
        uint32_t idGroup;
    };
    uint16_t idxSection;
    uint32_t offsetSection;
    uint32_t sizeSection;
    char     *nameSample;
    char     *hash;
} BINARY;

typedef struct _GROUPNODE {
    BINARY   *hContent;
    uint32_t nRank;
    struct _GROUPNODE *pPresent;
} GROUPNODE;

typedef struct _RELATION {
    uint32_t idBinSource;
    uint32_t idBinTarget;
    struct _RELATION *next;
} RELATION;

typedef struct _PARAM {
    uint32_t countBinary;
    uint8_t  idThread;
    uint8_t  countThread;
    uint8_t  simThreshold;
    RELATION *listRelationHead;
    RELATION *listRelationTail;
} PARAM;

#endif
