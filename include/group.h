#ifndef _GROUP_H_
#define _GROUP_H_


#include "cluster.h"


typedef struct _GROUP {
    CONFIG *cfgTask;

    int (*init_task)     (struct _GROUP*, CONFIG*);
    int (*deinit_task)   (struct _GROUP*);
    int (*generate_hash) (struct _GROUP*);
    int (*group_hash)    (struct _GROUP*);
} GROUP;


int grp_init_task(GROUP *self, CONFIG *cfgTask);


int grp_deinit_task(GROUP *self);


int grp_generate_hash(GROUP *self);


int grp_group_hash(GROUP *self);

#endif
