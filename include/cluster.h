#ifndef _CLUSTER_H_
#define _CLUSTER_H_


typedef struct _CONFIG {
    int  cfgParallelity;
    int  cfgBlkCount;
    int  cfgBlkSize;
    int  cfgSimilarity;
    char *cfgPathRoot;
    char *cfgPathPat;
} CONFIG;


typedef struct _CLUSTER {
    int (*init_ctx)           (struct _CLUSTER*, CONFIG*);
    int (*deinit_ctx)         (struct _CLUSTER*);
    int (*generate_group)     (struct _CLUSTER*);
    int (*generate_pattern)   (struct _CLUSTER*);
} CLUSTER;


#define INIT_CLUSTER(p)     p = (CLUSTER*)malloc(sizeof(CLUSTER));   \
                            if (p != NULL) {                         \
                                rc = cls_init_task(p);               \
                                if (rc == -1) {                      \
                                    free(p);                         \
                                }                                    \
                            }


#define DEINIT_CLUSTER(p)   if (p != NULL) {                         \
                                cls_deinit_task(p);                  \
                                free(p);                             \
                            }


int cls_init_task(CLUSTER *self);


int cls_deinit_task(CLUSTER *self);


int cls_init_ctx(CLUSTER *self, CONFIG *cfgTask);


int cls_deinit_ctx(CLUSTER *self);


int cls_generate_group(CLUSTER *self);


int cls_generate_pattern(CLUSTER *self);


#endif
