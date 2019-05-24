#include <stdio.h>
#include "mas.h"
#include "mas.yh"

struct instab instab[] = {
{0, "space",  ISPACE-256+BWL},
{0, "byte",  IBYTE-256+BWL},
{0, "short",  ISHORT-256+BWL},
{0, "long",  ILONG-256+BWL},
{0, "data",  IDATA-256+BWL},
{0, "global",  IGLOBAL-256+BWL},
{0, "set",  ISET-256+BWL},
{0, "text",  ITEXT-256+BWL},
{0, "comm",  ICOMM-256+BWL},
{0, "lcomm",  ILCOMM-256+BWL},
{0, "even",  IEVEN-256+BWL},
{0, "org",  IORG-256+BWL},
{0, "stabs", ISTABS-256+BWL},
{0, "stabn", ISTABN-256+BWL},
{0, "stabd", ISTABD-256+BWL},
{0, "pcrel", IPCREL-256+BWL},
{0, "optim", IOPTIM-256+BWL},
{0, "noopt", IOPTIM-256+BWL},
#include "ops.c"
0,
};
