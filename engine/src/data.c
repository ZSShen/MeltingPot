#include <stdlib.h>
#include "data.h"


void
DsFreeNameArray(gpointer gp_Slc)
{
    free(gp_Slc);
    return;    
}


void
DsFreeHashArray(gpointer gp_Name)
{
    free(gp_Name);
    return;
}
