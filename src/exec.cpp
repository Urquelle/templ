internal_proc void exec_stmt(Resolved_Stmt *stmt);
internal_proc void exec(Resolved_Templ *templ);

global_var char *gen_result = "";
global_var int gen_indent   = 0;

Resolved_Templ *global_current_tmpl;
Resolved_Stmt  *global_super_block;
Resolved_Stmt  *global_for_stmt;
b32             global_for_break;
b32             global_for_continue;

internal_proc void
gen_indentation() {
    gen_result = strf("%s%*.s", gen_result, 4 * gen_indent, "         ");
}

struct Iterator {
    Val *container;
    int pos;
    int step;
    Val *val;
};

internal_proc Iterator
iterator_init(Val *container) {
    Iterator result = {};

    assert(container->kind == VAL_RANGE || container->kind == VAL_LIST || container->kind == VAL_TUPLE);

    result.container = container;
    result.pos = 0;
    result.step = 1;

    if ( container->kind == VAL_RANGE ) {
        result.step = val_range2(container);
    }

    result.val = val_elem(container, 0);

    return result;
}

internal_proc b32
iterator_valid(Iterator *it) {
    b32 result = it->pos < it->container->len;

    return result;
}

internal_proc b32
iterator_is_last(Iterator *it) {
    b32 result = (it->pos + it->step) == it->container->len;

    return result;
}

internal_proc void
iterator_next(Iterator *it) {
    it->pos += it->step;
    it->val = val_elem(it->container, it->pos);
}

internal_proc Val *
exec_macro(Resolved_Expr *expr) {
    Type *type = expr->type;
    assert(type->kind == TYPE_MACRO);

    for ( int i = 0; i < type->type_macro.num_params; ++i ) {
        Type_Field *field = type->type_macro.params[i];

        Resolved_Arg *arg = (Resolved_Arg *)map_get(&expr->expr_call.nargs, field->sym->name);
        field->sym->val = arg->val;
    }

    char *old_gen_result = gen_result;
    char *temp = "";
    gen_result = temp;

    for ( int i = 0; i < type->type_macro.num_stmts; ++i ) {
        Resolved_Stmt *stmt = type->type_macro.stmts[i];
        exec_stmt(stmt);
    }

    Val *result = val_str(gen_result);
    gen_result = old_gen_result;

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
            result = val_op(expr->expr_unary.op, exec_expr(expr->expr_unary.expr));
        } break;

        case EXPR_FIELD: {
            result = expr->val;
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

                case T_DIV_TRUNC: {
                    Val calc = *exec_expr(expr->expr_binary.left) / *exec_expr(expr->expr_binary.right);
                    result = val_int((int)*(float *)calc.ptr);
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

                case T_LEQ: {
                    Val calc = *exec_expr(expr->expr_binary.left) <= *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_GT: {
                    Val calc = *exec_expr(expr->expr_binary.left) > *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_GEQ: {
                    Val calc = *exec_expr(expr->expr_binary.left) >= *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_AND: {
                    b32 calc = *exec_expr(expr->expr_binary.left) && *exec_expr(expr->expr_binary.right);
                    result = val_bool(calc);
                } break;

                case T_OR: {
                    b32 calc = *exec_expr(expr->expr_binary.left) || *exec_expr(expr->expr_binary.right);
                    result = val_bool(calc);
                } break;

                case T_PERCENT: {
                    Val calc = *exec_expr(expr->expr_binary.left) % *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_POT: {
                    Val calc = *exec_expr(expr->expr_binary.left) ^ *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_EQL: {
                    b32 calc = *exec_expr(expr->expr_binary.left) == *exec_expr(expr->expr_binary.right);
                    result = val_bool(calc);
                } break;

                case T_NEQ: {
                    b32 calc = *exec_expr(expr->expr_binary.left) != *exec_expr(expr->expr_binary.right);
                    result = val_bool(!calc);
                } break;

                default: {
                    illegal_path();
                } break;
            }
        } break;

        case EXPR_IS: {
            Val *var_val = exec_expr(expr->expr_is.expr);
            Type *type = expr->expr_is.test->type;
            assert(type->kind == TYPE_TEST);

            result = type->type_test.callback(var_val, type, expr->expr_is.args, expr->expr_is.num_args);
        } break;

        case EXPR_IN: {
            result = exec_expr(expr->expr_in.set);
        } break;

        case EXPR_CALL: {
            Type *type = expr->type;

            if ( type->kind == TYPE_MACRO ) {
                result = exec_macro(expr);
            } else {
                result = type->type_proc.callback(expr->expr_call.nargs, expr->expr_call.kwargs, expr->expr_call.varargs, expr->expr_call.num_varargs);
            }
        } break;

        case EXPR_TUPLE: {
            for ( int i = 0; i < expr->expr_list.num_expr; ++i ) {
                val_set(expr->val, exec_expr(expr->expr_list.expr[i]), i);
            }

            result = expr->val;
        } break;

        case EXPR_LIST: {
            for ( int i = 0; i < expr->expr_list.num_expr; ++i ) {
                val_set(expr->val, exec_expr(expr->expr_list.expr[i]), i);
            }

            result = expr->val;
        } break;

        case EXPR_BOOL: {
            result = expr->val;
        } break;

        case EXPR_DICT: {
            result = expr->val;
        } break;

        case EXPR_NOT: {
            result = val_neg(exec_expr(expr->expr_not.expr));
        } break;

        case EXPR_SUBSCRIPT: {
            Val *set = exec_expr(expr->expr_subscript.expr);
            Val *index = exec_expr(expr->expr_subscript.index);

            assert(index->kind == VAL_INT);
            result = val_subscript(set, val_int(index));
        } break;

        default: {
            fatal(expr->pos.name, expr->pos.row, "unerwarteter ausdruck");
            illegal_path();
        } break;
    }

    for ( int i = 0; i < expr->num_filters; ++i ) {
        Resolved_Filter *filter = expr->filters[i];
        result = filter->proc(result, filter->args, filter->num_args);
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
        exec_stmt(parent_stmt);
    }

    for ( int i = 0; i < global_current_tmpl->num_stmts; ++i ) {
        Resolved_Stmt *child_stmt = global_current_tmpl->stmts[i];

        if ( !child_stmt || child_stmt->kind == STMT_EXTENDS || child_stmt->kind == STMT_LIT ) {
            continue;
        }

        Resolved_Stmt *block = (Resolved_Stmt *)map_get(&templ->blocks, child_stmt->stmt_block.name);
        if ( !block ) {
            exec_stmt(child_stmt);
        }
    }
}

internal_proc void
exec_stmt_set(Val *dest, Val *source) {
    if ( dest->kind == VAL_INT ) {
        val_set(dest, val_int(source));
    } else if ( dest->kind == VAL_STR ) {
        dest = source;
    } else if ( dest->kind == VAL_LIST ) {
        dest->ptr = source->ptr;
    } else if ( dest->kind == VAL_TUPLE ) {
        dest->ptr = source->ptr;
    } else if ( dest->kind == VAL_CHAR ) {
        Val *orig = (Val *)dest->ptr;

        char * old_char_loc  = utf8_char_goto((char *)orig->ptr, dest->len);
        size_t size_new_char = utf8_char_size((char *)source->ptr);
        size_t size_old_char = utf8_char_size(old_char_loc);
        size_t old_size      = utf8_str_size((char *)orig->ptr);
        size_t new_size      = old_size - size_old_char + size_new_char;

        if ( new_size == old_size && size_new_char == size_old_char ) {
            size_t offset = utf8_char_offset((char *)orig->ptr, old_char_loc);
            utf8_char_write((char *)orig->ptr + offset, (char *)source->ptr);
        } else {
            size_t len = utf8_strlen((char *)orig->ptr);
            char *new_mem = (char *)xcalloc(1, new_size+1);

            for ( int i = 0; i < dest->len; ++i ) {
                utf8_char_write(utf8_char_goto(new_mem, i), utf8_char_goto((char *)orig->ptr, i));
            }

            utf8_char_write(utf8_char_goto(new_mem, dest->len), (char *)source->ptr);

            for ( size_t i = dest->len+1; i < len; ++i ) {
                utf8_char_write(utf8_char_goto(new_mem, i), utf8_char_goto((char *)orig->ptr, i));
            }

            new_mem[new_size] = 0;

            orig->ptr = new_mem;
        }
    } else if ( val_is_undefined(dest) ) {
        dest->kind = source->kind;
        dest->len  = source->len;
        dest->size = source->size;
        dest->ptr  = source->ptr;
    } else {
        fatal(0, 0, "nicht unterstützter datentyp wird in einer set anweisung verwendet");
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
                Val *value = exec_expr(stmt->stmt_var.expr);

                genf("%s", val_to_char(value));
            } else {
                if ( if_expr->expr_if.else_expr ) {
                    Val *else_val = exec_expr(if_expr->expr_if.else_expr);

                    genf("%s", val_to_char(else_val));
                }
            }
        } break;

        case STMT_BLOCK: {
            Resolved_Stmt *block = (Resolved_Stmt *)map_get(&global_current_tmpl->blocks, stmt->stmt_block.name);
            if ( block ) {
                global_super_block = stmt;
            } else {
                block = stmt;
            }

            genln();

            for ( int i = 0; i < block->stmt_block.num_stmts; ++i ) {
                exec_stmt(block->stmt_block.stmts[i]);
            }
        } break;

        case STMT_SET: {
            if ( stmt->stmt_set.num_names == 1 ) {
                Val *dest   = exec_expr(stmt->stmt_set.names[0]);
                Val *source = exec_expr(stmt->stmt_set.expr);

                exec_stmt_set(dest, source);
            } else {
                Val *source = exec_expr(stmt->stmt_set.expr);

                for ( int i = 0; i < stmt->stmt_set.num_names; ++i ) {
                    Val *dest = stmt->stmt_set.names[i]->val;
                    exec_stmt_set(dest, val_elem(source, i));
                }
            }
        } break;

        case STMT_FOR: {
            Val *list = exec_expr(stmt->stmt_for.set);

            if ( list->len ) {
                global_for_stmt = stmt;
                global_for_break = false;
                global_for_continue = false;

                val_set(stmt->stmt_for.loop_length->val, (s32)list->len);

                for ( Iterator it = iterator_init(list); iterator_valid(&it); iterator_next(&it) ) {
                    for ( int i = 0; i < stmt->stmt_for.num_vars; ++i ) {
                        stmt->stmt_for.vars[i]->val = val_elem(it.val, i);
                    }

                    val_set(stmt->stmt_for.loop_last->val, iterator_is_last(&it));

                    for ( int j = 0; j < stmt->stmt_for.num_stmts; ++j ) {
                        exec_stmt(stmt->stmt_for.stmts[j]);
                        if ( global_for_break ) {
                            break;
                        }

                        if ( global_for_continue ) {
                            break;
                        }
                    }

                    if ( global_for_break ) {
                        global_for_break = false;
                        break;
                    }

                    if ( global_for_continue ) {
                        global_for_continue = false;
                        continue;
                    }

                    val_inc(stmt->stmt_for.loop_index->val);
                    val_inc(stmt->stmt_for.loop_index0->val);
                    val_set(stmt->stmt_for.loop_first->val, false);
                }
            } else if ( stmt->stmt_for.else_stmts ) {
                for ( int i = 0; i < stmt->stmt_for.num_else_stmts; ++i ) {
                    exec_stmt(stmt->stmt_for.else_stmts[i]);
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
            for ( int i = 0; i < stmt->stmt_include.num_templ; ++i ) {
                Resolved_Templ *templ = stmt->stmt_include.templ[i];
                for ( int j = 0; j < templ->num_stmts; ++j ) {
                    exec_stmt(templ->stmts[j]);
                }
            }
        } break;

        case STMT_FILTER: {
            char *old_gen_result = gen_result;
            char *temp = "";
            gen_result = temp;

            for ( int i = 0; i < stmt->stmt_filter.num_stmts; ++i ) {
                exec_stmt(stmt->stmt_filter.stmts[i]);
            }

            Val *result = val_str(gen_result);
            for ( int i = 0; i < stmt->stmt_filter.num_filter; ++i ) {
                Resolved_Filter *filter = stmt->stmt_filter.filter[i];
                result = filter->proc(result, filter->args, filter->num_args);
            }

            gen_result = old_gen_result;
            genf("%s", val_to_char(result));
        } break;

        case STMT_RAW: {
            genf("%s", stmt->stmt_raw.value);
        } break;

        case STMT_WITH: {
            for ( int i = 0; i < stmt->stmt_with.num_stmts; ++i ) {
                exec_stmt(stmt->stmt_with.stmts[i]);
            }
        } break;

        case STMT_BREAK: {
            global_for_break = true;
        } break;

        case STMT_CONTINUE: {
            global_for_continue = true;
        } break;

        case STMT_FROM_IMPORT: {
            for ( int i = 0; i < stmt->stmt_module.num_stmts; ++i ) {
                Resolved_Stmt *imported_stmt = stmt->stmt_module.stmts[i];
                exec_stmt(imported_stmt);
            }
        } break;

        case STMT_IMPORT: {
            for ( int i = 0; i < stmt->stmt_module.num_stmts; ++i ) {
                Resolved_Stmt *imported_stmt = stmt->stmt_module.stmts[i];
                exec_stmt(imported_stmt);
            }
        } break;

        case STMT_MACRO: {
            /* @INFO: nix tun */
        } break;

        default: {
            implement_me();
        } break;
    }
}

PROC_CALLBACK(proc_super) {
    assert(global_super_block);
    assert(global_super_block->kind == STMT_BLOCK);

    char *old_gen_result = gen_result;
    char *temp = "";
    gen_result = temp;

    for ( int i = 0; i < global_super_block->stmt_block.num_stmts; ++i ) {
        exec_stmt(global_super_block->stmt_block.stmts[i]);
    }

    Val *result = val_str(gen_result);
    gen_result = old_gen_result;

    return result;
}

PROC_CALLBACK(proc_cycle) {
    assert(global_for_stmt->kind == STMT_FOR);

    s32 loop_index = val_int(global_for_stmt->stmt_for.loop_index->val);
    s32 arg_index  = loop_index % num_varargs;

    return varargs[arg_index]->val;
}

PROC_CALLBACK(proc_range) {
    Resolved_Arg *arg = (Resolved_Arg *)map_get(&nargs, intern_str("start"));
    int start = val_int(arg->val);

    arg = (Resolved_Arg *)map_get(&nargs, intern_str("stop"));
    int stop  = val_int(arg->val);

    arg = (Resolved_Arg *)map_get(&nargs, intern_str("step"));
    int step = val_int(arg->val);

    return val_range(start, stop, step);
}

global_var char *global_lorem_ipsum = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
PROC_CALLBACK(proc_lipsum) {
    Resolved_Arg *arg = (Resolved_Arg *)map_get(&nargs, intern_str("n"));
    s32 n = val_int(arg->val);

    arg = (Resolved_Arg *)map_get(&nargs, intern_str("html"));
    b32 html = val_bool(arg->val);

    arg = (Resolved_Arg *)map_get(&nargs, intern_str("min"));
    s32 min = val_int(arg->val);

    arg = (Resolved_Arg *)map_get(&nargs, intern_str("max"));
    s32 max = (s32)MIN(val_int(arg->val), utf8_strlen(global_lorem_ipsum));

    char *result = "";
    for ( int i = 0; i < n; ++i ) {
        result = strf("%s%s%.*s%s", result, (html) ? "<div>" : "", max-min, global_lorem_ipsum, (html) ? "</div>" : "");
    }

    return val_str(result);
}

internal_proc void
exec_reset() {
    if ( gen_result && utf8_strlen(gen_result) ) {
        free(gen_result);
        gen_result = "";
    }
}

internal_proc void
exec(Resolved_Templ *templ) {
    global_current_tmpl = templ;

    for ( int i = 0; i < templ->num_stmts; ++i ) {
        Resolved_Stmt *stmt = templ->stmts[i];
        if ( !stmt ) {
            continue;
        }

        if ( stmt->kind == STMT_EXTENDS && i > 0 ) {
            fatal(stmt->pos.name, stmt->pos.row, "extends anweisung muss die erste anweisung des templates sein");
        }

        exec_stmt(stmt);

        /* @AUFGABE: wird das noch benötigt? */
        if ( stmt->kind == STMT_EXTENDS ) {
            break;
        }
    }
}
