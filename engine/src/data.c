#include <stdlib.h>
#include "data.h"


void
DsFreeNameArray(gpointer gp_Name)
{
    if (gp_Name)
        free(gp_Name);

    return;
}


void
DsFreeHashArray(gpointer gp_Hash)
{
    if (gp_Hash)
        free(gp_Hash);

    return;
}


void
DsFreeBindArray(gpointer gp_Bind)
{
    if (gp_Bind)
        free(gp_Bind);

    return;
}
