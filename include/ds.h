#ifndef _H_DS_
#define _H_DS_


typedef unsigned int    uint;
typedef unsigned char   uchar;


typedef struct _SECTION {
    uint  offsetRaw;
    uint  sizeRaw;
    uchar *hash;
} SECTION;


typedef struct _SAMPLE {
    int     countSection;
    char    *pathSample;
    SECTION *arrSection;
} SAMPLE;


typedef struct _GROUPNODE {
    int  idGroup;
    int  idxSection;
    uint offsetSection;
    uint sizeSection;
    char *pathSample;   
} GROUPNODE;


#endif
