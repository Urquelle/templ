#ifdef _MSC_VER
#    include "os_win32.cpp"
#elif __linux__
#    include "os_linux.cpp"
#else
#    error "nicht unterstütztes system!"
#endif

#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <locale>

