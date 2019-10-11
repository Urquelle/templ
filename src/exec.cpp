#define genf(...)   gen_result = strf("%s%s", gen_result, strf(__VA_ARGS__))
#define genlnf(...) gen_result = strf("%s\n", gen_result); gen_indentation(); genf(__VA_ARGS__)
#define genln()     gen_result = strf("%s\n", gen_result); gen_indentation()

internal_proc void exec_stmt(Resolved_Stmt *stmt);

global_var char *gen_result = "";
global_var int gen_indent   = 0;

Resolved_Templ *global_current_tmpl;
Resolved_Stmt  *global_super_block;

internal_proc void
gen_indentation() {
    gen_result = strf("%s%*.s", gen_result, 4 * gen_indent, "         ");
}

internal_proc Val *
val_from_field(Resolved_Expr *expr) {
    Val *result = 0;
    u8 *raw = (u8 *)expr->expr_field.base->val->ptr + expr->expr_field.offset;

    switch ( expr->type->kind ) {
        case TYPE_STR: {
            result = val_str(*(char **)raw);
        } break;

        case TYPE_FLOAT: {
            result = val_float(*(float *)raw);
        } break;

        case TYPE_INT: {
            result = val_int(*(int *)raw);
        } break;
    }

    return result;
}

internal_proc Val *
exec_expr(Resolved_Expr *expr) {
    Val *result = 0;

    switch (expr->kind) {
        case EXPR_NAME: {
            result = expr->sym->val;
        } break;

        case EXPR_INT: {
            result = expr->val;
        } break;

        case EXPR_FLOAT: {
            result = expr->val;
        } break;

        case EXPR_STR: {
            result = expr->val;
        } break;

        case EXPR_RANGE: {
            result = val_range(expr->expr_range.min, expr->expr_range.max);
        } break;

        case EXPR_UNARY: {
            /* @AUFGABE: vorzeichen */
            result = exec_expr(expr->expr_unary.expr);
        } break;

        case EXPR_FIELD: {
            /* @AUFGABE: mehr als nur eine ebene unterstützen? */
            result = val_from_field(expr);
        } break;

        case EXPR_BINARY: {
            switch ( expr->expr_binary.op ) {
                case T_MUL: {
                    Val calc = *exec_expr(expr->expr_binary.left) * *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_DIV: {
                    Val calc = *exec_expr(expr->expr_binary.left) / *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_PLUS: {
                    Val calc = *exec_expr(expr->expr_binary.left) + *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_MINUS: {
                    Val calc = *exec_expr(expr->expr_binary.left) - *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_LT: {
                    Val calc = *exec_expr(expr->expr_binary.left) < *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;
            }
        } break;

        case EXPR_IS: {
            Val *var_val = exec_expr(expr->expr_is.expr);
            Type *type = expr->expr_is.test->type;
            assert(type->kind == TYPE_TEST);

            result = type->type_test.callback(var_val, expr->expr_is.args, expr->expr_is.num_args);
        } break;

        case EXPR_CALL: {
            Type *type = expr->type;

            assert(expr->stmt);
            assert(expr->stmt->block);
            assert(expr->stmt->block->super);

            /* @AUFGABE: expr->expr_call.args (?) */
            result = type->type_proc.callback(expr->stmt->block->super);
        } break;

        case EXPR_ARRAY_LIT: {
            /* @AUFGABE: schauen was wir hier überhaupt machen müssen */
        } break;

        default: {
            illegal_path();
        } break;
    }

    return result;
}

internal_proc b32
if_expr_cond(Resolved_Expr *if_expr) {
    if ( !if_expr ) {
        return true;
    }

    Val *if_val = exec_expr(if_expr->expr_if.cond);
    assert(if_val->kind == VAL_BOOL);

    return val_bool(if_val);
}

internal_proc void
exec_extends(Resolved_Stmt *stmt, Resolved_Templ *templ) {
    assert(stmt->kind == STMT_EXTENDS);

    for ( int i = 0; i < templ->num_stmts; ++i ) {
        Resolved_Stmt *parent_stmt = templ->stmts[i];

        if ( parent_stmt->kind == STMT_BLOCK ) {
            /* @AUFGABE: eine map aller block statements im tmpl */
            for ( int j = 0; j < global_current_tmpl->num_stmts; ++j ) {
                Resolved_Stmt *child_stmt = global_current_tmpl->stmts[j];
                if ( child_stmt->kind == STMT_BLOCK &&
                     child_stmt->stmt_block.name == parent_stmt->stmt_block.name )
                {
                    global_super_block = parent_stmt;
                    exec_stmt(child_stmt);
                }
            }
        } else {
            exec_stmt(parent_stmt);
        }
    }

    for ( int i = 0; i < global_current_tmpl->num_stmts; ++i ) {
        Resolved_Stmt *child_stmt = global_current_tmpl->stmts[i];

        if ( child_stmt->kind == STMT_EXTENDS ) {
            continue;
        }

        assert(child_stmt->kind == STMT_BLOCK);

        b32 block_already_executed = false;
        for ( int j = 0; j < templ->num_stmts; ++j ) {
            Resolved_Stmt *parent_stmt = templ->stmts[j];

            if ( parent_stmt->stmt_block.name == child_stmt->stmt_block.name ) {
                block_already_executed = true;
            }
        }

        if ( !block_already_executed ) {
            exec_stmt(child_stmt);
        }
    }
}

internal_proc void
exec_stmt(Resolved_Stmt *stmt) {
    switch ( stmt->kind ) {
        case STMT_LIT: {
            genf("%s", stmt->stmt_lit.lit);
        } break;

        case STMT_VAR: {
            Resolved_Expr *if_expr = stmt->stmt_var.if_expr;

            if ( !if_expr || if_expr_cond(if_expr) ) {
                char *value = to_char(exec_expr(stmt->stmt_var.expr));

                for ( int i = 0; i < stmt->stmt_var.num_filter; ++i ) {
                    Resolved_Filter *filter = stmt->stmt_var.filter[i];
                    value = filter->proc(value, filter->args, filter->num_args);
                }

                genf("%s", value);
            } else {
                if ( if_expr->expr_if.else_expr ) {
                    Val *else_val = exec_expr(if_expr->expr_if.else_expr);

                    genf("%s", to_char(else_val));
                }
            }
        } break;

        case STMT_BLOCK: {
            genlnf("<!-- %s -->", stmt->stmt_block.name);
            genln();
            for ( int i = 0; i < stmt->stmt_block.num_stmts; ++i ) {
                exec_stmt(stmt->stmt_block.stmts[i]);
            }
        } break;

        case STMT_SET: {
            stmt->stmt_set.sym->val = exec_expr(stmt->stmt_set.expr);
        } break;

        case STMT_FOR: {
            Val *list = exec_expr(stmt->stmt_for.expr);
            assert(list->kind == VAL_RANGE); /* @AUFGABE: arrays müssen auch funktionieren */

            for ( int i = val_range0(list); i < val_range1(list); ++i ) {
                val_set(stmt->stmt_for.it->val, i);
                for ( int j = 0; j < stmt->stmt_for.num_stmts; ++j ) {
                    exec_stmt(stmt->stmt_for.stmts[j]);
                }
            }
        } break;

        case STMT_IF: {
            Val *val = exec_expr(stmt->stmt_if.expr);

            if ( val_bool(val) ) {
                for ( int i = 0; i < stmt->stmt_if.num_stmts; ++i ) {
                    exec_stmt(stmt->stmt_if.stmts[i]);
                }
            } else {
                b32 elseif_matched = false;
                for ( int i = 0; i < stmt->stmt_if.num_elseifs; ++i ) {
                    Resolved_Stmt *elseif = stmt->stmt_if.elseifs[i];
                    val = exec_expr(elseif->stmt_if.expr);

                    if ( val_bool(val) ) {
                        for ( int j = 0; j < elseif->stmt_if.num_stmts; ++j ) {
                            exec_stmt(elseif->stmt_if.stmts[j]);
                        }
                        elseif_matched = true;
                        break;
                    }
                }

                if ( !elseif_matched && stmt->stmt_if.else_stmt ) {
                    Resolved_Stmt *else_stmt = stmt->stmt_if.else_stmt;
                    for ( int i = 0; i < else_stmt->stmt_if.num_stmts; ++i ) {
                        exec_stmt(else_stmt->stmt_if.stmts[i]);
                    }
                }
            }
        } break;

        case STMT_EXTENDS: {
            if ( stmt->stmt_extends.if_expr ) {
                Resolved_Expr *if_expr = stmt->stmt_extends.if_expr;
                Val *if_cond = exec_expr(if_expr->expr_if.cond);
                assert(if_cond->kind == VAL_BOOL);

                if ( val_bool(if_cond) ) {
                    exec_extends(stmt, stmt->stmt_extends.tmpl);
                } else if ( if_expr->expr_if.else_expr ) {
                    exec_extends(stmt, stmt->stmt_extends.else_tmpl);
                }
            } else {
                exec_extends(stmt, stmt->stmt_extends.tmpl);
            }
        } break;

        case STMT_INCLUDE: {
            Resolved_Expr *expr = stmt->stmt_include.expr;
            assert(expr->kind == EXPR_STR);

            char *content = 0;
            file_read(val_str(expr->val), &content);

            genf("<!-- include %s -->\n%s", val_str(expr->val), content);
        } break;

        default: {
            implement_me();
        } break;
    }
}

PROC_CALLBACK(super) {
    assert(global_super_block);
    assert(global_super_block->kind == STMT_BLOCK);

    for ( int i = 0; i < global_super_block->stmt_block.num_stmts; ++i ) {
        exec_stmt(global_super_block->stmt_block.stmts[i]);
    }

    return val_str("");
}

internal_proc void
exec(Resolved_Templ *templ) {
    global_current_tmpl = templ;

    for ( int i = 0; i < templ->num_stmts; ++i ) {

        if ( templ->stmts[i]->kind == STMT_EXTENDS && i > 0 ) {
            fatal("extends anweisung muss die erste anweisung des templates sein");
        }

        exec_stmt(templ->stmts[i]);
        if ( templ->stmts[i]->kind == STMT_EXTENDS ) {
            break;
        }
    }
}
