#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif

internal_proc void
path_join(char dest[MAX_PATH], char path[MAX_PATH]) {
    char *ptr = dest + strlen(dest);
    if ( ptr != dest && ptr[-1] == '/' ) {
        ptr--;
    }

    if ( *path == '/' ) {
        path++;
    }

    snprintf(ptr, dest + MAX_PATH - ptr, "/%s", path);
}

internal_proc void
path_normalize(char path[MAX_PATH]) {
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

internal_proc void
path_copy(char path[MAX_PATH], char *src) {
    strncpy(path, src, MAX_PATH);
    path[MAX_PATH - 1] = 0;
    path_normalize(path);
}

internal_proc char *
path_file(char path[MAX_PATH]) {
    path_normalize(path);
    for (char *ptr = path + strlen(path); ptr != path; ptr--) {
        if (ptr[-1] == '/') {
            return ptr;
        }
    }
    return path;
}

internal_proc char *
path_ext(char path[MAX_PATH]) {
    for (char *ptr = path + strlen(path); ptr != path; ptr--) {
        if (ptr[-1] == '.') {
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

internal_proc char **
dir_buf(char *filespec) {
    char **files = NULL;
    for ( Dir_Iterator it = dir_list("../data"); it.is_valid; dir_next(&it) ) {
        buf_push(files, _strdup(it.name));
    }

    return files;
}

