struct Parser {
    Lexer lex;
};

internal_proc Expr * parse_expr(Parser *p);
internal_proc Stmt * parse_stmt(Parser *p);
internal_proc Item * parse_var(Parser *p);
internal_proc Item * parse_lit(Parser *p);
internal_proc Var_Filter parse_filter(Parser *p);

global_var char ** keywords;
global_var char *keyword_if;
global_var char *keyword_else;
global_var char *keyword_for;
global_var char *keyword_in;
global_var char *keyword_end;
global_var char *keyword_extends;
global_var char *keyword_block;
global_var char *keyword_embed;
global_var char *keyword_filter;

internal_proc void
init_keywords() {
#define ADD_KEYWORD(K) keyword_##K = intern_str(#K); buf_push(keywords, keyword_##K)

    ADD_KEYWORD(if);
    ADD_KEYWORD(else);
    ADD_KEYWORD(for);
    ADD_KEYWORD(in);
    ADD_KEYWORD(end);
    ADD_KEYWORD(extends);
    ADD_KEYWORD(block);
    ADD_KEYWORD(embed);
    ADD_KEYWORD(filter);

#undef ADD_KEYWORD
}

internal_proc void
init_parser(Parser *p, char *input) {
    p->lex.input = input;
    refill(&p->lex);
    init_keywords();
    next_token(&p->lex);
}

internal_proc b32
is_token(Parser *p, Token_Kind kind) {
    return p->lex.token.kind == kind;
}

internal_proc b32
match_token(Parser *p, Token_Kind kind) {
    if ( is_token(p, kind) ) {
        next_token(&p->lex);
        return 1;
    }

    return 0;
}

internal_proc void
expect_token(Parser *p, Token_Kind kind) {
    if ( is_token(p, kind) ) {
        next_token(&p->lex);
    } else {
        assert(0);
    }
}

internal_proc b32
is_keyword(Parser *p, char *expected_keyword) {
    if ( p->lex.token.name == expected_keyword ) {
        return true;
    }
    return false;
}

internal_proc b32
match_keyword(Parser *p, char *expected_keyword) {
    if ( is_keyword(p, expected_keyword) ) {
        next_token(&p->lex);
        return true;
    }
    return false;
}

internal_proc void
expect_keyword(Parser *p, char *expected_keyword) {
    if ( match_keyword(p, expected_keyword) ) {
    } else {
        assert(0);
    }
}

internal_proc Expr *
parse_expr_base(Parser *p) {
    Lexer *lex = &p->lex;
    if ( is_token(p, T_INT) ) {
        Expr *result = expr_int(lex->token.int_value);
        next_token(lex);
        return result;
    } else if ( is_token(p, T_STR) ) {
        Expr *result = expr_str(lex->token.str_value);
        next_token(lex);
        return result;
    } else if ( is_token(p, T_NAME) ) {
        Expr *result = expr_name(lex->token.name);
        next_token(lex);
        return result;
    } else {
        assert(0);
        return 0;
    }
}

internal_proc Expr *
parse_expr_field(Parser *p) {
    Expr *left = parse_expr_base(p);

    if ( match_token(p, T_DOT) ) {
        left = expr_field(left, parse_expr_base(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_unary(Parser *p) {
    if ( match_token(p, T_PLUS) ) {
        return expr_unary(T_PLUS, parse_expr_unary(p));
    } else {
        return parse_expr_field(p);
    }
}

internal_proc Expr *
parse_expr_mul(Parser *p) {
    Expr *left = parse_expr_unary(p);

    if ( match_token(p, T_MUL) ) {
        left = expr_binary(T_MUL, left, parse_expr_unary(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_add(Parser *p) {
    Expr *left = parse_expr_mul(p);

    if ( match_token(p, T_PLUS) ) {
        left = expr_binary(T_PLUS, left, parse_expr_mul(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_range(Parser *p) {
    Expr *left = parse_expr_add(p);

    if ( match_token(p, T_RANGE) ) {
        left = expr_range(left, parse_expr_add(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_cmp(Parser *p) {
    Expr *left = parse_expr_range(p);

    if ( match_token(p, T_EQL) ) {
        left = expr_binary(T_EQL, left, parse_expr_range(p));
    } else if ( match_token(p, T_LT) ) {
        left = expr_binary(T_LT, left, parse_expr_range(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_ternary(Parser *p) {
    Expr *left = parse_expr_cmp(p);

    if ( match_token(p, T_QMARK) ) {
        Expr *middle = parse_expr_cmp(p);
        expect_token(p, T_COLON);
        left = expr_ternary(left, middle, parse_expr_cmp(p));
    }

    return left;
}

internal_proc Expr *
parse_expr(Parser *p) {
    Expr *expr = parse_expr_ternary(p);

    return expr;
}

internal_proc char *
parse_name(Parser *p) {
    Expr *expr = parse_expr(p);
    assert(expr->kind == EXPR_NAME);

    return expr->expr_name.value;
}

internal_proc char *
parse_str(Parser *p) {
    Expr *expr = parse_expr(p);
    assert(expr->kind == EXPR_STR);

    return expr->expr_str.value;
}

internal_proc Stmt *
parse_stmt_for(Parser *p) {
    Expr *it = parse_expr(p);
    expect_keyword(p, keyword_in);
    Expr *cond = parse_expr(p);
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_END ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            buf_push(stmts, parse_stmt(p));
        }
    }

    return stmt_for(it, cond, stmts, buf_len(stmts));
}

internal_proc Stmt *
parse_stmt_if(Parser *p) {
    Expr *cond = parse_expr(p);
    expect_token(p, T_CODE_END);

    Stmt *if_stmt = stmt_if(cond, 0, 0);
    Stmt **stmt_elseifs = 0;
    Stmt *stmt_else = 0;
    Stmt *curr_stmt = if_stmt;

    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_END ) {
                break;
            } else if ( stmt->kind == STMT_ELSEIF ) {
                buf_push(stmt_elseifs, stmt);
                curr_stmt = stmt;
            } else if ( stmt->kind == STMT_ELSE ) {
                stmt_else = stmt;
                curr_stmt = stmt;
            } else {
                buf_push(curr_stmt->stmt_if.stmts, stmt);
                curr_stmt->stmt_if.num_stmts = buf_len(curr_stmt->stmt_if.stmts);
            }
        } else {
            buf_push(curr_stmt->stmt_if.stmts, parse_stmt(p));
            curr_stmt->stmt_if.num_stmts = buf_len(curr_stmt->stmt_if.stmts);
        }
    }

    if_stmt->stmt_if.elseif_stmts = stmt_elseifs;
    if_stmt->stmt_if.num_elseifs = buf_len(stmt_elseifs);
    if_stmt->stmt_if.else_stmt = stmt_else;

    return if_stmt;
}

internal_proc Stmt *
parse_stmt_end(Parser *p) {
    expect_token(p, T_CODE_END);
    return stmt_end();
}

internal_proc Stmt *
parse_stmt_elseif(Parser *p) {
    Expr *cond = parse_expr(p);
    expect_token(p, T_CODE_END);

    return stmt_elseif(cond, 0, 0);
}

internal_proc Stmt *
parse_stmt_else(Parser *p) {
    expect_token(p, T_CODE_END);

    return stmt_else(0, 0);
}

internal_proc Stmt *
parse_stmt_block(Parser *p) {
    char *name = parse_name(p);
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_END ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            buf_push(stmts, parse_stmt(p));
        }
    }

    return stmt_block(name, stmts, buf_len(stmts));
}

internal_proc Stmt *
parse_stmt_extends(Parser *p) {
    char *name = parse_str(p);
    expect_token(p, T_CODE_END);

    return stmt_extends(name);
}

internal_proc Stmt *
parse_stmt_filter(Parser *p) {
    Var_Filter *filter = 0;
    buf_push(filter, parse_filter(p));

    while ( match_token(p, T_BAR) ) {
        buf_push(filter, parse_filter(p));
    }
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_END ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            buf_push(stmts, parse_stmt(p));
        }
    }

    return stmt_filter(filter, buf_len(filter), stmts, buf_len(stmts));
}

internal_proc Stmt *
parse_stmt(Parser *p) {
    Stmt *result = 0;
    if ( match_token(p, T_CODE_BEGIN) ) {
        if ( match_keyword(p, keyword_for) ) {
            result = parse_stmt_for(p);
        } else if ( match_keyword(p, keyword_if) ) {
            result = parse_stmt_if(p);
        } else if ( match_keyword(p, keyword_end) ) {
            result = parse_stmt_end(p);
        } else if ( match_keyword(p, keyword_else) ) {
            if ( match_keyword(p, keyword_if) ) {
                result = parse_stmt_elseif(p);
            } else {
                result = parse_stmt_else(p);
            }
        } else if ( match_keyword(p, keyword_block) ) {
            result = parse_stmt_block(p);
        } else if ( match_keyword(p, keyword_extends) ) {
            result = parse_stmt_extends(p);
        } else if ( match_keyword(p, keyword_filter) ) {
            result = parse_stmt_filter(p);
        } else {
            assert(0);
        }
    } else if ( match_token(p, T_VAR_BEGIN) ) {
        result = stmt_var(parse_var(p));
    } else {
        result = stmt_lit(parse_lit(p));
    }

    return result;
}

internal_proc Var_Filter
parse_filter(Parser *p) {
    char *name = parse_name(p);
    Expr **params = 0;
    while ( !is_token(p, T_BAR) && !is_token(p, T_VAR_END) && !is_token(p, T_CODE_END) ) {
        buf_push(params, parse_expr(p));
    }

    return var_filter(name, params, buf_len(params));
}

internal_proc Item *
parse_var(Parser *p) {
    Expr *expr = parse_expr(p);

    Var_Filter *filter = 0;
    while ( match_token(p, T_BAR) ) {
        buf_push(filter, parse_filter(p));
    }
    expect_token(p, T_VAR_END);

    return item_var(expr, filter, buf_len(filter));
}

internal_proc Item *
parse_code(Parser *p) {
    return item_code(parse_stmt(p));
}

internal_proc Item *
parse_lit(Parser *p) {
    char *lit = 0;
    while ( !is_token(p, T_VAR_BEGIN) && !is_token(p, T_CODE_BEGIN) ) {
        buf_printf(lit, "%s", p->lex.token.literal);
        next_raw_token(&p->lex);
    }

    return item_lit(lit, strlen(lit));
}

