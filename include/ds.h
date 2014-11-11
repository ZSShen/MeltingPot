#ifndef _H_DS_
#define _H_DS_


#define BUF_SIZE_SMALL      64
#define BUF_SIZE_MEDIUM     BUF_SIZE_SMALL  << 3
#define BUF_SIZE_LARGE      BUF_SIZE_MEDIUM << 3


typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   uchar;


typedef struct _SECTION {
    uint  rawOffset;
    uint  rawSize;
    char  *hash;
} SECTION;


typedef struct _SAMPLE {
    int     countSection;
    char    *nameSample;
    SECTION *arraySection;
} SAMPLE;


typedef struct _GROUPNODE {
    int  idGroup;
    int  idxSection;
    uint offsetSection;
    uint sizeSection;
    char *pathSample;   
} GROUPNODE;


#endif
