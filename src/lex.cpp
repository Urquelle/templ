enum Token_Kind {
    T_EOF,
    T_INT = 129,
    T_LIT,
    T_STR,
    T_CHAR,
    T_NAME,
    T_DOT,
    T_COMMA,
    T_RANGE,
    T_LPAREN,
    T_RPAREN,
    T_LBRACKET,
    T_RBRACKET,
    T_LBRACE,
    T_RBRACE,
    T_BAR,
    T_HASH,
    T_PERCENT,
    T_QMARK,
    T_ASSIGN,
    T_COLON,
    T_PLUS,
    T_MUL,

    T_SPACE,
    T_TAB,
    T_NEWLINE,
    T_COMMENT,

    T_LT,
    T_LEQ,
    T_EQL,
    T_UNEQL,
    T_GEQ,
    T_GT,

    T_VAR_BEGIN,
    T_VAR_END,
    T_CODE_BEGIN,
    T_CODE_END,
};

struct Token {
    int kind;
    char *literal;

    union {
        char *str_value;
        int   int_value;
        int   char_value;
        char *name;
    };
};

struct Lexer {
    Token token;
    char *input;
    char at[2];
};

internal_proc void
refill(Lexer *lex) {
    if ( lex->input[0] == 0 ) {
        lex->at[0] = 0;
        lex->at[1] = 0;
    } else if ( lex->input[1] == 0 ) {
        lex->at[0] = lex->input[0];
        lex->at[1] = 0;
    } else {
        lex->at[0] = lex->input[0];
        lex->at[1] = lex->input[1];
    }
}

internal_proc void
next(Lexer *lex, int count = 1) {
    for ( int i = 0; i < count; ++i ) {
        if (lex->input == 0) break;
        lex->input++;
    }
    refill(lex);
}

internal_proc void
skip_comment(Lexer *lex) {
    while ( lex->input ) {
        if ( lex->at[0] == '#' && lex->at[1] == '}' ) {
            next(lex, 2);
            return;
        }

        if ( lex->at[0] == '{' && lex->at[1] == '#' ) {
            next(lex, 2);
            skip_comment(lex);
        }

        next(lex);
    }
}

internal_proc b32
is_raw(Lexer *lex) {
    Token t = lex->token;
    b32 result = T_SPACE <= t.kind && t.kind <= T_COMMENT;

    return result;
}

internal_proc b32
is_space(Lexer *lex) {
    Token t = lex->token;
    b32 result = t.kind == T_SPACE || t.kind == T_TAB;

    return result;
}

internal_proc b32
is_lit(Lexer *lex) {
    b32 result = true;

    if ( lex->at[0] == '{' ) {
        if ( lex->at[1] == '{' || lex->at[1] == '#' || lex->at[1] == '%' ) {
            result = false;
        }
    }

    return result;
}

internal_proc void
next_raw_token(Lexer *lex) {
    char *start = lex->input;

    switch ( lex->at[0] ) {
        case 0: {
            lex->token.kind = T_EOF;
            next(lex);
        } break;

        case ',': {
            lex->token.kind = T_COMMA;
            next(lex);
        } break;

        case '.': {
            lex->token.kind = T_DOT;
            next(lex);

            if ( lex->at[0] == '.' ) {
                lex->token.kind = T_RANGE;
                next(lex);
            }
        } break;

        case '\'': {
            lex->token.kind = T_CHAR;
            next(lex);

            if ( lex->at[0] == '\\' ) {
                next(lex);
            }

            if ( lex->at[1] != '\'' ) {
                assert(!"fehler beim parsen eines char tokens");
            }

            lex->token.char_value = lex->at[0];
            next(lex, 2);
        } break;

        case '(': {
            lex->token.kind = T_LPAREN;
            next(lex);
        } break;

        case ')': {
            lex->token.kind = T_RPAREN;
            next(lex);
        } break;

        case '[': {
            lex->token.kind = T_LBRACKET;
            next(lex);
        } break;

        case ']': {
            lex->token.kind = T_RBRACKET;
            next(lex);
        } break;

        case '=': {
            lex->token.kind = T_ASSIGN;
            next(lex);

            if ( lex->at[0] == '=' ) {
                lex->token.kind = T_EQL;
                next(lex);
            }
        } break;

        case '<': {
            lex->token.kind = T_LT;
            next(lex);

            if ( lex->at[0] == '=' ) {
                lex->token.kind = T_LEQ;
                next(lex);
            }
        } break;

        case '+': {
            lex->token.kind = T_PLUS;
            next(lex);
        } break;

        case '*': {
            lex->token.kind = T_MUL;
            next(lex);
        } break;

        case '?': {
            lex->token.kind = T_QMARK;
            next(lex);
        } break;

        case ':': {
            lex->token.kind = T_COLON;
            next(lex);
        } break;

        case ' ': {
            lex->token.kind = T_SPACE;
            next(lex);
        } break;

        case '\t': {
            lex->token.kind = T_TAB;
            next(lex);
        } break;

        case '\n': {
            lex->token.kind = T_NEWLINE;
            next(lex);
        } break;

        case '\v': {
            lex->token.kind = T_SPACE;
            next(lex);
        } break;

        case '{': {
            lex->token.kind = T_LBRACE;
            next(lex);

            if ( lex->at[0] == '{' ) {
                lex->token.kind = T_VAR_BEGIN;
                next(lex);
            } else if ( lex->at[0] == '#' ) {
                lex->token.kind = T_COMMENT;
                next(lex);
                skip_comment(lex);
            } else if ( lex->at[0] == '%' ) {
                lex->token.kind = T_CODE_BEGIN;
                next(lex);
            }
        } break;

        case '}': {
            lex->token.kind = T_RBRACE;
            next(lex);

            if ( lex->at[0] == '}' ) {
                lex->token.kind = T_VAR_END;
                next(lex);
            }
        } break;

        case '"': {
            lex->token.kind = T_STR;
            next(lex);

            while (lex->at[0] != '"') {
                next(lex);
            }

            lex->token.str_value = intern_str(start+1, lex->input);
            next(lex);
        } break;

        case '#': {
            lex->token.kind = T_HASH;
            next(lex);
        } break;

        case '%': {
            lex->token.kind = T_PERCENT;
            next(lex);

            if ( lex->at[0] == '}' ) {
                lex->token.kind = T_CODE_END;
                next(lex);
            }
        } break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            lex->token.kind = T_INT;

            int value = 0;
            while (isdigit(lex->at[0])) {
                int digit = lex->at[0] - '0';
                value *= 10;
                value += digit;

                next(lex);
            }

            lex->token.int_value = value;
        } break;

        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z': case '_': {
            lex->token.kind = T_NAME;

            while ( isalnum(lex->at[0]) || lex->at[0] == '_' ) {
                next(lex);
            }

            lex->token.name = intern_str(start, lex->input);
        } break;

        case '|': {
            lex->token.kind = T_BAR;
            next(lex);
        } break;

        default: {
            lex->token.kind = lex->at[0];
            next(lex);
        } break;
    }

    lex->token.literal = intern_str(start, lex->input);
}

internal_proc void
next_token(Lexer *lex) {
    next_raw_token(lex);
    while ( is_raw(lex) ) {
        next_raw_token(lex);
    }
}

