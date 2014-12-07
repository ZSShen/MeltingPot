#include <stdlib.h>
#include <stdbool.h>
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


void
DsFreeKeyGroupHash(gpointer gp_Key)
{
    return;
}


void
DsFreeValueGroupHash(gpointer gp_Val)
{
    if (gp_Val) {
        GROUP *p_Grp = (GROUP*)gp_Val;
        if (p_Grp->a_Mbr)
            g_array_free(p_Grp->a_Mbr, true);
        if (p_Grp->a_BlkCand)
            g_ptr_array_free(p_Grp->a_BlkCand, true);
    }

    return;
}
