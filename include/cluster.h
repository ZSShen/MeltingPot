#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include <stdint.h>

typedef struct _CONFIG {
    uint8_t nParallelity;
    uint8_t nBlkCount;
    uint8_t nBlkSize;
    uint8_t nSimilarity;
    char    *szPathRoot;
    char    *pathPattern;
} CONFIG;


typedef struct _CLUSTER {
    int (*init_ctx)           (struct _CLUSTER*, CONFIG*);
    int (*deinit_ctx)         (struct _CLUSTER*);
    int (*generate_group)     (struct _CLUSTER*);
    int (*generate_pattern)   (struct _CLUSTER*);
} CLUSTER;


#define INIT_CLUSTER(p)     p = (CLUSTER*)malloc(sizeof(CLUSTER));   \
                            if (p != NULL) {                         \
                                iRtnCode = cls_init_task(p);               \
                                if (iRtnCode == -1) {                      \
                                    free(p);                         \
                                }                                    \
                            }


#define DEINIT_CLUSTER(p)   if (p != NULL) {                         \
                                cls_deinit_task(p);                  \
                                free(p);                             \
                            }


int cls_init_task(CLUSTER *self);


int cls_deinit_task(CLUSTER *self);


int cls_init_ctx(CLUSTER *self, CONFIG *pCfg);


int cls_deinit_ctx(CLUSTER *self);


int cls_generate_group(CLUSTER *self);


int cls_generate_pattern(CLUSTER *self);


#endif
