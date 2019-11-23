#include <shlwapi.h>

internal_proc char *
os_env(char *name) {
    char *result = getenv(name);

    return result;
}

internal_proc b32
os_file_read(char *filename, char **result) {
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if ( file == INVALID_HANDLE_VALUE ) {
        return false;
    }

    HANDLE file_mapping = CreateFileMapping(file, 0, PAGE_WRITECOPY, 0, 0, 0);
    *result = (char *)MapViewOfFileEx(file_mapping, FILE_MAP_COPY, 0, 0, 0, 0);

    CloseHandle(file);

    return true;
}

internal_proc b32
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

internal_proc b32
os_file_exists(char *filename) {
    b32 result = PathFileExistsA(filename);

    return result;
}

