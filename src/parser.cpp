struct Parser {
    Lexer lex;
};

global_var Stmt **parsed_stmts;

internal_proc Expr * parse_expr(Parser *p);
internal_proc char * parse_name(Parser *p);
internal_proc Stmt * parse_stmt(Parser *p);
internal_proc Stmt * parse_var(Parser *p);
internal_proc Stmt * parse_lit(Parser *p);
internal_proc Var_Filter parse_filter(Parser *p);
internal_proc Parsed_Templ * parse_file(char *filename);

global_var char ** keywords;
global_var char *keyword_true;
global_var char *keyword_false;
global_var char *keyword_if;
global_var char *keyword_else;
global_var char *keyword_for;
global_var char *keyword_in;
global_var char *keyword_is;
global_var char *keyword_endfor;
global_var char *keyword_endif;
global_var char *keyword_endblock;
global_var char *keyword_endfilter;
global_var char *keyword_extends;
global_var char *keyword_block;
global_var char *keyword_embed;
global_var char *keyword_set;
global_var char *keyword_filter;

internal_proc void
init_keywords() {
#define ADD_KEYWORD(K) keyword_##K = intern_str(#K); buf_push(keywords, keyword_##K)

    ADD_KEYWORD(true);
    ADD_KEYWORD(false);
    ADD_KEYWORD(if);
    ADD_KEYWORD(else);
    ADD_KEYWORD(for);
    ADD_KEYWORD(in);
    ADD_KEYWORD(is);
    ADD_KEYWORD(endfor);
    ADD_KEYWORD(endif);
    ADD_KEYWORD(endblock);
    ADD_KEYWORD(endfilter);
    ADD_KEYWORD(extends);
    ADD_KEYWORD(block);
    ADD_KEYWORD(embed);
    ADD_KEYWORD(set);
    ADD_KEYWORD(filter);

#undef ADD_KEYWORD
}

internal_proc void
init_parser(Parser *p, char *input, char *name) {
    p->lex.input = input;
    p->lex.pos.name = name;
    p->lex.pos.row = 1;
    p->lex.token.pos = p->lex.pos;
    refill(&p->lex);
    init_keywords();
    next_raw_token(&p->lex);
}

internal_proc b32
is_token(Parser *p, Token_Kind kind) {
    return p->lex.token.kind == kind;
}

internal_proc b32
is_valid(Parser *p) {
    b32 result = p->lex.at[0] != 0;

    return result;
}

internal_proc b32
match_token(Parser *p, Token_Kind kind) {
    if ( is_token(p, kind) ) {
        next_token(&p->lex);

        return true;
    }

    return false;
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
    Expr *result = 0;

    if ( is_token(p, T_INT) ) {
        result = expr_int(lex->token.int_value);
        next_token(lex);
    } else if ( is_token(p, T_FLOAT) ) {
        result = expr_float(lex->token.float_value);
        next_token(lex);
    } else if ( is_token(p, T_STR) ) {
        result = expr_str(lex->token.str_value);
        next_token(lex);
    } else if ( is_token(p, T_NAME) ) {
        if ( match_keyword(p, keyword_true) ) {
            result = expr_bool(true);
        } else if ( match_keyword(p, keyword_false) ) {
            result = expr_bool(false);
        } else {
            result = expr_name(lex->token.name);
            next_token(lex);
        }
    } else if ( match_token(p, T_LPAREN) ) {
        result = expr_paren(parse_expr(p));
        expect_token(p, T_RPAREN);
    }

    return result;
}

internal_proc Expr *
parse_expr_field_or_call_or_index(Parser *p) {
    Expr *left = parse_expr_base(p);

    while ( is_token(p, T_LPAREN) || is_token(p, T_LBRACKET) || is_token(p, T_DOT) ) {
        if ( match_token(p, T_LPAREN) ) {
            Expr **exprs = 0;
            if ( !is_token(p, T_RPAREN) ) {
                buf_push(exprs, parse_expr(p));
                while ( match_token(p, T_COMMA) ) {
                    buf_push(exprs, parse_expr(p));
                }
            }
            expect_token(p, T_RPAREN);
            left = expr_call(left, exprs, buf_len(exprs));
        } else if ( match_token(p, T_LBRACKET) ) {
            Expr *index = parse_expr(p);

            if ( !index ) {
                assert(!"index darf nicht leer sein");
            }

            left = expr_index(left, index);
            expect_token(p, T_RBRACKET);
        } else if ( match_token(p, T_DOT) ) {
            char *field = p->lex.token.name;
            expect_token(p, T_NAME);
            left = expr_field(left, field);
        }
    }

    return left;
}

internal_proc Expr *
parse_expr_unary(Parser *p) {
    if ( is_unary(p->lex.token.kind) ) {
        Token op = eat_token(&p->lex);
        return expr_unary(op.kind, parse_expr_unary(p));
    } else {
        return parse_expr_field_or_call_or_index(p);
    }
}

internal_proc Expr *
parse_expr_mul(Parser *p) {
    Expr *left = parse_expr_unary(p);

    while ( is_token(p, T_MUL) || is_token(p, T_DIV) ) {
        Token op = eat_token(&p->lex);
        left = expr_binary(op.kind, left, parse_expr_unary(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_add(Parser *p) {
    Expr *left = parse_expr_mul(p);

    while ( is_token(p, T_PLUS) || is_token(p, T_MINUS) ) {
        Token op = eat_token(&p->lex);
        left = expr_binary(op.kind, left, parse_expr_mul(p));
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

    while ( is_cmp(p->lex.token.kind) ) {
        Token op = eat_token(&p->lex);
        left = expr_binary(op.kind, left, parse_expr_range(p));
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
    Pos pos = p->lex.token.pos;
    Expr *expr = parse_expr_ternary(p);
    expr->pos = pos;

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
    char *it = parse_name(p);
    expect_keyword(p, keyword_in);
    Expr *cond = parse_expr(p);
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_ENDFOR ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            buf_push(stmts, parse_stmt(p));
        }
    }

    Stmt *result = stmt_for(it, cond, stmts, buf_len(stmts));
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_if(Parser *p) {
    Expr *cond = parse_expr(p);

    if ( match_keyword(p, keyword_is) ) {
        Expr *test = parse_expr(p);
        assert(test->kind == EXPR_NAME);

        Expr **args = 0;
        while ( !is_token(p, T_CODE_END ) ) {
            buf_push(args, parse_expr(p));
            match_token(p, T_COMMA);
        }

        cond = expr_is(cond, test, args, buf_len(args));
    }

    expect_token(p, T_CODE_END);

    Stmt *if_stmt = stmt_if(cond, 0, 0);
    Stmt **stmt_elseifs = 0;
    Stmt *stmt_else = 0;
    Stmt *curr_stmt = if_stmt;

    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_ENDIF ) {
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

    if_stmt->stmt_if.elseifs = stmt_elseifs;
    if_stmt->stmt_if.num_elseifs = buf_len(stmt_elseifs);
    if_stmt->stmt_if.else_stmt = stmt_else;

    return if_stmt;
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
            if ( stmt->kind == STMT_ENDBLOCK ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            buf_push(stmts, parse_stmt(p));
        }
    }

    Stmt *result = stmt_block(name, stmts, buf_len(stmts));
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_extends(Parser *p) {
    char *name = parse_str(p);
    expect_token(p, T_CODE_END);

    Parsed_Templ *templ = parse_file(name);

    return stmt_extends(name, templ);
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
            if ( stmt->kind == STMT_ENDFILTER ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            buf_push(stmts, parse_stmt(p));
        }
    }

    Stmt *result = stmt_filter(filter, buf_len(filter), stmts, buf_len(stmts));
    buf_free(filter);
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_set(Parser *p) {
    char *name = parse_name(p);
    expect_token(p, T_ASSIGN);
    Expr *expr = parse_expr(p);
    expect_token(p, T_CODE_END);

    return stmt_set(name, expr);
}

internal_proc Stmt *
parse_stmt_endfor(Parser *p) {
    expect_token(p, T_CODE_END);
    return stmt_endfor();
}

internal_proc Stmt *
parse_stmt_endif(Parser *p) {
    expect_token(p, T_CODE_END);
    return stmt_endif();
}

internal_proc Stmt *
parse_stmt_endblock(Parser *p) {
    match_token(p, T_NAME);
    expect_token(p, T_CODE_END);
    return stmt_endblock();
}

internal_proc Stmt *
parse_stmt_endfilter(Parser *p) {
    expect_token(p, T_CODE_END);
    return stmt_endfilter();
}

internal_proc Stmt *
parse_stmt(Parser *p) {
    Stmt *result = 0;
    if ( match_token(p, T_CODE_BEGIN) ) {
        if ( match_keyword(p, keyword_for) ) {
            result = parse_stmt_for(p);
        } else if ( match_keyword(p, keyword_if) ) {
            result = parse_stmt_if(p);
        } else if ( match_keyword(p, keyword_endfor) ) {
            result = parse_stmt_endfor(p);
        } else if ( match_keyword(p, keyword_endif) ) {
            result = parse_stmt_endif(p);
        } else if ( match_keyword(p, keyword_endblock) ) {
            result = parse_stmt_endblock(p);
        } else if ( match_keyword(p, keyword_endfilter) ) {
            result = parse_stmt_endfilter(p);
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
        } else if ( match_keyword(p, keyword_set) ) {
            result = parse_stmt_set(p);
        } else {
            assert(0);
        }
    } else if ( match_token(p, T_VAR_BEGIN) ) {
        result = parse_var(p);
    } else {
        result = parse_lit(p);
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

internal_proc Stmt *
parse_var(Parser *p) {
    Expr *expr = parse_expr(p);

    Var_Filter *filter = 0;
    while ( match_token(p, T_BAR) ) {
        buf_push(filter, parse_filter(p));
    }
    expect_token(p, T_VAR_END);

    return stmt_var(expr, filter, buf_len(filter));
}

internal_proc Stmt *
parse_code(Parser *p) {
    return parse_stmt(p);
}

internal_proc Stmt *
parse_lit(Parser *p) {
    char *lit = 0;

    buf_printf(lit, "%s", p->lex.token.literal);
    while ( is_valid(p) && is_lit(&p->lex) ) {
        buf_printf(lit, "%c", p->lex.at[0]);
        next(&p->lex);
    }

    Stmt *result = stmt_lit(lit, strlen(lit));
    next_token(&p->lex);

    return result;
}

internal_proc Parsed_Templ *
parse_file(char *filename) {
    char *content = 0;
    Parsed_Templ *templ = (Parsed_Templ *)xcalloc(1, sizeof(Parsed_Templ));

    /* @TODO: nur den dateinamen ohne dateierweiterung übernehmen */
    templ->name = filename;

    if ( file_read(filename, &content) ) {
        Parser parser = {};
        Parser *p = &parser;
        init_parser(p, content, filename);

        while (p->lex.token.kind != T_EOF) {
            if ( is_token(p, T_VAR_BEGIN) || is_token(p, T_CODE_BEGIN) ) {
                Stmt *stmt = parse_code(p);

                if ( stmt->kind == STMT_EXTENDS ) {
                    templ->parent = stmt->stmt_extends.templ;
                    continue;
                }

                buf_push(templ->stmts, stmt);
            } else {
                buf_push(templ->stmts, parse_lit(p));
            }
        }
    } else {
        assert(!"konnte datei nicht lesen");
    }

    templ->num_stmts = buf_len(templ->stmts);
    return templ;
}

