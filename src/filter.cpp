internal_proc PROC_CALLBACK(filter_abs) {
    assert(operand->kind == VAL_INT);
    s32 i = abs(val_int(operand));

    return val_int(i);
}

internal_proc PROC_CALLBACK(filter_attr) {
    Scope *scope = (Scope *)operand->ptr;
    Scope *prev_scope = scope_set(scope);

    Sym *sym = sym_get(val_str(narg("name")->val));
    Val *result = sym->val;

    scope_set(prev_scope);

    return result;
}

internal_proc PROC_CALLBACK(filter_batch) {
    return val_str("/* @IMPLEMENT: filter batch */");
}

internal_proc PROC_CALLBACK(filter_capitalize) {
    assert(operand->kind == VAL_STR);

    char *cap = utf8_toupper((char *)operand->ptr);
    size_t cap_size = utf8_char_size(cap);
    char *remainder = (char *)operand->ptr + cap_size;

    char *result = strf("%.*s%s", cap_size, cap, remainder);

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_center) {
    int width = val_int(narg("width")->val);

    if ( operand->len >= width ) {
        return operand;
    }

    s32 padding = (s32)((width - operand->len) / 2);

    char *result = strf("%*s%s%*s", padding, "", val_to_char(operand), padding, "");

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_default) {
    char *default_value = val_str(narg("s")->val);
    b32 boolean = val_bool(narg("boolean")->val);

    if ( val_is_undefined(operand) || boolean && !val_is_true(operand) ) {
        return val_str(default_value);
    }

    return operand;
}





internal_proc PROC_CALLBACK(filter_escape) {
    char *result = "";

    for ( int i = 0; i < operand->len; ++i ) {
        char c = ((char *)operand->ptr)[i];

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

internal_proc PROC_CALLBACK(filter_format) {
    assert(operand->kind == VAL_STR);

    char *format = val_str(operand);
    int num_arg = 0;
    char *result = "";

    while ( format[0] ) {
        if ( format[0] == '%' ) {
            int size = 6;
            format = utf8_char_next(format);

            if ( format[0] == '.' ) {
                size = 0;
                format = utf8_char_next(format);

                while ( is_numeric(format[0]) ) {
                    size *= 10;
                    size += format[0] - '0';

                    format = utf8_char_next(format);
                }
            }

            if ( format[0] == 's' ) {
                if ( num_arg < num_varargs ) {
                    Resolved_Arg *arg = varargs[num_arg];

                    if ( arg->val->kind != VAL_STR ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss string sein", arg->name);
                    }

                    result = strf("%s%s", result, val_to_char(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else if ( format[0] == 'd' ) {
                if ( num_arg < num_varargs ) {
                    Resolved_Arg *arg = varargs[num_arg];

                    if ( arg->val->kind != VAL_INT ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss int sein", arg->name);
                    }

                    result = strf("%s%s", result, val_to_char(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else if ( format[0] == 'f' ) {
                if ( num_arg < num_varargs ) {
                    Resolved_Arg *arg = varargs[num_arg];

                    if ( arg->val->kind != VAL_FLOAT ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss float sein", arg->name);
                    }

                    result = strf("%s%.*f", result, size, val_float(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else {
                illegal_path();
            }
        } else {
            result = strf("%s%.*s", result, utf8_char_size(format), format);
        }

        format = utf8_char_next(format);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_lower) {
    assert(operand->kind == VAL_STR);
    char *str = val_str(operand);
    char *result = "";

    size_t offset = 0;
    for ( int i = 0; i < utf8_strlen(str); ++i ) {
        size_t old_len = utf8_char_size(str + offset);
        char *c = utf8_tolower(str + offset);
        size_t len = utf8_char_size(c);

        result = strf("%s%.*s", result, len, c);
        offset += old_len;
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_truncate) {
    size_t len = MIN(operand->len, val_int(narg("length")->val));
    s32 leeway = val_int(narg("leeway")->val);

    u64 diff = abs((s64)(len - operand->len));
    if ( diff <= leeway ) {
        return operand;
    }

    char *result = "";
    char *ptr = val_str(operand);

    b32 killwords = val_bool(narg("killwords")->val);
    if ( !killwords ) {
        char *ptrend = utf8_char_goto(ptr, operand->len-1);
        u32 char_count = 0;
        for ( size_t i = operand->len-1; i >= len; --i ) {
            if ( *ptrend == ' ' ) {
                char_count = 0;
            } else {
                char_count += 1;
            }
            ptrend = utf8_char_goto(ptr, i-1);
        }

        if ( char_count && ptrend != ptr ) {
            size_t pos = len;
            while ( ptrend != ptr && *ptrend != ' ' ) {
                ptrend = utf8_char_goto(ptr, --pos);
            }
            len = pos;
        }
    }

    for ( int i = 0; i < len; ++i ) {
        result = strf("%s%.*s", result, utf8_char_size(ptr), ptr);
        ptr += utf8_char_size(ptr);
    }

    char *end = val_str(narg("end")->val);
    size_t end_len = utf8_strlen(end);
    for ( int i = 0; i < end_len; ++i ) {
        result = strf("%s%.*s", result, utf8_char_size(end), end);
        end += utf8_char_size(end);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(filter_upper) {
    assert(operand->kind == VAL_STR);
    char *str = val_str(operand);
    char *result = "";

    size_t offset = 0;
    for ( int i = 0; i < utf8_strlen(str); ++i ) {
        size_t old_len = utf8_char_size(str + offset);
        char *c = utf8_toupper(str + offset);
        size_t len = utf8_char_size(c);

        result = strf("%s%.*s", result, len, c);
        offset += old_len;
    }

    return val_str(result);
}

