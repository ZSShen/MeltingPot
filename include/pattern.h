#ifndef _PATTERN_H_
#define _PATTERN_H_


#include "cluster.h"


typedef struct _PATTERN {
    CONFIG *pCfg;

    int (*init_task)        (struct _PATTERN*, CONFIG*);
    int (*deinit_task)      (struct _PATTERN*);
    int (*generate_pattern) (struct _PATTERN*);
} PATTERN;


int ptn_init_task(PATTERN *self, CONFIG *pCfg);


int ptn_deinit_task(PATTERN *self);


int ptn_generate_pattern(PATTERN *self);


#endif
