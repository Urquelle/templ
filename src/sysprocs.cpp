#define BASE(EXPR) exec_expr(EXPR->expr_field.base)

internal_proc PROC_CALLBACK(proc_super) {
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

internal_proc PROC_CALLBACK(proc_exec_macro) {
    Scope *scope = operand->scope;
    Scope *prev_scope = scope_set(scope);

    for ( int i = 0; i < num_narg_keys; ++i ) {
        char *key = narg_keys[i];
        Resolved_Arg *arg = narg(key);
        Sym *sym = sym_get(key);
        sym->val = arg->val;
    }

    char *old_gen_result = gen_result;
    char *temp = "";
    gen_result = temp;

    Val_Proc *data = (Val_Proc *)operand->ptr;
    for ( int i = 0; i < data->num_stmts; ++i ) {
        Resolved_Stmt *stmt = data->stmts[i];
        exec_stmt(stmt);
    }

    scope_set(prev_scope);

    Val *result = val_str(gen_result);
    gen_result = old_gen_result;

    return result;
}

internal_proc PROC_CALLBACK(proc_dict) {
    Scope *scope = scope_new(0, "dict");
    Scope *prev_scope = scope_set(scope);

    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *arg = kwargs[i];
        sym_push_var(arg->name, arg->type, arg->val);
    }

    scope_set(prev_scope);

    return val_dict(scope);
}

internal_proc PROC_CALLBACK(proc_cycle) {
    Sym *sym = sym_get(symname_loop);
    Scope *scope = sym->val->scope;
    Scope *prev_scope = scope_set(scope);
    Sym *sym_idx = sym_get(symname_index);

    s32 loop_index = val_int(sym_idx->val);
    s32 arg_index  = loop_index % num_varargs;
    val_inc(sym_idx->val);

    scope_set(prev_scope);

    return varargs[arg_index]->val;
}

struct Cycler {
    size_t num_args;
    Resolved_Arg **args;
    s32 idx;
};
internal_proc PROC_CALLBACK(proc_cycler) {
    Cycler *cycler = ALLOC_STRUCT(&templ_arena, Cycler);
    cycler->num_args = num_varargs;
    cycler->args = varargs;
    cycler->idx = 0;

    Scope *cycler_scope = scope_new(0, "cycler");
    Scope *prev_scope = scope_set(cycler_scope);

    Val *val_next  = val_proc(0, 0, type_any, proc_cycler_next);
    val_next->user_data = cycler;
    Val *val_reset = val_proc(0, 0, 0,        proc_cycler_reset);
    val_reset->user_data = cycler;

    sym_push_proc("next",    type_proc(0, 0, type_any), val_next);
    sym_push_proc("reset",   type_proc(0, 0,        0), val_reset);
    sym_push_var("current",  type_any, varargs[0]->val);

    scope_set(prev_scope);

    return val_dict(cycler_scope);
}

internal_proc PROC_CALLBACK(proc_cycler_next) {
    Cycler *cycler = (Cycler *)operand->user_data;

    cycler->idx = (cycler->idx + 1) % cycler->num_args;

    return cycler->args[cycler->idx]->val;
}

internal_proc PROC_CALLBACK(proc_cycler_reset) {
    Cycler *cycler = (Cycler *)operand->user_data;
    cycler->idx = 0;

    return 0;
}

struct Joiner {
    int counter;
    Val *val;
};
internal_proc PROC_CALLBACK(proc_joiner) {
    Joiner *j = ALLOC_STRUCT(&templ_arena, Joiner);
    j->val = narg("sep")->val;
    j->counter = 0;

    Val *result = val_proc(0, 0, type_str, proc_joiner_call);
    result->user_data = j;

    return result;
}

internal_proc PROC_CALLBACK(proc_joiner_call) {
    Joiner *j = (Joiner *)operand->user_data;
    Val *result = 0;

    if ( j->counter == 0 ) {
        result = val_str("");
    } else {
        result = j->val;
    }

    j->counter += 1;
    return result;
}

internal_proc PROC_CALLBACK(proc_loop) {
    s32 depth  = 1;
    s32 depth0 = 0;

    /* elternloop */ {
        Scope *scope = operand->scope;
        Scope *prev_scope = scope_set(scope);

        Sym *sym_depth  = sym_get(intern_str("depth"));
        Sym *sym_depth0 = sym_get(intern_str("depth0"));

        depth  = val_int(sym_depth->val);
        depth0 = val_int(sym_depth0->val);

        scope_set(prev_scope);
    }

    Scope *scope_for = scope_enter("for");

    Resolved_Expr *set = args[0];

    /* loop variablen {{{ */
    Type_Field *any_type[] = { type_field("s", type_any) };
    Type_Field *loop_type[] = { type_field("s", type_any) };

    Scope *scope = scope_new(current_scope, "loop");

    Type *type = type_proc(loop_type, 1, 0);
    type->scope = scope;

    Val *val = val_proc(loop_type, 1, 0, proc_loop);
    val->user_data = scope;

    sym_push_var(symname_loop, type, val);
    Scope *prev_scope = scope_set(scope);

    Sym *loop_index     = sym_push_var(symname_index, type_int,  val_int(1));
    Sym *loop_index0    = sym_push_var("index0",      type_int,  val_int(0));
    Sym *loop_revindex  = sym_push_var("revindex",    type_int,  val_int(0));
    Sym *loop_revindex0 = sym_push_var("revindex0",   type_int,  val_int(0));
    Sym *loop_first     = sym_push_var("first",       type_bool, val_bool(true));
    Sym *loop_last      = sym_push_var("last",        type_bool, val_bool(false));
    Sym *loop_length    = sym_push_var("length",      type_int,  val_int(0));
    Sym *loop_cycle     = sym_push_proc("cycle",      type_proc(any_type, 0, 0), val_proc(any_type, 0, 0, proc_cycle));
    Sym *loop_depth     = sym_push_var("depth",       type_int,  val_int(depth + 1));
    Sym *loop_depth0    = sym_push_var("depth0",      type_int,  val_int(depth0 + 1));

    scope_set(prev_scope);
    /* }}} */

    scope_leave();

    Resolved_Stmt *stmt = resolved_stmt_for(scope_for,
            global_for_stmt->stmt_for.vars, global_for_stmt->stmt_for.num_vars,
            set, global_for_stmt->stmt_for.stmts, buf_len(global_for_stmt->stmt_for.stmts),
            global_for_stmt->stmt_for.else_stmts, buf_len(global_for_stmt->stmt_for.else_stmts),
            loop_index, loop_index0, loop_revindex, loop_revindex0,
            loop_first, loop_last, loop_length, loop_cycle, loop_depth, loop_depth0);


    char *old_gen_result = gen_result;
    char *temp = "";
    gen_result = temp;

    exec_stmt(stmt);

    Val *result = val_str(gen_result);
    gen_result = old_gen_result;

    return result;
}

internal_proc PROC_CALLBACK(proc_range) {
    int start = val_int(narg("start")->val);
    int stop  = val_int(narg("stop")->val);
    int step  = val_int(narg("step")->val);

    return val_range(start, stop, step);
}

global_var char *global_lorem_ipsum = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
internal_proc PROC_CALLBACK(proc_lipsum) {
    s32 n    = val_int(narg("n")->val);
    b32 html = val_bool(narg("html")->val);
    s32 min  = val_int(narg("min")->val);
    s32 max  = (s32)MIN(val_int(narg("max")->val), utf8_strlen(global_lorem_ipsum));

    char *result = "";
    for ( int i = 0; i < n; ++i ) {
        result = strf("%s%s%.*s%s", result, (html) ? "<div>" : "", max-min, global_lorem_ipsum, (html) ? "</div>" : "");
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_namespace) {
    Scope *scope = scope_new(0, "namespace");
    Scope *prev_scope = scope_set(scope);

    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *arg = kwargs[i];
        sym_push_var(arg->name, arg->type, arg->val);
    }

    scope_set(prev_scope);

    return val_dict(scope);
}
/* @INFO: any methoden {{{ */
internal_proc PROC_CALLBACK(proc_any_default) {
    char *default_value = val_str(narg("s")->val);
    b32 boolean = val_bool(narg("boolean")->val);

    Val *val = exec_expr(expr->expr_field.base);
    if ( val_is_undefined(val) || boolean && !val_is_true(val) ) {
        return val_str(default_value);
    }

    return val;
}

internal_proc PROC_CALLBACK(proc_any_first) {
    Val *val = exec_expr(expr->expr_field.base);
    Val *result = val_elem(val, 0);

    return result;
}

internal_proc PROC_CALLBACK(proc_any_groupby) {
    char *attribute = val_str(narg("attribute")->val);

    Scope *prev_scope = current_scope;
    Map list = {};
    char **keys = 0;
    Val *val = exec_expr(expr->expr_field.base);

    for ( int i = 0; i < val->len; ++i ) {
        Val *v = val_elem(val, i);
        assert(v->kind == VAL_DICT);

        scope_set(v->scope);
        Sym *sym = sym_get(attribute);

        char *key = intern_str(val_print(sym->val));

        Val **vals = (Val **)map_get(&list, key);
        buf_push(keys, key);
        buf_push(vals, v);
        map_put(&list, key, vals);
    }

    Val **vals = 0;
    for ( int i = 0; i < buf_len(keys); ++i ) {
        Scope *scope = scope_new(0, keys[i]);
        scope_set(scope);
        sym_push_var(intern_str("grouper"), type_str, val_str(keys[i]));

        Val **v = (Val **)map_get(&list, keys[i]);
        sym_push_var(intern_str("list"), type_list(type_any), val_list(v, buf_len(v)));
        buf_push(vals, val_dict(scope));
    }

    scope_set(prev_scope);

    return val_list(vals, buf_len(vals));
}

internal_proc PROC_CALLBACK(proc_any_join) {
    char *sep = val_str(narg("d")->val);
    Val *attr = narg("attribute")->val;
    char *result = "";

    Val *val = exec_expr(expr->expr_field.base);
    if (val->len < 2) {
        return val;
    }

    if ( attr->kind == VAL_NONE ) {
        result = strf("%s", val_print(val_elem(val, 0)));
        for ( int i = 1; i < val->len; ++i ) {
            result = strf("%s%s%s", result, sep, val_print(val_elem(val, i)));
        }
    } else {
        Val *v = val_elem(val, 0);
        Scope *scope = v->scope;
        Scope *prev_scope = scope_set(scope);

        Sym *sym = sym_get(val_str(attr));
        result = strf("%s", val_print(sym->val));

        for ( int i = 1; i < val->len; ++i ) {
            v = val_elem(val, i);
            scope_set(v->scope);
            sym = sym_get(val_str(attr));

            result = strf("%s%s%s", result, sep, val_print(sym->val));
        }

        scope_set(prev_scope);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_any_last) {
    Val *val = exec_expr(expr->expr_field.base);
    Val *result = val_elem(val, (int)val->len-1);

    return result;
}

internal_proc PROC_CALLBACK(proc_any_length) {
    Val *val = exec_expr(expr->expr_field.base);
    Val *result = val_int((int)val->len);

    return result;
}

internal_proc PROC_CALLBACK(proc_any_list) {
    Val *val = exec_expr(expr->expr_field.base);
    if ( val->kind == VAL_LIST ) {
        return operand;
    }

    Val **vals = 0;
    for ( int i = 0; i < val->len; ++i ) {
        Val *v = val_elem(val, i);
        buf_push(vals, v);
    }

    return val_list(vals, buf_len(vals));
}

internal_proc PROC_CALLBACK(proc_any_map) {
    char *attribute = 0;
    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *kwarg = kwargs[i];
        if ( kwarg->name == intern_str("attribute") ) {
            attribute = val_str(kwarg->val);
        }
    }

    Val *val = exec_expr(expr->expr_field.base);
    Scope *prev_scope = current_scope;
    Val **vals = 0;
    if ( attribute ) {
        for ( int i = 0; i < val->len; ++i ) {
            Val *elem = val_elem(val, i);
            scope_set(elem->scope);
            Sym *sym = sym_get(attribute);

            if ( sym ) {
                buf_push(vals, sym->val);
            }
        }
    }

    scope_set(prev_scope);
    return val_list(vals, buf_len(vals));
}

internal_proc PROC_CALLBACK(proc_any_max) {
    Val *val = exec_expr(expr->expr_field.base);
    if ( val->len == 1 ) {
        return val;
    }

    b32 case_sensitive = val_bool(narg("case_sensitive")->val);
    Val *attribute = narg("attribute")->val;

    Val *result = val_elem(val, 0);
    for ( int i = 1; i < val->len; ++i ) {
        Val *right = val_elem(val, i);

        Val *left_tmp = result;
        Val *right_tmp = right;

        if ( attribute->kind != VAL_NONE ) {
            assert(result->kind == VAL_DICT);
            assert(right->kind == VAL_DICT);

            left_tmp  = scope_attr(result->scope, val_str(attribute))->val;
            right_tmp = scope_attr(right->scope, val_str(attribute))->val;
        }

        if ( !case_sensitive && left_tmp->kind == VAL_STR && right_tmp->kind == VAL_STR ) {
            char *left_str  = utf8_str_tolower(val_str(left_tmp));
            char *right_str = utf8_str_tolower(val_str(right_tmp));

            result = (utf8_strcmp(left_str, right_str) > 0) ? result : right;
        } else {
            result = (*left_tmp > *right_tmp) ? result : right;
        }
    }

    return result;
}

internal_proc PROC_CALLBACK(proc_any_min) {
    Val *val = exec_expr(expr->expr_field.base);
    if ( val->len == 1 ) {
        return val;
    }

    b32 case_sensitive = val_bool(narg("case_sensitive")->val);
    Val *attribute = narg("attribute")->val;

    Val *result = val_elem(val, 0);
    for ( int i = 1; i < val->len; ++i ) {
        Val *right  = val_elem(val, i);

        Val *left_tmp  = result;
        Val *right_tmp = right;

        if ( attribute->kind != VAL_NONE ) {
            assert(result->kind == VAL_DICT);
            assert(right->kind == VAL_DICT);

            left_tmp  = scope_attr(result->scope, val_str(attribute))->val;
            right_tmp = scope_attr(right->scope, val_str(attribute))->val;
        }

        if ( !case_sensitive && left_tmp->kind == VAL_STR && right_tmp->kind == VAL_STR ) {
            char *left_str  = utf8_str_tolower(val_str(left_tmp));
            char *right_str = utf8_str_tolower(val_str(right_tmp));

            result = (utf8_strcmp(left_str, right_str) < 0) ? result : right;
        } else {
            result = (*left_tmp < *right_tmp) ? result : right;
        }
    }

    return result;
}

internal_proc PROC_CALLBACK(proc_any_pprint) {
    Val *val = exec_expr(expr->expr_field.base);
    b32 verbose = val_bool(narg("verbose")->val);
    char *result = val_pprint(val, verbose);

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_any_random) {
    Val *val = exec_expr(expr->expr_field.base);
    s32 idx = rand() % val->len;
    Val *result = val_elem(val, idx);

    return result;
}

internal_proc PROC_CALLBACK(proc_any_replace) {
    Val *val = exec_expr(expr->expr_field.base);

    char *str = val_str(val);
    char  *old_str = val_str(narg("old")->val);
    size_t old_size = utf8_str_size(old_str);
    char  *new_str = val_str(narg("new")->val);
    size_t new_size = utf8_str_size(new_str);
    Resolved_Arg *count_arg = narg("count");
    s32 count = ( count_arg->val->kind == VAL_NONE ) ? -1 : val_int(count_arg->val);

    char *result = "";
    char *substr_ptr = strstr(str, old_str);
    char *ptr = str;
    while ( substr_ptr && count ) {
        result = "";

        while ( ptr != substr_ptr ) {
            result = strf("%s%c", result, *ptr);
            ptr++;
        }

        for ( int i = 0; i < new_size; ++i ) {
            result = strf("%s%c", result, new_str[i]);
        }

        ptr += old_size;
        while ( *ptr ) {
            result = strf("%s%c", result, *ptr);
            ptr++;
        }

        substr_ptr = strstr(result, old_str);
        ptr = result;

        count--;
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_any_reject) {
    assert(num_varargs > 0);
    Sym *sym = scope_attr(&tester_scope, val_str(varargs[0]->val));
    assert(sym->val->kind == VAL_PROC);
    Val_Proc *proc = (Val_Proc *)sym->val->ptr;

    Resolved_Arg **tester_varargs = (Resolved_Arg **)varargs + 1;
    Resolved_Expr **tester_args = (Resolved_Expr **)args + 1;
    Map tester_nargs = {};
    char **tester_narg_keys = 0;

    /* @INFO: args in nargs speichern bei übereinstimmung. das erste argument
     *        wird übersprungen, da es sich um den namen des testers handelt.
     */
    for ( int i = 0; i < num_args-1; ++i ) {
        Resolved_Expr *arg = tester_args[i];
        if ( sym->type->type_proc.num_params > i ) {
            Type_Field *param = sym->type->type_proc.params[i];
            map_put(&tester_nargs, param->name,
                    resolved_arg(arg->pos, param->name, arg->type, arg->val));
        }
    }

    /* @INFO: kwargs in nargs speichern bei übereinstimmung */
    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *kwarg = kwargs[i];
        for ( int j = 0; j < sym->type->type_proc.num_params; ++j ) {
            Type_Field *param = sym->type->type_proc.params[j];

            if ( kwarg->name == param->name ) {
                map_put(&tester_nargs, param->name, kwarg);
            }
        }
    }

    Val *val = exec_expr(expr->expr_field.base);
    Val **result = 0;
    for ( int i = 0; i < val->len; ++i ) {
        Val *elem = val_elem(val, i);

        Val *ret = proc->callback(
                elem, expr, args, num_args, &tester_nargs,
                tester_narg_keys, buf_len(tester_narg_keys), kwargs, num_kwargs,
                tester_varargs, num_varargs-1);

        if ( val_bool(ret) ) {
            buf_push(result, elem);
        }
    }

    return val_list(result, buf_len(result));
}

internal_proc PROC_CALLBACK(proc_any_rejectattr) {
    Val *val = exec_expr(expr->expr_field.base);

    if ( num_varargs == 1 ) {
        char *attr = val_str(varargs[0]->val);
        Val **vals = 0;
        for ( int i = 0; i < val->len; ++i ) {
            Val *v = val_elem(val, i);
            Sym *sym = scope_attr(v->scope, attr);

            if ( val_bool(sym->val) ) {
                buf_push(vals, v);
            }
        }

        return val_list(vals, buf_len(vals));
    } else if ( num_varargs > 1 ) {
        Sym *sym = scope_attr(&tester_scope, val_str(varargs[1]->val));
        assert(sym->val->kind == VAL_PROC);
        Val_Proc *proc = (Val_Proc *)sym->val->ptr;

        Resolved_Arg **tester_varargs = (Resolved_Arg **)varargs + 2;
        Resolved_Expr **tester_args = (Resolved_Expr **)args + 2;
        Map tester_nargs = {};
        char **tester_narg_keys = 0;

        /* @INFO: args in nargs speichern bei übereinstimmung. das erste argument
        *        wird übersprungen, da es sich um den namen des testers handelt.
        */
        for ( int i = 0; i < num_args-2; ++i ) {
            Resolved_Expr *arg = tester_args[i];
            if ( sym->type->type_proc.num_params > i ) {
                Type_Field *param = sym->type->type_proc.params[i];
                map_put(&tester_nargs, param->name,
                        resolved_arg(arg->pos, param->name, arg->type, arg->val));
            }
        }

        /* @INFO: kwargs in nargs speichern bei übereinstimmung */
        for ( int i = 0; i < num_kwargs; ++i ) {
            Resolved_Arg *kwarg = kwargs[i];
            for ( int j = 0; j < sym->type->type_proc.num_params; ++j ) {
                Type_Field *param = sym->type->type_proc.params[j];

                if ( kwarg->name == param->name ) {
                    map_put(&tester_nargs, param->name, kwarg);
                }
            }
        }

        char *attr = val_str(varargs[0]->val);
        Val **result = 0;
        for ( int i = 0; i < val->len; ++i ) {
            Val *elem = val_elem(val, i);
            Sym *elem_sym = scope_attr(elem->scope, attr);

            Val *ret = proc->callback(
                    elem_sym->val, expr, args, num_args, &tester_nargs,
                    tester_narg_keys, buf_len(tester_narg_keys), kwargs, num_kwargs,
                    tester_varargs, num_varargs-1);

            if ( val_bool(ret) ) {
                buf_push(result, elem);
            }
        }

        return val_list(result, buf_len(result));
    }

    return val_str("");
}

internal_proc PROC_CALLBACK(proc_any_reverse) {
    Val *val = exec_expr(expr->expr_field.base);
    if ( val->len == 1 ) {
        return val;
    }

    Val *result = 0;
    if ( val->kind == VAL_STR ) {
        char *str = "";
        int i = 1;
        size_t size = utf8_str_size((char *)val->ptr);
        while ( i <= size ) {
            int offset = (int)size - i;
            char c = ((char *)val->ptr)[offset];
            str = strf("%s%c", str, c);
            i++;
        }

        result = val_str(str);
    } else {
        Val **vals = 0;
        int i = 1;
        while ( i <= val->len ) {
            Val *v = val_elem(val, (int)val->len - i);
            buf_push(vals, v);
            i++;
        }

        result = val_list(vals, buf_len(vals));
    }

    return result;
}
/* }}} */
/* @INFO: dict methoden {{{ */
internal_proc PROC_CALLBACK(proc_dict_attr) {
    Resolved_Expr *base = expr->expr_field.base;

    Scope *scope = base->val->scope;
    Scope *prev_scope = scope_set(scope);

    Val *name = narg("name")->val;
    Sym *sym = sym_get(val_str(name));
    Val *result = sym->val;

    scope_set(prev_scope);

    return result;
}

internal_proc void
quicksort(Sym **left, Sym **right, b32 case_sensitive, char *by, b32 reverse) {
    char *key = intern_str("key");

#define compneq(left, right) ((by == key) ? (utf8_strcmp((left)->name, (right)->name) != 0) : (*((left)->val) != *((right)->val)))
#define compgt(left, right)  ((by == key) ? (utf8_strcmp((left)->name, (right)->name)  > 0) : (*(left)->val > *(right)->val))
#define complt(left, right)  ((by == key) ? (utf8_strcmp((left)->name, (right)->name)  < 0) : (*(left)->val < *(right)->val))

    if ( compneq(*left, *right) ) {
        Sym **ptr0 = left;
        Sym **ptr1 = left;
        Sym **ptr2 = left;

        Sym *pivot = *left;

        do {
            ptr2 = ptr2 + 1;
            b32 check = ( reverse ) ? ( compgt(*ptr2, pivot) ) : ( complt(*ptr2, pivot) );

            if ( check ) {
                ptr0 = ptr1;
                ptr1 = ptr1 + 1;

                Sym *temp = *ptr1;
                *ptr1 = *ptr2;
                *ptr2 = temp;
            }
        } while ( compneq(*ptr2, *right) );

        Sym *temp = *left;
        *left = *ptr1;
        *ptr1 = temp;

        if ( compneq(*ptr1, *right) ) {
            ptr1 = ptr1 + 1;
        }

        quicksort(left, ptr0, case_sensitive, by, reverse);
        quicksort(ptr1, right, case_sensitive, by, reverse);
    }

#undef compneq
#undef compgt
#undef complt
}
internal_proc PROC_CALLBACK(proc_dict_dictsort) {
    b32 case_sensitive = val_bool(narg("case_sensitive")->val);
    char *by = val_str(narg("by")->val);
    b32 reverse = val_bool(narg("reverse")->val);

    /* @TEST: umgeschrieben, aber noch nicht getestet */
    /* @AUFGABE: eventuell umbauen um ein pair zurückzugeben. dabei kann auch
     *           einfacher auf case_sensitive eingegangen werden
     */
    Val *val = exec_expr(expr->expr_field.base);
    Sym **syms = 0;
    {
        Scope *scope = val->scope;
        for ( int i = 0; i < scope->num_syms; ++i ) {
            buf_push(syms, scope->sym_list[i]);
        }

        quicksort(syms, syms + buf_len(syms)-1, case_sensitive, by, reverse);
    }

    Scope *scope = scope_new(0, "dictsort_dict");
    Scope *prev_scope = scope_set(scope);
    for ( int i = 0; i < buf_len(syms); ++i ) {
        Sym *sym = syms[i];
        sym_push_var(sym->name, sym->type, sym->val);
    }
    scope_set(prev_scope);

    return val_dict(scope);
}
/* }}} */
/* @INFO: float methoden {{{ */
internal_proc PROC_CALLBACK(proc_float_round) {
    s32 precision = val_int(narg("precision")->val);
    char *method  = val_str(narg("method")->val);
    f32 value = val_float(exec_expr(expr->expr_field.base));

    f32 result = 0;
    if ( method == intern_str("common") ) {
        result = round(value);
    } else if ( method == intern_str("ceil") ) {
        result = ceil(value);
    } else {
        result = floor(value);
    }

    return val_float(result);
}
/* }}} */
/* @INFO: int methoden {{{ */
internal_proc PROC_CALLBACK(proc_int_abs) {
    Resolved_Expr *base = expr->expr_field.base;

    s32 val = val_int(exec_expr(base));
    s32 i = abs(val);

    return val_int(i);
}

internal_proc PROC_CALLBACK(proc_int_filesizeformat) {
    char *suff_bin[] = {
        "Bytes", "kiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"
    };

    char *suff_dec[] = {
        "Bytes", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"
    };

    b32 binary = val_bool(narg("binary")->val);
    int c = (binary) ? 1024 : 1000;
    char **suff = (binary) ? suff_bin : suff_dec;

    Val *val = exec_expr(expr->expr_field.base);
    s64 size = val_int(val);
    s32 mag = (s32)(log(size)/log(c));
    f32 calc_size = ( mag ) ? ((f32)size/(mag*c)) : (f32)size;

    char *result = strf("%.3f %s", calc_size, suff[mag]);

    return val_str(result);
}
/* }}} */
/* @INFO: list methoden {{{ */
internal_proc PROC_CALLBACK(proc_list_append) {
    assert(expr->kind == EXPR_FIELD);
    assert(expr->expr_field.base->kind == EXPR_NAME);

    Val *val = exec_expr(expr->expr_field.base);
    Val *elem = narg("elem")->val;

    Val **vals = 0;
    for ( int i = 0; i < val->len; ++i ) {
        buf_push(vals, ((Val **)val->ptr)[i]);
    }

    buf_push(vals, elem);
    exec_stmt_set(val, val_list(vals, buf_len(vals)));

    return 0;
}

internal_proc PROC_CALLBACK(proc_list_batch) {
    s32 line_count = val_int(narg("line_count")->val);
    Val *fill_with = narg("fill_with")->val;

    Val *val = BASE(expr);
    s32 it_count = (s32)ceil((f32)val->len / line_count);

    Val **result = 0;
    for ( int i = 0; i < it_count; ++i ) {
        Val **vals = 0;

        for ( int j = 0; j < line_count; ++j ) {
            s32 idx = i*line_count + j;
            if ( idx >= val->len ) {
                buf_push(vals, fill_with);
            } else {
                buf_push(vals, val_elem(val, i*line_count + j));
            }
        }

        buf_push(result, val_list(vals, buf_len(vals)));
    }

    return val_list(result, buf_len(result));
}
/* }}} */
/* @INFO: string methoden {{{ */
internal_proc PROC_CALLBACK(proc_string_capitalize) {
    Resolved_Expr *base = expr->expr_field.base;

    char *val = val_str(exec_expr(base));
    char *cap = utf8_char_toupper(val);
    size_t cap_size = utf8_char_size(cap);
    char *remainder = val + cap_size;

    char *result = strf("%.*s%s", cap_size, cap, remainder);

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_string_center) {
    int   width    = val_int(narg("width")->val);
    char *fillchar = val_str(narg("fillchar")->val);

    Val *val = exec_expr(expr->expr_field.base);
    if ( val->len >= width ) {
        return val;
    }

    s32 padding = (s32)((width - val->len) / 2);

    char *result = "";
    for ( int i = 0; i < padding; ++i ) {
        result = strf("%s%s", result, fillchar);
    }
    result = strf("%s%s", result, val_print(val));
    for ( int i = 0; i < padding; ++i ) {
        result = strf("%s%s", result, fillchar);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_string_escape) {
    char *result = "";
    Val *val = exec_expr(expr->expr_field.base);
    char *ptr = (char *)val->ptr;

    for ( int i = 0; i < val->len; ++i ) {
        erstes_if ( *ptr == '<' ) {
            result = strf("%s&lt;", result);
        } else if ( *ptr == '>' ) {
            result = strf("%s&gt;", result);
        } else if ( *ptr == '&' ) {
            result = strf("%s&amp;", result);
        } else if ( *ptr == '\'' ) {
            result = strf("%s&#39;", result);
        } else if ( *ptr == '"' ) {
            result = strf("%s&#34;", result);
        } else {
            result = strf("%s%.*s", result, utf8_char_size(ptr), ptr);
        }

        ptr += utf8_char_size(ptr);
    }

    return val_str(result);
}


internal_proc PROC_CALLBACK(proc_string_float) {
    char *end = 0;
    Val *val = exec_expr(expr->expr_field.base);
    f32 f = strtof(val_str(val), &end);
    Val *result = (end == val->ptr) ? narg("default")->val : val_float(f);

    return result;
}

internal_proc PROC_CALLBACK(proc_string_format) {
    Resolved_Expr *base = expr->expr_field.base;

    char *format = val_str(exec_expr(base));
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
                        fatal(arg->pos.name, arg->pos.line, "der datentyp des übergebenen arguments %s muss string sein", arg->name);
                    }

                    result = strf("%s%s", result, val_print(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.line, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
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
                        fatal(arg->pos.name, arg->pos.line, "der datentyp des übergebenen arguments %s muss int sein", arg->name);
                    }

                    result = strf("%s%s", result, val_print(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.line, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
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
                        fatal(arg->pos.name, arg->pos.line, "der datentyp des übergebenen arguments %s muss float sein", arg->name);
                    }

                    result = strf("%s%.*f", result, size, val_float(arg->val));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg->pos.name, arg->pos.line, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
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

internal_proc PROC_CALLBACK(proc_string_indent) {
    s32 width = val_int(narg("width")->val);
    b32 first = val_bool(narg("first")->val);
    b32 blank = val_bool(narg("blank")->val);

    Val *val = exec_expr(expr->expr_field.base);
    char *result = "";
    char *ptr = (char *)val->ptr;

    if ( first ) {
        result = strf("%*s", width, "");
    }

    while ( *ptr ) {
        result = strf("%s%c", result, *ptr);

        if ( *ptr == '\n' ) {
            char *newline = ptr+1;
            b32 line_is_blank = true;

            while ( *newline && *newline != '\n' ) {
                if ( *newline != ' ' && *newline != '\t' ) {
                    line_is_blank = false;
                    break;
                }
                newline++;
            }

            if ( !line_is_blank || blank ) {
                result = strf("\n%s%*s", result, width, "");
            }
        }

        ptr++;
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_string_int) {
    Val *val = exec_expr(expr->expr_field.base);
    char *end = 0;
    s32 i = strtol(val_str(val), &end, val_int(narg("base")->val));
    Val *result = (end == val->ptr) ? narg("default")->val : val_int(i);

    return result;
}

internal_proc PROC_CALLBACK(proc_string_lower) {
    Val *val = exec_expr(expr->expr_field.base);
    char *str = val_str(val);
    char *result = utf8_str_tolower(str);

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_string_truncate) {
    Val *val = exec_expr(expr->expr_field.base);

    size_t len = MIN(val->len, val_int(narg("length")->val));
    s32 leeway = val_int(narg("leeway")->val);

    u64 diff = abs((s64)(len - val->len));
    if ( diff <= leeway ) {
        return val;
    }

    char *result = "";
    char *ptr = val_str(val);

    b32 killwords = val_bool(narg("killwords")->val);
    if ( !killwords ) {
        char *ptrend = utf8_char_goto(ptr, val->len-1);
        u32 char_count = 0;
        for ( size_t i = val->len-1; i >= len; --i ) {
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

internal_proc PROC_CALLBACK(proc_string_upper) {
    Resolved_Expr *base = expr->expr_field.base;

    char *val = val_str(exec_expr(base));
    char *result = utf8_str_toupper(val);

    return val_str(result);
}
/* }}} */

