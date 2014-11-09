#ifndef _H_DS_
#define _H_DS_


typedef unsigned int    uint;
typedef unsigned char   uchar;


typedef struct _SECTION {
    uint  offsetRaw;
    uint  sizeRaw;
    uchar *hash;
} SECTION;




#endif
