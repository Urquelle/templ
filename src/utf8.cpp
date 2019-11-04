internal_proc size_t
utf8_char_size(char *str) {
    erstes_if ( (*str & 0x80) == 0x00 ) {
        return 1;
    } else if ( (*str & 0xE0) == 0xc0 ) {
        return 2;
    } else if ( (*str & 0xF0) == 0xE0 ) {
        return 3;
    } else if ( (*str & 0xF0) == 0xF0 ) {
        return 4;
    } else {
        illegal_path();
    }

    return 0;
}

internal_proc size_t
utf8_strlen(char *str) {
    size_t result = 0;
    char *ptr = str;

    while ( *ptr ) {
        size_t size = utf8_char_size(ptr);
        ptr += size;
        result++;
    }

    return result;
}

internal_proc size_t
utf8_str_size(char *str, size_t len) {
    size_t result = 0;
    char *ptr = str;

    for ( int i = 0; i < len; ++i ) {
        result += utf8_char_size(ptr+result);
    }

    return result;
}

internal_proc size_t
utf8_str_size(char *str) {
    size_t len = utf8_strlen(str);
    size_t result = utf8_str_size(str, len);

    return result;
}

internal_proc void
utf8_char_write(char *dest, char *c) {
    size_t num_bytes = utf8_char_size(c);

    for ( int i = 0; i < num_bytes; ++i ) {
        dest[i] = c[i];
    }
}

internal_proc char *
utf8_char_next(char *input) {
    char *result = input + utf8_char_size(input);

    return result;
}

internal_proc size_t
utf8_char_offset(char *ptr, char *c) {
    size_t result = c - ptr;

    return result;
}

internal_proc char *
utf8_char_lastbyte(char *input) {
    char *result = input + utf8_char_size(input) - 1;

    return result;
}

internal_proc char *
utf8_char_goto(char *input, size_t count) {
    size_t len = utf8_strlen(input);
    if ( len < count ) {
        return input;
    }

    char *result = input;
    for ( int i = 0; i < count; ++i ) {
        result += utf8_char_size(result);
    }

    return result;
}

internal_proc char *
utf8_toupper(char *str) {
    size_t size = utf8_char_size(str);

    if ( size == 1 && *str >= 'a' && *str <= 'z') {
        *str -= 0x20;
    } else if ( size == 2 ) {
        /* @AUFGABE: implementieren */
    } else if ( size == 3 ) {
        /* @AUFGABE: implementieren */
    } else if ( size == 4 ) {
        /* @AUFGABE: implementieren */
    }

    return str;
}

