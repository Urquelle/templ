struct Parser {
    Lexer lex;
};

internal_proc b32
parser_valid(Parser *p) {
    b32 result = p->lex.at[0] != 0;

    return result;
}

internal_proc char *
parser_input(Parser *p) {
    char *result = p->lex.input;

    return result;
}

global_var char ** keywords;
global_var char *keyword_and;
global_var char *keyword_or;
global_var char *keyword_true;
global_var char *keyword_false;
global_var char *keyword_none;
global_var char *keyword_if;
global_var char *keyword_elif;
global_var char *keyword_else;
global_var char *keyword_for;
global_var char *keyword_from;
global_var char *keyword_in;
global_var char *keyword_is;
global_var char *keyword_endfor;
global_var char *keyword_endif;
global_var char *keyword_endblock;
global_var char *keyword_endfilter;
global_var char *keyword_endmacro;
global_var char *keyword_endraw;
global_var char *keyword_endwith;
global_var char *keyword_extends;
global_var char *keyword_block;
global_var char *keyword_embed;
global_var char *keyword_set;
global_var char *keyword_filter;
global_var char *keyword_include;
global_var char *keyword_macro;
global_var char *keyword_import;
global_var char *keyword_raw;
global_var char *keyword_not;
global_var char *keyword_with;
global_var char *keyword_break;
global_var char *keyword_continue;
global_var char *keyword_recursive;

internal_proc void
init_keywords() {
#define ADD_KEYWORD(K) keyword_##K = intern_str(#K); buf_push(keywords, keyword_##K)

    ADD_KEYWORD(and);
    ADD_KEYWORD(or);
    ADD_KEYWORD(true);
    ADD_KEYWORD(false);
    ADD_KEYWORD(none);
    ADD_KEYWORD(if);
    ADD_KEYWORD(elif);
    ADD_KEYWORD(else);
    ADD_KEYWORD(for);
    ADD_KEYWORD(from);
    ADD_KEYWORD(in);
    ADD_KEYWORD(is);
    ADD_KEYWORD(endfor);
    ADD_KEYWORD(endif);
    ADD_KEYWORD(endblock);
    ADD_KEYWORD(endfilter);
    ADD_KEYWORD(endmacro);
    ADD_KEYWORD(endraw);
    ADD_KEYWORD(endwith);
    ADD_KEYWORD(extends);
    ADD_KEYWORD(block);
    ADD_KEYWORD(embed);
    ADD_KEYWORD(set);
    ADD_KEYWORD(filter);
    ADD_KEYWORD(include);
    ADD_KEYWORD(macro);
    ADD_KEYWORD(import);
    ADD_KEYWORD(raw);
    ADD_KEYWORD(not);
    ADD_KEYWORD(with);
    ADD_KEYWORD(break);
    ADD_KEYWORD(continue);
    ADD_KEYWORD(recursive);

#undef ADD_KEYWORD
}

internal_proc void
parser_init(Parser *p, char *input, char *name) {
    p->lex.input = input;
    p->lex.pos.name = name;
    p->lex.pos.row = 1;
    p->lex.token.pos = p->lex.pos;

    p->lex.ignore_whitespace = false;
    p->lex.trim_blocks       = true;
    p->lex.lstrip_blocks     = false;

    refill(&p->lex);
    init_keywords();
    next_raw_token(&p->lex);
}

internal_proc b32
is_token(Parser *p, Token_Kind kind) {
    b32 result = p->lex.token.kind == kind;

    return result;
}

internal_proc b32
is_prev_token(Parser *p, Token_Kind kind) {
    b32 result = p->lex.token_prev.kind == kind;

    return result;
}

internal_proc char *
token_literal(Parser *p) {
    char *result = p->lex.token.literal;

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
        fatal(p->lex.pos.name, p->lex.pos.row, "unerwartetes token. erwartet %s, erhalten %s", tokenkind_to_str(kind), tokenkind_to_str(p->lex.token.kind));
    }
}

internal_proc b32
is_keyword(Parser *p, char *expected_keyword) {
    if ( is_token(p, T_NAME) && p->lex.token.name == expected_keyword ) {
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

internal_proc b32
match_str(Parser *p, char *str) {
    if ( is_token(p, T_NAME) && p->lex.token.literal == intern_str(str) ) {
        next_token(&p->lex);

        return true;
    }

    return false;
}

internal_proc b32
is_str(Parser *p, char *str) {
    if ( p->lex.token.literal == intern_str(str) ) {
        return true;
    }

    return false;
}

internal_proc void
expect_str(Parser *p, char *str) {
    if ( !match_str(p, str) ) {
        fatal(p->lex.pos.name, p->lex.pos.row, "zeichenkette \"%s\" erwartet, stattdessen \"%s\" gefunden", str, p->lex.token.literal);
    }
}

internal_proc void
expect_keyword(Parser *p, char *expected_keyword) {
    if ( !match_keyword(p, expected_keyword) ) {
        fatal(p->lex.pos.name, p->lex.pos.row, "schlüsselwort \"%s\" erwartet, stattdessen \"%s\" gefunden", expected_keyword, p->lex.token.literal);
    }
}

internal_proc Expr *
parse_expr_base(Parser *p) {
    Lexer *lex = &p->lex;
    Expr *result = &expr_illegal;
    Pos pos = lex->pos;

    if ( is_token(p, T_INT) ) {
        result = expr_int(pos, lex->token.int_value);
        next_token(lex);
    } else if ( is_token(p, T_FLOAT) ) {
        result = expr_float(pos, lex->token.float_value);
        next_token(lex);
    } else if ( is_token(p, T_STR) ) {
        result = expr_str(pos, lex->token.str_value);
        next_token(lex);
    } else if ( is_token(p, T_NAME) ) {
        if ( match_keyword(p, keyword_true) ) {
            result = expr_bool(pos, true);
        } else if ( match_keyword(p, keyword_false) ) {
            result = expr_bool(pos, false);
        } else if ( match_keyword(p, keyword_none) ) {
            result = expr_none(pos);
        } else {
            result = expr_name(pos, lex->token.name);
            next_token(lex);
        }
    } else if ( match_token(p, T_LBRACE) ) {
        Pair **pairs = 0;

        do {
            char *key = parse_str(p);
            expect_token(p, T_COLON);
            Expr *value = parse_expr(p);

            buf_push(pairs, pair_new(key, value));
        } while ( match_token(p, T_COMMA) );

        expect_token(p, T_RBRACE);

        result = expr_dict(pos, pairs, buf_len(pairs));

    } else if ( match_token(p, T_LPAREN) ) {
        Expr **exprs = 0;
        buf_push(exprs, parse_expr(p));

        while ( match_token(p, T_COMMA) ) {
            /* @INFO: aus der jinja2 doku geht folgendes hervor:
             *        "If a tuple only has one item, it must be followed by a comma (('1-tuple',))"
             */
            if ( is_token(p, T_RPAREN) ) {
                break;
            }

            buf_push(exprs, parse_expr(p));
        }

        expect_token(p, T_RPAREN);

        size_t num_exprs = buf_len(exprs);
        if ( num_exprs > 1 ) {
            result = expr_tuple(pos, exprs, num_exprs);
        } else {
            result = expr_paren(pos, (num_exprs) ? exprs[0] : 0);
        }

        buf_free(exprs);
    }

    return result;
}

internal_proc Expr *
parse_expr_list(Parser *p) {
    Pos pos = p->lex.pos;

    if ( match_token(p, T_LBRACKET) ) {
        Expr **expr = 0;

        if ( match_token(p, T_RBRACKET) ) {
            return expr_list(pos, expr, buf_len(expr));
        }

        do {
            buf_push(expr, parse_expr(p));
        } while ( match_token(p, T_COMMA) );

        expect_token(p, T_RBRACKET);

        Expr *result = expr_list(pos, expr, buf_len(expr));
        buf_free(expr);

        return result;
    } else {
        return parse_expr_base(p);
    }
}

internal_proc Expr *
parse_expr_field_or_call_or_subscript(Parser *p) {
    Expr *left = parse_expr_list(p);
    Pos pos = p->lex.pos;

    while ( is_token(p, T_LPAREN) || is_token(p, T_LBRACKET) ||
            is_token(p, T_DOT) )
    {
        if ( match_token(p, T_LPAREN) ) {
            Arg **args = 0;

            if ( !is_token(p, T_RPAREN) ) {
                do {
                    char *name = 0;
                    Expr *expr = parse_expr(p);

                    if ( match_token(p, T_ASSIGN) ) {
                        assert(expr->kind == EXPR_NAME);
                        name = expr->expr_name.value;
                        expr = parse_expr(p);
                    }

                    buf_push(args, arg_new(expr->pos, name, expr));
                } while ( match_token(p, T_COMMA) );
            }

            expect_token(p, T_RPAREN);
            left = expr_call(pos, left, args, buf_len(args));
        } else if ( is_token(p, T_LBRACKET ) ) {
            if ( is_prev_token(p, T_SPACE) ) {
                break;
            }

            match_token(p, T_LBRACKET);

            left = expr_subscript(pos, left, parse_expr(p));
            expect_token(p, T_RBRACKET);
        } else if ( match_token(p, T_DOT) ) {
            char *field = p->lex.token.name;
            expect_token(p, T_NAME);
            left = expr_field(pos, left, field);
        }
    }

    return left;
}

internal_proc Expr *
parse_expr_unary(Parser *p) {
    if ( is_unary(p->lex.token.kind) ) {
        Token op = eat_token(&p->lex);
        return expr_unary(p->lex.pos, op.kind, parse_expr_unary(p));
    } else {
        return parse_expr_field_or_call_or_subscript(p);
    }
}

internal_proc Expr *
parse_expr_exponent(Parser *p) {
    Expr *left = parse_expr_unary(p);

    while ( match_token(p, T_POT) ) {
        left = expr_binary(p->lex.pos, T_POT, left, parse_expr_unary(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_mul(Parser *p) {
    Expr *left = parse_expr_exponent(p);

    while ( is_token(p, T_MUL) || is_token(p, T_DIV) || is_token(p, T_DIV_TRUNC) ||
            is_token(p, T_PERCENT) )
    {
        Token op = eat_token(&p->lex);
        left = expr_binary(p->lex.pos, op.kind, left, parse_expr_exponent(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_add(Parser *p) {
    Expr *left = parse_expr_mul(p);

    while ( is_token(p, T_PLUS) || is_token(p, T_MINUS) ) {
        Token op = eat_token(&p->lex);
        left = expr_binary(p->lex.pos, op.kind, left, parse_expr_mul(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_range(Parser *p) {
    Expr *left = parse_expr_add(p);

    if ( match_token(p, T_RANGE) ) {
        left = expr_range(p->lex.pos, left, parse_expr_add(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_is(Parser *p) {
    Expr *left = parse_expr_range(p);

    if ( match_keyword(p, keyword_is) ) {
        b32 not = false;
        if ( match_keyword(p, keyword_not) ) {
            not = true;
        }

        Expr *tester = parse_tester(p);
        left = expr_is(p->lex.pos, left, tester);

        if ( not ) {
            left = expr_not(p->lex.pos, left);
        }
    }

    return left;
}

internal_proc Expr *
parse_expr_not(Parser *p) {
    Expr *left = 0;

    if ( match_keyword(p, keyword_not) ) {
        left = expr_not(p->lex.pos, parse_expr(p));
    } else {
        left = parse_expr_is(p);
    }

    return left;
}

internal_proc Expr *
parse_expr_cmp(Parser *p) {
    Expr *left = parse_expr_not(p);

    while ( is_cmp(p->lex.token.kind) ) {
        Token op = eat_token(&p->lex);
        left = expr_binary(p->lex.pos, op.kind, left, parse_expr_not(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_and(Parser *p) {
    Expr *left = parse_expr_cmp(p);

    while ( match_keyword(p, keyword_and) ) {
        left = expr_binary(p->lex.pos, T_AND, left, parse_expr(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_or(Parser *p) {
    Expr *left = parse_expr_and(p);

    while ( match_keyword(p, keyword_or) ) {
        left = expr_binary(p->lex.pos, T_OR, left, parse_expr(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_tilde(Parser *p) {
    Expr *left = parse_expr_or(p);

    while ( match_token(p, T_TILDE) ) {
        left = expr_binary(p->lex.pos, T_TILDE, left, parse_expr(p));
    }

    return left;
}

internal_proc Expr **
parse_filter(Parser *p) {
    Expr **result = 0;

    do {
        Expr *call = parse_expr(p, false);

        Arg **args = 0;
        while ( !is_token(p, T_BAR) && !is_token(p, T_CODE_END) && !is_token(p, T_VAR_END) && !is_token(p, T_COMMA) ) {
            Expr *expr = parse_expr(p, false);
            buf_push(args, arg_new(expr->pos, 0, expr));
        }

        size_t num_args = buf_len(args);

        if ( call->kind != EXPR_CALL ) {
            call = expr_call(p->lex.pos, call, args, num_args);
        }

        assert(call->kind == EXPR_CALL);
        assert(call->expr_call.expr->kind == EXPR_NAME);

        buf_push(result, call);
    } while ( match_token(p, T_BAR) );

    return result;
}

internal_proc Expr *
parse_tester(Parser *p) {
    char *name = token_literal(p);
    match_token(p, T_NAME);
    Expr *result = expr_name(p->lex.pos, name);

    Arg **args = 0;
    while ( !is_keyword(p, keyword_else) && !is_keyword(p, keyword_and) &&
            !is_keyword(p, keyword_or) && !is_token(p, T_CODE_END) )
    {
        Expr *expr = parse_expr(p, false);
        buf_push(args, arg_new(expr->pos, 0, expr));
    }

    size_t num_args = buf_len(args);

    if ( result->kind != EXPR_CALL ) {
        result = expr_call(p->lex.pos, result, args, num_args);
    }

    assert(result->kind == EXPR_CALL);
    assert(result->expr_call.expr->kind == EXPR_NAME);

    return result;
}

internal_proc Expr *
parse_expr(Parser *p, b32 do_parse_filter) {
    Expr *expr = parse_expr_tilde(p);

    Expr *if_expr = 0;
    if ( match_keyword(p, keyword_if) ) {
        Expr *cond = parse_expr(p);
        Expr *else_expr = 0;

        if ( match_keyword(p, keyword_else) ) {
            else_expr = parse_expr(p);
        }

        if_expr = expr_if(p->lex.pos, cond, else_expr);
    }

    expr->if_expr = if_expr;

    Expr **filters = 0;
    if ( do_parse_filter ) {
        if ( match_token(p, T_BAR) ) {
            filters = parse_filter(p);
        }
    }

    expr->filters = filters;
    expr->num_filters = buf_len(filters);

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
    Expr **vars = 0;
    do {
        buf_push(vars, parse_expr(p));
    } while ( match_token(p, T_COMMA) );

    expect_str(p, "in");

    Expr *set = parse_expr(p);

    b32 recursive = false;
    if ( match_keyword(p, keyword_recursive) ) {
        recursive = true;
    }

    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    Stmt **else_stmts = 0;
    Stmt ***stmts_ptr = &stmts;

    while ( status_is_not_error() ) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            /* @AUFGABE: darauf prüfen ob hier versehentlich
             *           ein falsches END statement verwendet wurde
             */
            if ( stmt->kind == STMT_ENDFOR ) {
                break;
            } else if ( stmt->kind == STMT_ELSE ) {
                stmts_ptr = &else_stmts;
            } else {
                buf_push(*stmts_ptr, stmt);
            }
        } else {
            Stmt *stmt = parse_stmt(p);
            if ( stmt ) {
                buf_push(*stmts_ptr, stmt);
            }
        }
    }

    Stmt *result = stmt_for(vars, buf_len(vars), set, stmts,
            buf_len(stmts), else_stmts, buf_len(else_stmts), recursive);
    buf_free(vars);
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_if(Parser *p) {
    Expr *cond = parse_expr(p);

    expect_token(p, T_CODE_END);

    Stmt *if_stmt = stmt_if(cond, 0, 0);
    Stmt *curr_stmt = if_stmt;

    while ( status_is_not_error() ) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_ENDIF ) {
                break;
            } else if ( stmt->kind == STMT_ELSE ) {
                curr_stmt->stmt_if.else_stmt = stmt;
                curr_stmt = stmt;
            } else {
                buf_push(curr_stmt->stmt_if.stmts, stmt);
                curr_stmt->stmt_if.num_stmts = buf_len(curr_stmt->stmt_if.stmts);
            }
        } else {
            Stmt *stmt = parse_stmt(p);

            if ( stmt ) {
                buf_push(curr_stmt->stmt_if.stmts, stmt);
                curr_stmt->stmt_if.num_stmts = buf_len(curr_stmt->stmt_if.stmts);
            }
        }
    }

    return if_stmt;
}

internal_proc Stmt *
parse_stmt_elseif(Parser *p) {
    Expr *cond = parse_expr(p);
    expect_token(p, T_CODE_END);

    return stmt_else(cond, 0, 0);
}

internal_proc Stmt *
parse_stmt_else(Parser *p) {
    expect_token(p, T_CODE_END);

    return stmt_else(expr_bool(p->lex.pos, true), 0, 0);
}

internal_proc Stmt *
parse_stmt_block(Parser *p) {
    char *name = parse_name(p);
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    while ( status_is_not_error() ) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_ENDBLOCK ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            Stmt *stmt = parse_stmt(p);

            if ( stmt ) {
                buf_push(stmts, stmt);
            }
        }
    }

    Stmt *result = stmt_block(name, stmts, buf_len(stmts));
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_extends(Parser *p) {
    Expr *name = parse_expr(p);
    assert(name->kind == EXPR_STR);

    expect_token(p, T_CODE_END);

    /* @AUFGABE: im thread verarbeiten */
    Parsed_Templ *templ = parse_file(name->expr_str.value);
    Parsed_Templ *else_templ = 0;
    if ( name->if_expr && name->if_expr->expr_if.else_expr ) {
        Expr *else_expr = name->if_expr->expr_if.else_expr;
        assert(else_expr->kind == EXPR_STR);

        /* @AUFGABE: überprüfen ob endlosschleife bei imports besteht */
        /* @AUFGABE: im thread verarbeiten */
        else_templ = parse_file(else_expr->expr_str.value);
    }

    return stmt_extends(name, templ, else_templ);
}

internal_proc Stmt *
parse_stmt_include(Parser *p) {
    Expr *expr = parse_expr(p);

    b32 ignore_missing = false;
    if ( match_str(p, "ignore") && match_str(p, "missing") ) {
        ignore_missing = true;
    }

    b32 with_context = true;
    if ( match_str(p, "with") && match_str(p, "context") ) {
        /* tue nix */
    }

    if ( match_str(p, "without") && match_str(p, "context") ) {
        with_context = false;
    }

    expect_token(p, T_CODE_END);

    Parsed_Templ **templ = 0;
    if ( expr->kind == EXPR_LIST ) {
        for ( int i = 0; i < expr->expr_list.num_expr; ++i ) {
            Expr *name_expr = expr->expr_list.expr[i];
            assert(name_expr->kind == EXPR_STR);

            b32 success = os_file_exists(name_expr->expr_str.value);
            if ( !success && !ignore_missing ) {
                fatal(p->lex.pos.name, p->lex.pos.row, "konnte datei %s nicht finden", name_expr->expr_str.value);
            }

            /* @AUFGABE: überprüfen ob eine endlosschleife in includes besteht */
            if ( success ) {
                /* @AUFGABE: im thread verarbeiten */
                buf_push(templ, parse_file(name_expr->expr_str.value));
            }
        }
    } else {
        b32 success = os_file_exists(expr->expr_str.value);
        if ( !success && !ignore_missing ) {
            fatal(p->lex.pos.name, p->lex.pos.row, "konnte datei %s nicht finden", expr->expr_str.value);
        }

        if ( success ) {
            /* @AUFGABE: im thread verarbeiten */
            buf_push(templ, parse_file(expr->expr_str.value));
        }
    }

    return stmt_include(templ, buf_len(templ), ignore_missing, with_context);
}

internal_proc Stmt *
parse_stmt_filter(Parser *p) {
    Expr **filters = parse_filter(p);
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    while ( status_is_not_error() ) {
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

    Stmt *result = stmt_filter(filters, buf_len(filters), stmts, buf_len(stmts));
    buf_free(filters);
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_set(Parser *p) {
    Expr **names = 0;
    do {
        Expr *name = parse_expr(p);
        buf_push(names, name);
    } while ( match_token(p, T_COMMA) );

    expect_token(p, T_ASSIGN);
    Expr *expr = parse_expr(p);
    expect_token(p, T_CODE_END);

    return stmt_set(names, buf_len(names), expr);
}

internal_proc Stmt *
parse_stmt_macro(Parser *p) {
    expect_token(p, T_NAME);
    char *name = p->lex.token.name;
    expect_token(p, T_LPAREN);

    Param **params = 0;
    if ( !match_token(p, T_RPAREN) ) {
        char *param_name = parse_name(p);
        Expr *default_value = 0;
        if ( match_token(p, T_ASSIGN) ) {
            default_value = parse_expr(p);
        }
        buf_push(params, param_new(param_name, default_value));

        while ( match_token(p, T_COMMA) ) {
            param_name = parse_name(p);
            default_value = 0;
            if ( match_token(p, T_ASSIGN) ) {
                default_value = parse_expr(p);
            }
            buf_push(params, param_new(param_name, default_value));
        }

        expect_token(p, T_RPAREN);
    }

    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    Stmt *stmt = parse_stmt(p);

    while ( stmt->kind != STMT_ENDMACRO ) {
        buf_push(stmts, stmt);
        stmt = parse_stmt(p);
    }

    return stmt_macro(name, params, buf_len(params), stmts, buf_len(stmts));
}

internal_proc Stmt *
parse_stmt_import(Parser *p) {
    char *filename = parse_str(p);
    expect_str(p, "as");
    char *name = parse_name(p);
    expect_token(p, T_CODE_END);

    /* @AUFGABE: überprüfen ob endlosschleife bei imports besteht */
    /* @AUFGABE: im thread verarbeiten */
    return stmt_import(parse_file(filename), name);

}

internal_proc Stmt *
parse_stmt_from_import(Parser *p) {
    char *filename = parse_str(p);
    expect_keyword(p, keyword_import);

    Imported_Sym **syms = 0;
    do {
        char *name = parse_name(p);
        char *alias = 0;

        if ( match_str(p, "as") ) {
            alias = parse_name(p);
        }

        buf_push(syms, imported_sym(name, alias));
    } while ( match_token(p, T_COMMA) );

    expect_token(p, T_CODE_END);

    /* @AUFGABE: überprüfen ob endlosschleife bei imports besteht */
    /* @AUFGABE: im thread verarbeiten */
    return stmt_from_import(parse_file(filename), syms, buf_len(syms));
}

internal_proc Stmt *
parse_stmt_raw(Parser *p) {
    char *start = parser_input(p);
    char *end = start;

    expect_token(p, T_CODE_END);

    while ( status_is_not_error() ) {
        while ( !match_token(p, T_CODE_BEGIN) ) {
            end = parser_input(p);
            next_token(&p->lex);
        }

        if ( match_keyword(p, keyword_endraw) ) {
            expect_token(p, T_CODE_END);
            break;
        }
    }

    return stmt_raw(intern_str(start, end));
}

internal_proc Stmt *
parse_stmt_with(Parser *p) {
    Arg **args = 0;
    if ( !is_token(p, T_CODE_END) ) {
        do {
            char *name = parse_name(p);
            expect_token(p, T_ASSIGN);
            Expr *expr = parse_expr(p);

            buf_push(args, arg_new(expr->pos, name, expr));
        } while ( match_token(p, T_COMMA) );
    }

    expect_token(p, T_CODE_END);
    Stmt **stmts = 0;
    Stmt *stmt = parse_stmt(p);

    while ( stmt && stmt->kind != STMT_ENDWITH ) {
        buf_push(stmts, stmt);
        stmt = parse_stmt(p);
    }

    return stmt_with(args, buf_len(args), stmts, buf_len(stmts));
}

internal_proc Stmt *
parse_stmt_break(Parser *p) {
    expect_token(p, T_CODE_END);
    return &stmt_break;
}

internal_proc Stmt *
parse_stmt_continue(Parser *p) {
    expect_token(p, T_CODE_END);
    return &stmt_continue;
}

internal_proc Stmt *
parse_stmt_endfor(Parser *p) {
    expect_token(p, T_CODE_END);
    return &stmt_endfor;
}

internal_proc Stmt *
parse_stmt_endif(Parser *p) {
    expect_token(p, T_CODE_END);
    return &stmt_endif;
}

internal_proc Stmt *
parse_stmt_endblock(Parser *p) {
    match_token(p, T_NAME);
    expect_token(p, T_CODE_END);
    return &stmt_endblock;
}

internal_proc Stmt *
parse_stmt_endfilter(Parser *p) {
    expect_token(p, T_CODE_END);
    return &stmt_endfilter;
}

internal_proc Stmt *
parse_stmt_endmacro(Parser *p) {
    expect_token(p, T_CODE_END);
    return &stmt_endmacro;
}

internal_proc Stmt *
parse_stmt_endwith(Parser *p) {
    expect_token(p, T_CODE_END);
    return &stmt_endwith;
}

internal_proc Stmt *
parse_stmt(Parser *p) {
    Pos pos = p->lex.pos;

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
        } else if ( match_keyword(p, keyword_endmacro) ) {
            result = parse_stmt_endmacro(p);
        } else if ( match_keyword(p, keyword_endwith) ) {
            result = parse_stmt_endwith(p);
        } else if ( match_keyword(p, keyword_else) ) {
            result = parse_stmt_else(p);
        } else if ( match_keyword(p, keyword_elif) ) {
            result = parse_stmt_elseif(p);
        } else if ( match_keyword(p, keyword_block) ) {
            result = parse_stmt_block(p);
        } else if ( match_keyword(p, keyword_extends) ) {
            result = parse_stmt_extends(p);
        } else if ( match_keyword(p, keyword_filter) ) {
            result = parse_stmt_filter(p);
        } else if ( match_keyword(p, keyword_set) ) {
            result = parse_stmt_set(p);
        } else if ( match_keyword(p, keyword_include) ) {
            result = parse_stmt_include(p);
        } else if ( match_keyword(p, keyword_macro) ) {
            result = parse_stmt_macro(p);
        } else if ( match_keyword(p, keyword_import) ) {
            result = parse_stmt_import(p);
        } else if ( match_keyword(p, keyword_from) ) {
            result = parse_stmt_from_import(p);
        } else if ( match_keyword(p, keyword_raw) ) {
            result = parse_stmt_raw(p);
        } else if ( match_keyword(p, keyword_with) ) {
            result = parse_stmt_with(p);
        } else if ( match_keyword(p, keyword_break) ) {
            result = parse_stmt_break(p);
        } else if ( match_keyword(p, keyword_continue) ) {
            result = parse_stmt_continue(p);
        } else {
            result = &stmt_illegal;
            fatal(p->lex.pos.name, p->lex.pos.row, "unbekanntes token aufgetreten: %s", tokenkind_to_str(p->lex.token.kind));
        }
    } else if ( match_token(p, T_VAR_BEGIN) ) {
        result = parse_stmt_var(p);
    } else {
        result = parse_stmt_lit(p);
    }

    if ( result ) {
        result->pos = pos;
    }

    return result;
}

internal_proc Stmt *
parse_stmt_var(Parser *p) {
    Expr *expr = parse_expr(p);

    expect_token(p, T_VAR_END);

    return stmt_var(expr);
}

internal_proc Stmt *
parse_code(Parser *p) {
    return parse_stmt(p);
}

internal_proc Stmt *
parse_stmt_lit(Parser *p) {
    char *lit = 0;

    if ( !is_token(p, T_COMMENT) && (
            !p->lex.trim_blocks ||
            !is_prev_token(p, T_CODE_END) ||
            p->lex.token.literal[0] != '\n')
    ) {
        buf_printf(lit, "%s", p->lex.token.literal);
    }

    while ( parser_valid(p) && is_lit(&p->lex) ) {
        char *ptr = parser_input(p);
        next(&p->lex);

#if 1
        buf_printf(lit, "%.*s", utf8_char_size(ptr), ptr);
#else
        if ( is_lit(&p->lex) && !p->lex.lstrip_blocks ) {
            buf_printf(lit, "%.*s", utf8_char_size(ptr), ptr);
        }
#endif
    }

    Stmt *result = ( lit ) ? stmt_lit(lit, utf8_str_size(lit)) : 0;
    next_token(&p->lex);

    return result;
}

internal_proc Parsed_Templ *
parse_string(char *content, char *sourcename = "<string>") {
    Parsed_Templ *templ = parsed_templ(sourcename);

    if ( !content ) return templ;

    Parser parser = {};
    Parser *p = &parser;
    parser_init(p, content, sourcename);

    while ( !match_token(p, T_EOF) ) {
        if ( is_token(p, T_VAR_BEGIN) || is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_code(p);

            if ( stmt ) {
                buf_push(templ->stmts, stmt);
            }
        } else if ( is_token(p, T_COMMENT) ) {
            next_token(&p->lex);
        } else {
            Stmt *stmt = parse_stmt_lit(p);

            if ( stmt ) {
                buf_push(templ->stmts, stmt);
            }
        }
    }

    templ->num_stmts = buf_len(templ->stmts);
    return templ;
}

internal_proc Parsed_Templ *
parse_file(char *filename) {
    Parsed_Templ *templ = 0;

    char *content = 0;
    if ( os_file_read(filename, &content) ) {
        templ = parse_string(content, filename);
    } else {
        fatal(0, 0, "konnte datei %s nicht lesen", filename);
    }

    return templ;
}

