internal_proc char *
os_env(char *name) {
    char *result = getenv(name);

    return result;
}

internal_proc b32
file_read(char *filename, char **result) {
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if ( file == INVALID_HANDLE_VALUE ) {
        return false;
    }

    // result->size = GetFileSize(file, 0);
    HANDLE file_mapping = CreateFileMappingA(file, 0, PAGE_WRITECOPY, 0, 0, 0);
    *result = (char *)MapViewOfFileEx(file_mapping, FILE_MAP_COPY, 0, 0, 0, 0);

    return true;
}

internal_proc b32
file_write(char *filename, char *data, size_t len) {
    HANDLE file = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if ( file == INVALID_HANDLE_VALUE ) {
        return false;
    }

    DWORD bytes_written = 0;
    HRESULT h = WriteFile(file, data, (DWORD)len, &bytes_written, NULL);

    CloseHandle(file);

    return SUCCEEDED(h);
}

internal_proc void
path_canonical(char *dest, char *path) {
    path_normalize(path);
    size_t size = strlen(path);
    DWORD result = GetFullPathNameA(path, MAX_PATH, dest, NULL);
}

struct Dir_Iterator {
    b32 is_valid;
    b32 error;
    b32 is_dir;
    size_t size;

    char name[MAX_PATH];
    char path[MAX_PATH];
    char query[MAX_PATH];
    char full_path[MAX_PATH];

    intptr_t _handle;
};

internal_proc void
dir_free(Dir_Iterator *it) {
    if ( it->is_valid ) {
        FindClose((HANDLE)it->_handle);
    }
}

internal_proc void
dir__update(Dir_Iterator *it, WIN32_FIND_DATA *data, b32 done) {
    it->is_dir = false;
    if ( data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
        it->is_dir = true;
    }

    if ( !done ) {
        size_t len = strlen(data->cFileName);
        memcpy(it->name, data->cFileName, len);
        it->name[len] = 0;

        path_copy(it->full_path, it->path);
        path_join(it->full_path, it->name);

        if ( data->nFileSizeHigh ) {
            it->size = ((size_t)data->nFileSizeHigh << 32) | (size_t)data->nFileSizeLow;
        } else {
            it->size = (size_t)data->nFileSizeLow;
        }
    }
}

internal_proc b32
dir__excluded(Dir_Iterator *it) {
    if ( strcmp(it->name, ".") == 0 || strcmp(it->name, "..") == 0 ) {
        return true;
    }
    return false;
}

internal_proc void
dir_next(Dir_Iterator *it) {
    WIN32_FIND_DATA data = {};
    b32 done = FindNextFile((HANDLE)it->_handle, &data) == 0;
    it->is_valid = !done;
    it->error    = done && GetLastError() != ERROR_NO_MORE_FILES;

    dir__update(it, &data, done);
    if ( it->is_valid && dir__excluded(it) ) {
        dir_next(it);
    }

    if ( done ) {
        dir_free(it);
    }
}

internal_proc Dir_Iterator
dir_list(char *filespec) {
    Dir_Iterator result = {};

    path_copy(result.path, filespec);
    path_copy(result.query, filespec);

    if ( *(result.query + strlen(result.query) - 1) != '*' ) {
        path_join(result.query, "*");
    }

    WIN32_FIND_DATA data = {};
    path_normalize(filespec);
    HANDLE handle = FindFirstFileA(result.query, &data);

    result.is_valid = true;
    if ( handle == INVALID_HANDLE_VALUE ) {
        result.is_valid = false;
        result.error    = true;

        return result;
    }

    result.error = ( handle == INVALID_HANDLE_VALUE ) && GetLastError();
    result._handle = (intptr_t)handle;

    dir__update(&result, &data, result.error);
    if ( result.is_valid && dir__excluded(&result) ) {
        dir_next(&result);
    }

    return result;
}
