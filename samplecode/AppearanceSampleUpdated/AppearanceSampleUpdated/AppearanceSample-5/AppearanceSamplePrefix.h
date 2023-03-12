#define TARGET_CARBON 	1

#include "PrefixCommon.h"

#ifndef CARBON_ON_MACH_O
#define CARBON_ON_MACH_O	0
#endif

#ifndef BUILDING_FOR_CARBON_8
#define BUILDING_FOR_CARBON_8	0
#endif

#ifndef HELP_TAGS_ENABLED
#define HELP_TAGS_ENABLED	1
#endif

#include <Carbon/Carbon.h>
