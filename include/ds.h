#ifndef _H_DS_
#define _H_DS_


typedef unsigned int    uint;
typedef unsigned char   uchar;


typedef struct _SECTION {
    uint  offsetRaw;
    uint  sizeRaw;
    char  *hash;
} SECTION;


typedef struct _SAMPLE {
    int     countSection;
    char    *nameSample;
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
