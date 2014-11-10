#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include "ds.h"
#include "spew.h"
#include "group.h"


/*-----------------------------------------------------------*
 *           Declaration for Internal Functions              *
 *-----------------------------------------------------------*/



/*-----------------------------------------------------------*
 *          Implementation for External Functions            *
 *-----------------------------------------------------------*/
int grp_init_task(GROUP *self, CONFIG *cfgTask) {

    self->cfgTask = cfgTask;
    self->generate_hash = grp_generate_hash;
    self->group_hash = grp_group_hash;
    return 0;
}


int grp_deinit_task(GROUP *self) {


    return 0;
}


int grp_generate_hash(GROUP *self) {
    int rc;
    DIR *dirRoot;
    struct dirent *entFile;

    rc = 0;

    /* Open the root path of designated sample set. */
    dirRoot = opendir(self->cfgTask->cfgPathRoot);
    if (dirRoot == NULL) {
        rc = -1;
        Spew1("Error: %s", strerror(errno));
        goto EXIT;
    }

    /* Traverse each sample for section hash generation. */
    while ((entFile = readdir(dirRoot)) != NULL) {
        printf("%s\n", entFile->d_name);
    }

    closedir(dirRoot);

EXIT:
    return rc;
}


int grp_group_hash(GROUP *self) {
    int rc;

    rc = 0;

    return rc;
}


/*-----------------------------------------------------------*
 *          Implementation for Internal Functions            *
 *-----------------------------------------------------------*/
