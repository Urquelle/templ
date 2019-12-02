#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif

namespace templ {

enum { MAX_ENV_VAR_LENGTH = 32767 };
static char global_env_var_buf[MAX_ENV_VAR_LENGTH];
static char *
os_env(char *name) {
    GetEnvironmentVariable(name, global_env_var_buf, MAX_ENV_VAR_LENGTH);

    return global_env_var_buf;
}

static bool
os_file_read(char *filename, char **result) {
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if ( file == INVALID_HANDLE_VALUE ) {
        return false;
    }

    HANDLE file_mapping = CreateFileMapping(file, 0, PAGE_WRITECOPY, 0, 0, 0);
    *result = (char *)MapViewOfFileEx(file_mapping, FILE_MAP_COPY, 0, 0, 0, 0);

    return true;
}

static bool
os_file_write(char *filename, char *data, size_t len) {
    HANDLE file = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if ( file == INVALID_HANDLE_VALUE ) {
        return false;
    }

    DWORD bytes_written = 0;
    HRESULT h = WriteFile(file, data, (DWORD)len, &bytes_written, NULL);

    CloseHandle(file);

    return SUCCEEDED(h);
}

static bool
os_file_exists(char *filename) {
    bool result = PathFileExistsA(filename);

    return result;
}

static int
os_sprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, size, format, args);
    va_end(args);

    return result;
}

static void *
os_mem_alloc(size_t size) {
    void *result = VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    return result;
}

}

