#include <stdlib.h>
#include "data.h"


void
DsFreeNameArray(gpointer gp_Name)
{
    char *szName = (char*)gp_Name;
    if (szName) {
        free(szName);
    }

    return;
}


void
DsFreeHashArray(gpointer gp_Hash)
{
    char *szHash = (char*)gp_Hash;
    if (szHash) {
        free(szHash);
    }

    return;
}
