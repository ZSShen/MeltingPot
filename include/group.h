#ifndef _GROUP_H_
#define _GROUP_H_


#include "cluster.h"


typedef struct _GROUP {
    CONFIG *cfgTask;

    int (*generate_hash) (struct _GROUP*);
    int (*group_hash)    (struct _GROUP*);
} GROUP;


#define INIT_GROUP(p, q)    p = (GROUP*)malloc(sizeof(GROUP));   \
                            if (p != NULL) {                     \
                                rc = grp_init_task(p, q);        \
                                if (rc == -1) {                  \
                                    free(p);                     \
                                }                                \
                            }


#define DEINIT_GROUP(p)     if (p != NULL) {                     \
                                grp_deinit_task(p);              \
                                free(p);                         \
                            }


int grp_init_task(GROUP *self, CONFIG *cfgTask);


int grp_deinit_task(GROUP *self);


int grp_generate_hash(GROUP *self);


int grp_group_hash(GROUP *self);


#endif
