#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif

internal_proc void
os_path_normalize(char path[MAX_PATH]) {
    char *ptr;
    for ( ptr = path; *ptr; ++ptr ) {
        if ( *ptr == '\\' ) {
            *ptr = '/';
        }
    }

    if ( ptr != path && ptr[-1] == '/' ) {
        ptr[-1] = 0;
    }
}

internal_proc char *
os_path_file(char path[MAX_PATH]) {
    os_path_normalize(path);
    for (char *ptr = path + _mbstrlen(path); ptr != path; ptr--) {
        if (ptr[-1] == '/') {
            return ptr;
        }
    }
    return path;
}

#ifdef _MSC_VER
#include "os_win32.cpp"
#else
#error "nicht unterstütztes system!"
#endif

