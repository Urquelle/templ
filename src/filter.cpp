internal_proc FILTER_CALLBACK(filter_abs) {
    assert(val->kind == VAL_INT);
    s32 i = abs(val_int(val));

    return val_int(i);
}

internal_proc FILTER_CALLBACK(filter_capitalize) {
    assert(val->kind == VAL_STR);

    char first_letter = ((char *)val->ptr)[0];
    ((char *)val->ptr)[0] = std::toupper(first_letter, std::locale());

    return val_str( val_str(val) );
}

internal_proc FILTER_CALLBACK(filter_default) {
    assert(val->kind == VAL_STR);
    assert(num_args > 0);

    if ( !val->len ) {
        return args[0]->val;
    }

    return val;
}

internal_proc FILTER_CALLBACK(filter_upper) {
    assert(val->kind == VAL_STR);
    char *str = val_str(val);
    char *result = "";

    size_t offset = 0;
    for ( int i = 0; i < strlen(str); ++i ) {
        result = strf("%s%s", result, utf8_char_to_uppercase(str + offset));
        offset += utf8_char_size(str + offset);
    }

    return val_str(result);
}

internal_proc FILTER_CALLBACK(filter_escape) {
    char *result = "";

    for ( int i = 0; i < val->len; ++i ) {
        char c = ((char *)val->ptr)[i];

        erstes_if ( c == '<' ) {
            result = strf("%s%s", result, "&lt;");
        } else if ( c == '>' ) {
            result = strf("%s%s", result, "&gt;");
        } else if ( c == '&' ) {
            result = strf("%s%s", result, "&amp;");
        } else if ( c == ' ' ) {
            result = strf("%s%s", result, "&nbsp;");
        } else {
            result = strf("%s%c", result, c);
        }
    }

    return val_str(result);
}

internal_proc FILTER_CALLBACK(filter_format) {
    assert(val->kind == VAL_STR);

    return val_str("<!-- @AUFGABE: format implementieren -->");
}

internal_proc FILTER_CALLBACK(filter_truncate) {
    assert(val->kind == VAL_STR);

    return val_str("<!-- @AUFGABE: truncate implementieren -->");
}

