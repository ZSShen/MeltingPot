#ifndef _CLUSTER_H_
#define _CLUSTER_H_


#include <stdint.h>


typedef struct _CONFIG {
    uint8_t ucParallelity;
    uint8_t ucBlkCount;
    uint8_t ucBlkSize;
    uint8_t ucSimilarity;
    char *szPathInput;
    char *szPathOutput;
} CONFIG;


typedef struct _CLUSTER {
    int (*initCtx)           (struct _CLUSTER*, CONFIG*);
    int (*deinitCtx)         (struct _CLUSTER*);
    int (*generateGroup)     (struct _CLUSTER*);
    int (*generatePattern)   (struct _CLUSTER*);
} CLUSTER;


#define INIT_CLUSTER(p)     p = (CLUSTER*)malloc(sizeof(CLUSTER));     \
                            if (p != NULL) {                           \
                                iRtnCode = ClsInitTask(p);             \
                                if (iRtnCode == -1) {                  \
                                    free(p);                           \
                                }                                      \
                            }


#define DEINIT_CLUSTER(p)   if (p != NULL) {                           \
                                ClsDeinitTask(p);                      \
                                free(p);                               \
                            }


int ClsInitTask(CLUSTER *self);


int ClsDeinitTask(CLUSTER *self);


int ClsInitCtx(CLUSTER *self, CONFIG *pCfg);


int ClsDeinitCtx(CLUSTER *self);


int ClsGenerateGroup(CLUSTER *self);


int ClsGeneratePattern(CLUSTER *self);


#endif
