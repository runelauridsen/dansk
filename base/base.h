
#if _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <intrin.h>
#else
#   include <time.h> // NOTE(rune): For clock_getttime
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h> // NOTE(rune): Macros for printf'ing int64_t etc...

#ifdef __cplusplus
#include "base_core.h"
#include "base_memory.h"
#include "base_memory.c"
#else
#include "base_ansi.h"
#include "base_core.h"

// rune: [h] core
#include "base_memory.h"
#include "base_string.h"
#include "base_buf.h"
#include "base_map.h"
#include "base_unicode.h"
#include "base_fmt.h"

// rune: [c] core
#include "base_memory.c"
#include "base_string.c"
#include "base_buf.c"
#include "base_map.c"
#include "base_unicode.c"
#include "base_fmt.c"

// rune: [h,c] os
#include "base_os.h"
#include "base_os.c"

// rune: [h,c] testing
#include "base_test.h"
#include "base_test.c"
#endif
