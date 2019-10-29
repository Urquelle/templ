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
    for ( int i = 0; i < os_strlen(str); ++i ) {
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

    char *format = val_str(val);
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
                if ( num_arg < num_args ) {
                    Resolved_Arg *arg = args[num_arg];

                    if ( arg->val->kind != VAL_STR ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss string sein", arg->name);
                    }

                    result = strf("%s%s", result, val_to_char(arg->val));
                } else {
                    if ( num_args ) {
                        Resolved_Arg *arg = args[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_args);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else if ( format[0] == 'd' ) {
                if ( num_arg < num_args ) {
                    Resolved_Arg *arg = args[num_arg];

                    if ( arg->val->kind != VAL_INT ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss int sein", arg->name);
                    }

                    result = strf("%s%s", result, val_to_char(arg->val));
                } else {
                    if ( num_args ) {
                        Resolved_Arg *arg = args[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_args);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else if ( format[0] == 'f' ) {
                if ( num_arg < num_args ) {
                    Resolved_Arg *arg = args[num_arg];

                    if ( arg->val->kind != VAL_FLOAT ) {
                        fatal(arg->pos.name, arg->pos.row, "der datentyp des übergebenen arguments %s muss float sein", arg->name);
                    }

                    result = strf("%s%.*f", result, size, val_float(arg->val));
                } else {
                    if ( num_args ) {
                        Resolved_Arg *arg = args[0];
                        warn(arg->pos.name, arg->pos.row, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_args);
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

internal_proc FILTER_CALLBACK(filter_truncate) {
    assert(val->kind == VAL_STR);

    return val_str("<!-- @AUFGABE: truncate implementieren -->");
}

