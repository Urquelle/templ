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

internal_proc size_t
utf8_str_uppersize(char *str) {
    char *ptr = str;
    size_t len = utf8_strlen(str);
    size_t result = 0;

    for ( int i = 0; i < len; ++i ) {
        size_t old_size = utf8_char_size(ptr);
        char *c = utf8_char_toupper(ptr);
        size_t size = utf8_char_size(c);
        result += size;
        ptr += old_size;
    }

    return result;
}

internal_proc size_t
utf8_str_lowersize(char *str) {
    char *ptr = str;
    size_t len = utf8_strlen(str);
    size_t result = 0;

    for ( int i = 0; i < len; ++i ) {
        size_t old_size = utf8_char_size(ptr);
        char *c = utf8_char_tolower(ptr);
        size_t size = utf8_char_size(c);
        result += size;
        ptr += old_size;
    }

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

internal_proc s32
utf8_strcmp(char *left, char *right) {
    return strcmp(left, right);
}

global_var char global_toupper_buf[5];
internal_proc char *
utf8_char_toupper(char *str) {
    /*
     * @INFO: tabelle mit werten für die zeichen
     *        https://unicode-table.com/
     *        https://www.utf8-chartable.de/unicode-utf8-table.pl
     */

    size_t size = utf8_char_size(str);
    memcpy(global_toupper_buf, str, size);

    u8 c0 =                (u8)str[0];
    u8 c1 = ( size > 1 ) ? (u8)str[1] : 0;
    u8 c2 = ( size > 2 ) ? (u8)str[2] : 0;

    erstes_if (
        /* @INFO: ascii */
        ( size == 1 && *str >= 'a' && *str <= 'z' ) ||

        /* @INFO: latin */
        ( size == 2 && c0 == 0xc3 && ( c1 >= 0xa0 && c1 <= 0xb6 || c1 >= 0xb8 && c1 <= 0xbe ) ) ||

        /* @INFO: абвгдежзийклмноп */
        ( size == 2 && c0 == 0xd0 && c1 >= 0xb0 && c1 <= 0xbf )
    ) {
        global_toupper_buf[size-1] -= 0x20;
        global_toupper_buf[size]    = 0;
    } else if ( size == 2 ) {
        /* @INFO: ß */
        erstes_if ( c0 == 0xc3 && c1 == 0x9f ) {
            global_toupper_buf[0] = (u8)0xe1;
            global_toupper_buf[1] = (u8)0xba;
            global_toupper_buf[2] = (u8)0x9e;
            global_toupper_buf[3] = 0;

        /*  @INFO: ÿ */
        } else if ( c0 == 0xc3 && c1 == 0xbf ) {
            global_toupper_buf[0] = (u8)0xc5;
            global_toupper_buf[1] = (u8)0xb8;
            global_toupper_buf[2] = 0;
        } else if (
            ( c0 == 0xc4 && c1 <= 0xb7 && (c1 & 0x1) == 1 ) ||
            ( c0 == 0xc4 && c1 >= 0xba && c1 <= 0xbe && (c1 & 0x1) == 0 ) ||
            ( c0 == 0xc5 && c1 >= 0x82 && c1 <= 0x88 && (c1 & 0x1) == 0 ) ||
            ( c0 == 0xc5 && c1 >= 0x8b && c1 <= 0xbe && (c1 & 0x1) == 1 )
        ) {
            global_toupper_buf[1] -= 0x1;
            global_toupper_buf[2]  = 0;

        } else if ( c0 == 0xc5 && c1 == 0x80 ) {
            global_toupper_buf[0] -= 0x01;
            global_toupper_buf[1] += 0x3f;
            global_toupper_buf[2]  = 0;

        /* @INFO: рстуфхцчшщъыьэюя ё */
        } else if ( c0 == 0xd1 ) {

            /* @INFO: рстуфхцчшщъыьэюя */
            if ( c1 <= 0x8f ) {
                global_toupper_buf[0] -= 0x01;
                global_toupper_buf[1] += 0x20;

            /* @INFO: ё */
            } else if ( c1 == 0x91 ) {
                global_toupper_buf[0] -= 0x01;
                global_toupper_buf[1] -= 0x10;
            }

            global_toupper_buf[2] = 0;
        }

    /* @AUFGABE: überprüfen ob zeichen klein ist, bevor konvertierung */
    } else if ( size == 3 ) {
        /* @AUFGABE: implementieren */

    /* @AUFGABE: überprüfen ob zeichen klein ist, bevor konvertierung */
    } else if ( size == 4 ) {
        /* @AUFGABE: implementieren */
    }

    return global_toupper_buf;
}

internal_proc char *
utf8_str_toupper(char *str) {
    size_t offset = 0;
    size_t size = utf8_str_uppersize(str);

    char *result = (char *)calloc(1, size+1);

    for ( int i = 0; i < utf8_strlen(str); ++i ) {
        size_t old_len = utf8_char_size(str + offset);
        char *c = utf8_char_toupper(str + offset);
        size_t len = utf8_char_size(c);

        sprintf_s(result, size+1, "%s%.*s", result, (int)len, c);
        offset += old_len;
    }
    result[size] = 0;

    return result;
}

global_var char global_tolower_buf[5];
internal_proc char *
utf8_char_tolower(char *str) {
    /*
     * @INFO: tabelle mit werten für die zeichen
     *        https://unicode-table.com/
     *        https://www.utf8-chartable.de/unicode-utf8-table.pl
     */

    size_t size = utf8_char_size(str);
    memcpy(global_tolower_buf, str, size);

    u8 c0 = (u8)str[0];
    u8 c1 = ( size > 1 ) ? (u8)str[1] : 0;
    u8 c2 = ( size > 2 ) ? (u8)str[2] : 0;

    erstes_if (
        /* @INFO: ascii */
        ( size == 1 && *str >= 'A' && *str <= 'Z' ) ||

        /* @INFO: latin */
        ( size == 2 && c0 == 0xc3 && c1 >= 0x80 && c1 <= 0x9e ) ||

        /* @INFO: АБВГДЕЖЗИЙКЛМНОП */
        ( size == 2 && c0 == 0xd0 && c1 >= 0x90 && c1 <= 0x9f )
    ) {
        global_tolower_buf[size-1] += 0x20;
        global_tolower_buf[size]    = 0;
    } else if ( size == 2 ) {
        /* @INFO: РСТУФХЦЧШЩЪЫЬЭЮЯ */
        if ( c0 == 0xd0 && c1 >= 0xa0 && c1 <= 0xaf) {
            global_tolower_buf[0] += 0x01;
            global_tolower_buf[1] -= 0x20;
            global_tolower_buf[2] = 0;

        /* @INFO: Ё */
        } else if ( c0 == 0xd0 && c1 == 0x81 ) {
            global_tolower_buf[0] += 0x01;
            global_tolower_buf[1] += 0x10;
            global_tolower_buf[2] = 0;
        } else if (
            ( c0 == 0xc4 && c1 <= 0xb6 && (c1 & 0x1) == 0 ) ||
            ( c0 == 0xc4 && c1 >= 0xb9 && c1 <= 0xbd && (c1 & 0x1) == 1 ) ||
            ( c0 == 0xc5 && c1 >= 0x81 && c1 <= 0x87 && (c1 & 0x1) == 1 ) ||
            ( c0 == 0xc5 && c1 >= 0x8a && c1 <= 0xbd && (c1 & 0x1) == 0 )
        ) {
            global_tolower_buf[1] += 0x1;
            global_tolower_buf[2]  = 0;

        } else if ( c0 == 0xc4 && c1 == 0xbf ) {
            global_tolower_buf[0] += 0x01;
            global_tolower_buf[1] -= 0x3f;
            global_tolower_buf[2]  = 0;

        /* @INFO: Ÿ */
        } else if ( c0 == 0xc5 && c1 == 0xb8 ) {
            global_tolower_buf[0] = (u8)0xc3;
            global_tolower_buf[1] = (u8)0xbf;
            global_tolower_buf[2] = 0;
        }
    } else if ( size == 3 ) {

        /* @INFO: ẞ */
        if ( c0 == 0xe1 && c1 == 0xba && c2 == 0x9e ) {
            global_tolower_buf[0] = (u8)0xc3;
            global_tolower_buf[1] = (u8)0x9f;
            global_tolower_buf[2] = 0;
        } else {
            /* @AUFGABE: implementieren */
        }
    } else if ( size == 4 ) {
        /* @AUFGABE: implementieren */
    }

    return global_tolower_buf;
}

internal_proc char *
utf8_str_tolower(char *str) {
    size_t size = utf8_str_lowersize(str);
    char *result = (char *)calloc(1, size+1);
    size_t str_len = utf8_strlen(str);
    size_t offset = 0;

    for ( int i = 0; i < str_len; ++i ) {
        size_t old_len = utf8_char_size(str + offset);
        char *c = utf8_char_tolower(str + offset);
        size_t len = utf8_char_size(c);

        sprintf_s(result, size+1, "%s%.*s", result, (int)len, c);
        offset += old_len;
    }
    result[size] = 0;

    return result;
}

