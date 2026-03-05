#ifndef RMR_ZERO_COMPAT_H
#define RMR_ZERO_COMPAT_H

#if defined(RMR_JNI_BUILD) && RMR_JNI_BUILD
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

#ifndef rmr_malloc
#define rmr_malloc(sz) malloc(sz)
#endif
#ifndef rmr_free
#define rmr_free(p) free(p)
#endif
#else
#include "rmr_baremetal_compat.h"
#endif

#include "rmr_corelib.h"

#endif
