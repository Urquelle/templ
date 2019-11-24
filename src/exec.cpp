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

    assert(container->kind == VAL_RANGE || container->kind == VAL_LIST || container->kind == VAL_TUPLE || container->kind == VAL_DICT);

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
exec_expr(Resolved_Expr *expr) {
    Val *result = 0;

    switch (expr->kind) {
        case EXPR_NAME: {
            Sym *sym = sym_get(expr->expr_name.name);
            result = sym->val;
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
            Val *base = exec_expr(expr->expr_field.base);
            Scope *scope = (base->kind == VAL_DICT) ? (Scope *)base->ptr : (Scope *)base->user_data;

            Scope *prev_scope = scope_set(scope);
            Sym *sym = sym_get(expr->expr_field.field);
            scope_set(prev_scope);

            result = sym->val;
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

                case T_TILDE: {
                    Val *left  = exec_expr(expr->expr_binary.left);
                    Val *right = exec_expr(expr->expr_binary.right);

                    char *out  = strf("%s%s", val_print(left), val_print(right));
                    result = val_str(out);
                } break;

                default: {
                    illegal_path();
                } break;
            }
        } break;

        case EXPR_IS: {
            Val *operand = exec_expr(expr->expr_is.operand);
            Resolved_Expr *tester = expr->expr_is.tester;
            Val_Proc *proc = (Val_Proc *)tester->expr_call.expr->val->ptr;

            result = proc->callback( operand,
                    tester->expr_call.args, tester->expr_call.num_args,
                    tester->expr_call.nargs, tester->expr_call.narg_keys,
                    tester->expr_call.num_narg_keys, tester->expr_call.kwargs,
                    tester->expr_call.num_kwargs, tester->expr_call.varargs,
                    tester->expr_call.num_varargs);
        } break;

        case EXPR_IF: {
            result = exec_expr(expr->expr_if.cond);
        } break;

        case EXPR_CALL: {
            Val *val = exec_expr(expr->expr_call.expr);
            Val_Proc *proc = (Val_Proc *)val->ptr;

            result = proc->callback(val,
                    expr->expr_call.args, expr->expr_call.num_args,
                    expr->expr_call.nargs, expr->expr_call.narg_keys,
                    expr->expr_call.num_narg_keys, expr->expr_call.kwargs,
                    expr->expr_call.num_kwargs, expr->expr_call.varargs,
                    expr->expr_call.num_varargs);
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

            if ( set->kind == VAL_DICT ) {
                assert(index->kind == VAL_STR);

                Scope *scope = (Scope *)set->ptr;
                Scope *prev_scope = scope_set(scope);
                Sym *sym = sym_get(val_str(index));
                result = sym->val;

                scope_set(prev_scope);
            } else {
                assert(index->kind == VAL_INT);
                result = val_subscript(set, val_int(index));
            }
        } break;

        case EXPR_NONE: {
            return val_none();
        } break;

        default: {
            fatal(expr->pos.name, expr->pos.row, "unerwarteter ausdruck");
            illegal_path();
        } break;
    }

    for ( int i = 0; i < expr->num_filters; ++i ) {
        Resolved_Expr *filter = expr->filters[i];
        Val_Proc *proc = (Val_Proc *)filter->expr_call.expr->val->ptr;
        result = proc->callback( result,
                filter->expr_call.args, filter->expr_call.num_args,
                filter->expr_call.nargs, filter->expr_call.narg_keys,
                filter->expr_call.num_narg_keys, filter->expr_call.kwargs,
                filter->expr_call.num_kwargs, filter->expr_call.varargs,
                filter->expr_call.num_varargs);
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
exec_stmt_set(Val *dest, Val *source) {
    erstes_if ( source->kind == VAL_STR && dest->kind == VAL_CHAR ) {
        Val *orig = (Val *)dest->ptr;

        char * old_char_loc  = utf8_char_goto((char *)orig->ptr, dest->len);
        size_t size_new_char = utf8_char_size((char *)source->ptr);
        size_t size_old_char = utf8_char_size(old_char_loc);
        size_t old_size      = utf8_str_size((char *)orig->ptr);
        size_t new_size      = old_size - size_old_char + size_new_char;

        if ( new_size == old_size && size_new_char == size_old_char ) {
            size_t offset = utf8_char_offset((char *)orig->ptr, old_char_loc);
            utf8_char_write((char *)orig->ptr + offset, (char *)source->ptr);
            dest->ptr = intern_str((char *)dest->ptr);
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
            orig->ptr = intern_str(new_mem);
        }
    } else {
        dest->kind = source->kind;
        dest->size = source->size;
        dest->len  = source->len;
        dest->ptr  = source->ptr;
        dest->user_data = source->user_data;
    }
}

internal_proc void
exec_stmt(Resolved_Stmt *stmt) {
    switch ( stmt->kind ) {
        case STMT_LIT: {
            genf("%s", stmt->stmt_lit.lit);
        } break;

        case STMT_VAR: {
            Resolved_Expr *if_expr = stmt->stmt_var.expr->if_expr;

            if ( !if_expr || if_expr_cond(if_expr) ) {
                Val *value = exec_expr(stmt->stmt_var.expr);

                genf("%s", val_print(value));
            } else {
                if ( if_expr->expr_if.else_expr ) {
                    Val *else_val = exec_expr(if_expr->expr_if.else_expr);

                    genf("%s", val_print(else_val));
                }
            }
        } break;

        case STMT_BLOCK: {
            if ( !stmt->stmt_block.executed ) {
                Resolved_Stmt *parent_block = stmt->stmt_block.parent_block;
                global_super_block = parent_block;

                if ( stmt->stmt_block.child_block ) {
                    Resolved_Stmt *block = stmt->stmt_block.child_block;
                    global_super_block = stmt;
                    for ( int i = 0; i < block->stmt_block.num_stmts; ++i ) {
                        exec_stmt(block->stmt_block.stmts[i]);
                    }
                    block->stmt_block.executed = true;
                } else {
                    for ( int i = 0; i < stmt->stmt_block.num_stmts; ++i ) {
                        exec_stmt(stmt->stmt_block.stmts[i]);
                    }
                }
            }
        } break;

        case STMT_SET: {
            Val *source = exec_expr(stmt->stmt_set.expr);

            /* @AUFGABE: evtl. überlegen ob eine liste als eine verkettung von
             *           Val * umgesetzt werden sollte, die einen zeiger zum
             *           nächsten element enthalten.
             *
             *           damit müßten wir uns nicht darum kümmern ob links von
             *           der zuweisung nur ein element steht, oder mehrere.
             */
            if ( stmt->stmt_set.num_names == 1 ) {
                Val *dest = exec_expr(stmt->stmt_set.names[0]);
                exec_stmt_set(dest, source);
            } else {
                for ( int i = 0; i < stmt->stmt_set.num_names; ++i ) {
                    Val *dest = exec_expr(stmt->stmt_set.names[i]);
                    exec_stmt_set(dest, val_elem(source, i));
                }
            }
        } break;

        case STMT_FOR: {
            global_for_stmt = stmt;
            Val *list = exec_expr(stmt->stmt_for.set);

            Scope *prev_scope = scope_set(stmt->stmt_for.scope);
            if ( list->len ) {
                global_for_break = false;
                global_for_continue = false;

                val_set(stmt->stmt_for.loop_length->val, (s32)list->len);
                val_set(stmt->stmt_for.loop_revindex->val, (s32)list->len);
                val_set(stmt->stmt_for.loop_revindex0->val, (s32)list->len-1);

                for ( Iterator it = iterator_init(list); iterator_valid(&it); iterator_next(&it) ) {
                    for ( int i = 0; i < stmt->stmt_for.num_vars; ++i ) {
                        stmt->stmt_for.vars[i]->val = val_elem(it.val, i);
                    }

                    if ( stmt->stmt_for.set->if_expr ) {
                        Val *ret = exec_expr(stmt->stmt_for.set->if_expr);

                        if ( !val_bool(ret) ) {
                            continue;
                        }
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
                    val_dec(stmt->stmt_for.loop_revindex->val);
                    val_dec(stmt->stmt_for.loop_revindex0->val);
                    val_set(stmt->stmt_for.loop_first->val, false);
                }
            } else if ( stmt->stmt_for.else_stmts ) {
                for ( int i = 0; i < stmt->stmt_for.num_else_stmts; ++i ) {
                    exec_stmt(stmt->stmt_for.else_stmts[i]);
                }
            }

            scope_set(prev_scope);
        } break;

        case STMT_IF: {
            Val *val = exec_expr(stmt->stmt_if.expr);

            if ( val_bool(val) ) {
                for ( int i = 0; i < stmt->stmt_if.num_stmts; ++i ) {
                    exec_stmt(stmt->stmt_if.stmts[i]);
                }
            } else if ( stmt->stmt_if.else_stmt ) {
                exec_stmt(stmt->stmt_if.else_stmt);
            }
        } break;

        case STMT_EXTENDS: {
            if ( stmt->stmt_extends.name->if_expr ) {
                Resolved_Expr *if_expr = stmt->stmt_extends.name->if_expr;
                Val *if_cond = exec_expr(if_expr->expr_if.cond);
                assert(if_cond->kind == VAL_BOOL);

                if ( val_bool(if_cond) ) {
                    exec(stmt->stmt_extends.tmpl);
                } else if ( if_expr->expr_if.else_expr ) {
                    exec(stmt->stmt_extends.else_tmpl);
                }
            } else {
                exec(stmt->stmt_extends.tmpl);
            }
        } break;

        case STMT_INCLUDE: {
            for ( int i = 0; i < stmt->stmt_include.num_templ; ++i ) {
                Resolved_Templ *t = stmt->stmt_include.templ[i];
                Scope *prev_scope = scope_set(t->scope);

                for ( int j = 0; j < t->num_stmts; ++j ) {
                    exec_stmt(t->stmts[j]);
                }

                scope_set(prev_scope);
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
                Resolved_Expr *filter = stmt->stmt_filter.filter[i];

                Val_Proc *proc = (Val_Proc *)filter->expr_call.expr->val->ptr;
                result = proc->callback( result,
                        filter->expr_call.args, filter->expr_call.num_args,
                        filter->expr_call.nargs, filter->expr_call.narg_keys,
                        filter->expr_call.num_narg_keys, filter->expr_call.kwargs,
                        filter->expr_call.num_kwargs, filter->expr_call.varargs,
                        filter->expr_call.num_varargs);
            }

            gen_result = old_gen_result;
            genf("%s", val_print(result));
        } break;

        case STMT_RAW: {
            genf("%s", stmt->stmt_raw.value);
        } break;

        case STMT_WITH: {
            Scope *prev_scope = scope_set(stmt->stmt_with.scope);
            for ( int i = 0; i < stmt->stmt_with.num_stmts; ++i ) {
                exec_stmt(stmt->stmt_with.stmts[i]);
            }
            scope_set(prev_scope);
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
            Scope *scope = (Scope *)stmt->stmt_module.sym->val->ptr;
            Scope *prev_scope = scope_set(scope);

            for ( int i = 0; i < stmt->stmt_module.num_stmts; ++i ) {
                Resolved_Stmt *imported_stmt = stmt->stmt_module.stmts[i];
                exec_stmt(imported_stmt);
            }

            scope_set(prev_scope);
        } break;

        case STMT_MACRO: {
            /* @INFO: nix tun */
        } break;

        default: {
            implement_me();
        } break;
    }
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
    Scope *prev_scope = scope_set(templ->scope);

    for ( int i = 0; i < templ->num_stmts; ++i ) {
        Resolved_Stmt *stmt = templ->stmts[i];

        if ( !stmt ) {
            continue;
        }

        exec_stmt(stmt);
    }

    scope_set(prev_scope);
}
