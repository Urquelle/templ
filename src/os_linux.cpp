#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"

namespace templ {

static char *
os_env(char *name) {
    char *result = getenv(name);

    return result;
}

static bool
os_file_read(char *filename, char **result) {
    int fd = open(filename, O_RDONLY);

    if ( fd == -1 ) {
        return false;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        return false;
    }

    char *addr = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if ( addr == MAP_FAILED ) {
        return false;
    }

    *result = addr;

    close(fd);

    return true;
}

static bool
os_file_write(char *filename, char *data, size_t len) {
    int fd = open(filename, O_WRONLY);

    if ( fd == -1 ) {
        return false;
    }

    size_t sz = write(fd, data, len);
    if ( sz != len ) {
        return false;
    }

    close(fd);

    return false;
}

static int
os_sprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, size, format, args);
    va_end(args);

    return result;
}

static bool
os_file_exists(char *filename) {
    struct stat sb;
    if ( stat(filename, &sb) == 0 ) {
        return true;
    }

    return false;
}

static void *
os_mem_alloc(size_t size) {
    void *result = malloc(size);

    return result;
}

}

