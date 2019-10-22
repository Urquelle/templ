struct Pos {
    char *name;
    s64 row;
    char *start;
};

enum Token_Kind {
    T_EOF,
    T_INT = 129,
    T_FLOAT,
    T_STR,
    T_NAME,
    T_LIT,
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
    T_MUL,
    T_DIV,
    T_DIV_TRUNC,

    T_BANG,
    T_UNARY_FIRST = T_BANG,
    T_MINUS,
    T_PLUS,
    T_UNARY_LAST = T_PLUS,

    T_SPACE,
    T_TAB,
    T_NEWLINE,
    T_COMMENT,

    T_LT,
    T_CMP_FIRST = T_LT,
    T_LEQ,
    T_EQL,
    T_NEQ,
    T_GEQ,
    T_GT,
    T_CMP_LAST = T_GT,

    T_AND,
    T_OR,

    T_VAR_BEGIN,
    T_VAR_END,
    T_CODE_BEGIN,
    T_CODE_END,
};

struct Token {
    Pos pos;
    Token_Kind kind;
    char *literal;

    union {
        char *str_value;
        char *name;
        int   int_value;
        float float_value;
        int   char_value;
    };
};

struct Lexer {
    Token token;
    Pos pos;
    char *input;
    char at[2];
};

internal_proc char
at0(Lexer *lex) {
    return lex->at[0];
}

internal_proc char
at1(Lexer *lex) {
    return lex->at[1];
}

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
        if (at0(lex) == '\n') {
            lex->pos.row++;
        }

        if (lex->input == 0) break;
        lex->input++;
    }
    refill(lex);
}

internal_proc void
skip_comment(Lexer *lex) {
    while ( lex->input ) {
        if ( at0(lex) == '#' && at1(lex) == '}' ) {
            next(lex, 2);
            return;
        }

        if ( at0(lex) == '{' && at1(lex) == '#' ) {
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

    if ( at0(lex) == '{' ) {
        if ( at1(lex) == '{' || at1(lex) == '#' || at1(lex) == '%' ) {
            result = false;
        }
    }

    return result;
}

internal_proc b32
is_cmp(Token_Kind kind) {
    b32 result = T_CMP_FIRST <= kind && kind <= T_CMP_LAST;

    return result;
}

internal_proc b32
is_unary(Token_Kind kind) {
    b32 result = T_UNARY_FIRST <= kind && kind <= T_UNARY_LAST;

    return result;
}

internal_proc b32
is_eql(Token_Kind kind) {
    b32 result = (kind == T_EQL || kind == T_NEQ);

    return result;
}

internal_proc b32
is_alpha(char c) {
    return ( c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' );
}

internal_proc b32
is_numeric(char c) {
    return ( c >= '0' && c <= '9' );
}

internal_proc int
scan_int(Lexer *lex) {
    int value = 0;
    while (isdigit(at0(lex))) {
        int digit = at0(lex) - '0';
        value *= 10;
        value += digit;

        next(lex);
    }

    return value;
}

internal_proc float
scan_float(Lexer *lex) {
    char *start = lex->input;

    if ( at0(lex) == '.' ) {
        next(lex);
        while ( isdigit(at0(lex)) ) {
            next(lex);
        }
    }

    return strtof(start, NULL);
}

internal_proc void
next_raw_token(Lexer *lex) {
    char *start = lex->input;
    lex->token.pos = lex->pos;
    lex->token.pos.start = start;

    char c = at0(lex);
    if ( c == 0 ) {
        lex->token.kind = T_EOF;
        next(lex);
    } else if ( c == ',' ) {
        lex->token.kind = T_COMMA;
        next(lex);
    } else if ( c == '.' ) {
        lex->token.kind = T_DOT;
        next(lex);

        if ( at0(lex) == '.' ) {
            lex->token.kind = T_RANGE;
            next(lex);
        }
    } else if ( c == '(' ) {
        lex->token.kind = T_LPAREN;
        next(lex);
    } else if ( c == ')' ) {
        lex->token.kind = T_RPAREN;
        next(lex);
    } else if ( c == '[' ) {
        lex->token.kind = T_LBRACKET;
        next(lex);
    } else if ( c == ']' ) {
        lex->token.kind = T_RBRACKET;
        next(lex);
    } else if ( c == '!' ) {
        lex->token.kind = T_BANG;
        next(lex);

        if ( at0(lex) == '=' ) {
            lex->token.kind = T_NEQ;
            next(lex);
        }
    } else if ( c == '=' ) {
        lex->token.kind = T_ASSIGN;
        next(lex);

        if ( at0(lex) == '=' ) {
            lex->token.kind = T_EQL;
            next(lex);
        }
    } else if ( c == '<' ) {
        lex->token.kind = T_LT;
        next(lex);

        if ( at0(lex) == '=' ) {
            lex->token.kind = T_LEQ;
            next(lex);
        }
    } else if ( c == '>' ) {
        lex->token.kind = T_GT;
        next(lex);

        if ( at0(lex) == '=' ) {
            lex->token.kind = T_GEQ;
            next(lex);
        }
    } else if ( c == '+' ) {
        lex->token.kind = T_PLUS;
        next(lex);
    } else if ( c == '-' ) {
        lex->token.kind = T_MINUS;
        next(lex);
    } else if ( c == '*' ) {
        lex->token.kind = T_MUL;
        next(lex);
    } else if ( c == '/' ) {
        lex->token.kind = T_DIV;
        next(lex);

        if ( at0(lex) == '/' ) {
            lex->token.kind = T_DIV_TRUNC;
            next(lex);
        }
    } else if ( c == '?' ) {
        lex->token.kind = T_QMARK;
        next(lex);
    } else if ( c == ':' ) {
        lex->token.kind = T_COLON;
        next(lex);
    } else if ( c == ' ' ) {
        lex->token.kind = T_SPACE;
        next(lex);
    } else if ( c == '\t' ) {
        lex->token.kind = T_TAB;
        next(lex);
    } else if ( c == '\n' ) {
        lex->token.kind = T_NEWLINE;
        next(lex);
    } else if ( c == '\v' ) {
        lex->token.kind = T_SPACE;
        next(lex);
    } else if ( c == '{' ) {
        lex->token.kind = T_LBRACE;
        next(lex);

        if ( at0(lex) == '{' ) {
            lex->token.kind = T_VAR_BEGIN;
            next(lex);
        } else if ( at0(lex) == '#' ) {
            lex->token.kind = T_COMMENT;
            next(lex);
            skip_comment(lex);
        } else if ( at0(lex) == '%' ) {
            lex->token.kind = T_CODE_BEGIN;
            next(lex);
        }
    } else if ( c == '}' ) {
        lex->token.kind = T_RBRACE;
        next(lex);

        if ( at0(lex) == '}' ) {
            lex->token.kind = T_VAR_END;
            next(lex);
        }
    } else if ( c == '\'' || c == '"' ) {
        lex->token.kind = T_STR;
        next(lex);

        while (at0(lex) != '"' && at0(lex) != '\'') {
            next(lex);
        }

        lex->token.str_value = intern_str(start+1, lex->input);
        next(lex);
    } else if ( c == '#' ) {
        lex->token.kind = T_HASH;
        next(lex);
    } else if ( c == '%' ) {
        lex->token.kind = T_PERCENT;
        next(lex);

        if ( at0(lex) == '}' ) {
            lex->token.kind = T_CODE_END;
            next(lex);
        }
    } else if ( c >= '0' && c <= '9' ) {
        lex->token.kind = T_INT;
        s32 int_value = scan_int(lex);

        if ( at0(lex) == '.' && isdigit(at1(lex)) ) {
            lex->token.kind = T_FLOAT;
            lex->token.float_value = int_value + scan_float(lex);
        } else {
            lex->token.int_value = int_value;
        }
    } else if ( c == '_' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || (u8)c >= 0xC0 ) {
        lex->token.kind = T_NAME;

        while ( is_alpha(at0(lex)) || is_numeric(at0(lex)) || at0(lex) == '_' ) {
            next(lex);
        }

        lex->token.name = intern_str(start, lex->input);
    } else if ( c == '|' ) {
        lex->token.kind = T_BAR;
        next(lex);
    } else {
        lex->token.kind = (Token_Kind)at0(lex);
        next(lex);
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

internal_proc Token
eat_token(Lexer *lex) {
    Token result = lex->token;
    next_token(lex);

    return result;
}

