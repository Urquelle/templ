internal_proc void
quicksort(Val **left, Val **right, b32 reverse, b32 case_sensitive, char *attr = 0) {
#define compneq(left, right) ((attr) ? 0 : (*left != *right))
#define compgt(left, right)  ((attr) ? 0 : (*left  > *right))
#define complt(left, right)  ((attr) ? 0 : (*left  < *right))

    if ( compneq(*left, *right) ) {
        Val **ptr0 = left;
        Val **ptr1 = left;
        Val **ptr2 = left;

        Val *pivot = *left;

        do {
            ptr2 = ptr2 + 1;
            b32 check = ( reverse ) ? ( compgt(*ptr2, pivot) ) : ( complt(*ptr2, pivot) );

            if ( check ) {
                ptr0 = ptr1;
                ptr1 = ptr1 + 1;

                Val *temp = *ptr1;
                *ptr1 = *ptr2;
                *ptr2 = temp;
            }
        } while ( compneq(*ptr2, *right) );

        Val *temp = *left;
        *left = *ptr1;
        *ptr1 = temp;

        if ( compneq(*ptr1, *right) ) {
            ptr1 = ptr1 + 1;
        }

        quicksort(left, ptr0, reverse, case_sensitive);
        quicksort(ptr1, right, reverse, case_sensitive);
    }

#undef compneq
#undef compgt
#undef complt
}

internal_proc void
quicksort(Sym **left, Sym **right, b32 case_sensitive, char *by, b32 reverse) {
    char *key = intern_str("key");

#define compneq(left, right) ((by == key) ? (utf8_strcmp(sym_name(left), sym_name(right)) != 0) : (*sym_val(left) != *(sym_val(right))))
#define compgt(left, right)  ((by == key) ? (utf8_strcmp(sym_name(left), sym_name(right))  > 0) : (*sym_val(left)  > *(sym_val(right))))
#define complt(left, right)  ((by == key) ? (utf8_strcmp(sym_name(left), sym_name(right))  < 0) : (*sym_val(left)  < *(sym_val(right))))

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
/* @INFO: globale methoden {{{ */
internal_proc PROC_CALLBACK(proc_super) {
    assert(global_super_block);
    assert(stmt_kind(global_super_block) == STMT_BLOCK);

    char *old_gen_result = gen_result;
    char *temp = "";
    gen_result = temp;

    Resolved_Stmt **stmts = stmt_block_stmts(global_super_block);
    for ( int i = 0; i < stmt_block_num_stmts(global_super_block); ++i ) {
        exec_stmt(stmts[i]);
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
        sym_val(sym, arg_val(arg));
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
        sym_push_var(arg_name(arg), arg_type(arg), arg_val(arg));
    }

    scope_set(prev_scope);

    return val_dict(scope);
}

internal_proc PROC_CALLBACK(proc_cycle) {
    Sym *sym = sym_get(symname_loop);
    Scope *scope = sym_val(sym)->scope;
    Scope *prev_scope = scope_set(scope);
    Sym *sym_idx = sym_get(symname_index);

    s32 loop_index = val_int(sym_val(sym_idx));
    s32 arg_index  = loop_index % num_varargs;
    val_inc(sym_val(sym_idx));

    scope_set(prev_scope);

    return arg_val(varargs[arg_index]);
}

struct Cycler {
    size_t num_args;
    Resolved_Arg **args;
    s32 idx;
};
internal_proc PROC_CALLBACK(proc_cycler_next) {
    Cycler *cycler = (Cycler *)operand->user_data;

    cycler->idx = (cycler->idx + 1) % cycler->num_args;

    return arg_val(cycler->args[cycler->idx]);
}

internal_proc PROC_CALLBACK(proc_cycler_reset) {
    Cycler *cycler = (Cycler *)operand->user_data;
    cycler->idx = 0;

    return 0;
}

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
    sym_push_var("current",  type_any, arg_val(varargs[0]));

    scope_set(prev_scope);

    Val *result = val_dict(cycler_scope);
    result->user_data = cycler;

    return result;
}

struct Joiner {
    int counter;
    Val *val;
};
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

internal_proc PROC_CALLBACK(proc_joiner) {
    Joiner *j = ALLOC_STRUCT(&templ_arena, Joiner);
    j->val = arg_val(narg("sep"));
    j->counter = 0;

    Val *result = val_proc(0, 0, type_str, proc_joiner_call);
    result->user_data = j;

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

        depth  = val_int(sym_val(sym_depth));
        depth0 = val_int(sym_val(sym_depth0));

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

    Resolved_Stmt *s = global_for_stmt;
    Resolved_Stmt *stmt = resolved_stmt_for(scope_for,
            stmt_for_vars(s), stmt_for_num_vars(s),
            set,
            stmt_for_stmts(s), stmt_for_num_stmts(s),
            stmt_for_else_stmts(s), stmt_for_num_else_stmts(s),
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
    int start = val_int(arg_val(narg("start")));
    int stop  = val_int(arg_val(narg("stop")));
    int step  = val_int(arg_val(narg("step")));

    return val_range(start, stop, step);
}

global_var char *global_lorem_ipsum = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
internal_proc PROC_CALLBACK(proc_lipsum) {
    s32 n    = val_int(arg_val(narg("n")));
    b32 html = val_bool(arg_val(narg("html")));
    s32 min  = val_int(arg_val(narg("min")));
    s32 max  = (s32)MIN(val_int(arg_val(narg("max"))), utf8_strlen(global_lorem_ipsum));

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
        sym_push_var(arg_name(arg), arg_type(arg), arg_val(arg));
    }

    scope_set(prev_scope);

    return val_dict(scope);
}
/* }}} */
/* @INFO: any methoden {{{ */
internal_proc PROC_CALLBACK(proc_any_default) {
    char *default_value = val_str(arg_val(narg("s")));
    b32 boolean = val_bool(arg_val(narg("boolean")));

    if ( val_is_undefined(value) || boolean && !val_is_true(value) ) {
        return val_str(default_value);
    }

    return value;
}

internal_proc PROC_CALLBACK(proc_any_list) {
    if ( value->kind == VAL_LIST ) {
        return value;
    }

    Val **vals = 0;
    for ( int i = 0; i < value->len; ++i ) {
        Val *v = val_elem(value, i);
        buf_push(vals, v);
    }

    return val_list(vals, buf_len(vals));
}

internal_proc PROC_CALLBACK(proc_any_max) {
    if ( value->len == 1 ) {
        return value;
    }

    b32 case_sensitive = val_bool(arg_val(narg("case_sensitive")));
    Val *attribute = arg_val(narg("attribute"));

    Val *result = val_elem(value, 0);
    for ( int i = 1; i < value->len; ++i ) {
        Val *right = val_elem(value, i);

        Val *left_tmp = result;
        Val *right_tmp = right;

        if ( attribute->kind != VAL_NONE ) {
            assert(result->kind == VAL_DICT);
            assert(right->kind == VAL_DICT);

            left_tmp  = sym_val(scope_attr(result->scope, val_str(attribute)));
            right_tmp = sym_val(scope_attr(right->scope, val_str(attribute)));
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
    if ( value->len == 1 ) {
        return value;
    }

    b32 case_sensitive = val_bool(arg_val(narg("case_sensitive")));
    Val *attribute = arg_val(narg("attribute"));

    Val *result = val_elem(value, 0);
    for ( int i = 1; i < value->len; ++i ) {
        Val *right  = val_elem(value, i);

        Val *left_tmp  = result;
        Val *right_tmp = right;

        if ( attribute->kind != VAL_NONE ) {
            assert(result->kind == VAL_DICT);
            assert(right->kind == VAL_DICT);

            left_tmp  = sym_val(scope_attr(result->scope, val_str(attribute)));
            right_tmp = sym_val(scope_attr(right->scope, val_str(attribute)));
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
    b32 verbose = val_bool(arg_val(narg("verbose")));
    char *result = val_pprint(value, verbose);

    return val_str(result);
}
/* }}} */
/* @INFO: sequence methoden {{{ */
internal_proc PROC_CALLBACK(proc_seq_batch) {
    s32 line_count = val_int(arg_val(narg("line_count")));
    Val *fill_with = arg_val(narg("fill_with"));

    s32 it_count = (s32)ceil((f32)value->len / line_count);

    Val **result = 0;
    for ( int i = 0; i < it_count; ++i ) {
        Val **vals = 0;

        for ( int j = 0; j < line_count; ++j ) {
            s32 idx = i*line_count + j;
            if ( idx >= value->len ) {
                buf_push(vals, fill_with);
            } else {
                buf_push(vals, val_elem(value, i*line_count + j));
            }
        }

        buf_push(result, val_list(vals, buf_len(vals)));
    }

    return val_list(result, buf_len(result));
}

internal_proc PROC_CALLBACK(proc_seq_groupby) {
    char *attribute = val_str(arg_val(narg("attribute")));

    Scope *prev_scope = current_scope;
    Map list = {};
    char **keys = 0;

    for ( int i = 0; i < value->len; ++i ) {
        Val *v = val_elem(value, i);
        assert(v->kind == VAL_DICT);

        scope_set(v->scope);
        Sym *sym = sym_get(attribute);

        char *key = intern_str(val_print(sym_val(sym)));

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

internal_proc PROC_CALLBACK(proc_seq_first) {
    Val *result = val_elem(value, 0);

    return result;
}

internal_proc PROC_CALLBACK(proc_seq_join) {
    char *sep = val_str(arg_val(narg("d")));
    Val *attr = arg_val(narg("attribute"));
    char *result = "";

    if (value->len < 2) {
        return value;
    }

    Val *v = val_elem(value, 0);
    if ( attr->kind != VAL_NONE ) {
        Sym *sym = scope_attr(v->scope, val_str(attr));
        v = sym_val(sym);
    }
    result = strf("%s", val_print(v));

    for ( int i = 1; i < value->len; ++i ) {
        v = val_elem(value, i);
        if ( attr->kind != VAL_NONE ) {
            Sym *sym = scope_attr(v->scope, val_str(attr));
            v = sym_val(sym);
        }
        result = strf("%s%s%s", result, sep, val_print(v));
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_seq_last) {
    Val *result = val_elem(value, (int)value->len-1);

    return result;
}

internal_proc PROC_CALLBACK(proc_seq_length) {
    Val *result = val_int((int)value->len);

    return result;
}

internal_proc PROC_CALLBACK(proc_seq_map) {
    char *attribute = 0;
    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *kwarg = kwargs[i];
        if ( arg_name(kwarg) == intern_str("attribute") ) {
            attribute = val_str(arg_val(kwarg));
        }
    }

    Scope *prev_scope = current_scope;
    Val **vals = 0;
    if ( attribute ) {
        for ( int i = 0; i < value->len; ++i ) {
            Val *elem = val_elem(value, i);
            scope_set(elem->scope);
            Sym *sym = sym_get(attribute);

            if ( sym ) {
                buf_push(vals, sym_val(sym));
            }
        }
    }

    scope_set(prev_scope);
    return val_list(vals, buf_len(vals));
}

internal_proc PROC_CALLBACK(proc_seq_random) {
    s32 idx = rand() % value->len;
    Val *result = val_elem(value, idx);

    return result;
}

internal_proc PROC_CALLBACK(proc_seq_replace) {
    char *str = val_str(value);
    char  *old_str = val_str(arg_val(narg("old")));
    size_t old_size = utf8_str_size(old_str);
    char  *new_str = val_str(arg_val(narg("new")));
    size_t new_size = utf8_str_size(new_str);
    Resolved_Arg *count_arg = narg("count");
    s32 count = ( arg_val(count_arg)->kind == VAL_NONE ) ? -1 : val_int(arg_val(count_arg));

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

internal_proc PROC_CALLBACK(proc_seq_reject) {
    assert(num_varargs > 0);
    Sym *sym = scope_attr(&tester_scope, val_str(arg_val(varargs[0])));
    assert(sym_val(sym)->kind == VAL_PROC);
    Val_Proc *proc = (Val_Proc *)sym_val(sym)->ptr;

    Resolved_Arg **tester_varargs = (Resolved_Arg **)varargs + 1;
    Resolved_Expr **tester_args = (Resolved_Expr **)args + 1;
    Map tester_nargs = {};
    char **tester_narg_keys = 0;

    /* @INFO: args in nargs speichern bei übereinstimmung. das erste argument
     *        wird übersprungen, da es sich um den namen des testers handelt.
     */
    for ( int i = 0; i < num_args-1; ++i ) {
        Resolved_Expr *arg = tester_args[i];
        if ( sym_type(sym)->type_proc.num_params > i ) {
            Type_Field *param = sym_type(sym)->type_proc.params[i];
            map_put(&tester_nargs, param->name,
                    resolved_arg(expr_pos(arg), param->name, expr_type(arg), expr_val(arg)));
        }
    }

    /* @INFO: kwargs in nargs speichern bei übereinstimmung */
    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *kwarg = kwargs[i];
        for ( int j = 0; j < sym_type(sym)->type_proc.num_params; ++j ) {
            Type_Field *param = sym_type(sym)->type_proc.params[j];

            if ( arg_name(kwarg) == param->name ) {
                map_put(&tester_nargs, param->name, kwarg);
            }
        }
    }

    Val **result = 0;
    for ( int i = 0; i < value->len; ++i ) {
        Val *elem = val_elem(value, i);

        Val *ret = proc->callback(
                elem, value, args, num_args, &tester_nargs,
                tester_narg_keys, buf_len(tester_narg_keys), kwargs, num_kwargs,
                tester_varargs, num_varargs-1);

        if ( !val_bool(ret) ) {
            buf_push(result, elem);
        }
    }

    return val_list(result, buf_len(result));
}

internal_proc PROC_CALLBACK(proc_seq_rejectattr) {
    if ( num_varargs == 1 ) {
        char *attr = val_str(arg_val(varargs[0]));
        Val **vals = 0;
        for ( int i = 0; i < value->len; ++i ) {
            Val *v = val_elem(value, i);
            Sym *sym = scope_attr(v->scope, attr);

            if ( !val_bool(sym_val(sym)) ) {
                buf_push(vals, v);
            }
        }

        return val_list(vals, buf_len(vals));
    } else if ( num_varargs > 1 ) {
        Sym *sym = scope_attr(&tester_scope, val_str(arg_val(varargs[1])));
        assert(sym_val(sym)->kind == VAL_PROC);
        Val_Proc *proc = (Val_Proc *)sym_val(sym)->ptr;

        Resolved_Arg **tester_varargs = (Resolved_Arg **)varargs + 2;
        Resolved_Expr **tester_args = (Resolved_Expr **)args + 2;
        Map tester_nargs = {};
        char **tester_narg_keys = 0;

        /* @INFO: args in nargs speichern bei übereinstimmung. das erste argument
        *        wird übersprungen, da es sich um den namen des testers handelt.
        */
        for ( int i = 0; i < num_args-2; ++i ) {
            Resolved_Expr *arg = tester_args[i];
            if ( sym_type(sym)->type_proc.num_params > i ) {
                Type_Field *param = sym_type(sym)->type_proc.params[i];
                map_put(&tester_nargs, param->name,
                        resolved_arg(expr_pos(arg), param->name, expr_type(arg),
                            expr_val(arg)));
            }
        }

        /* @INFO: kwargs in nargs speichern bei übereinstimmung */
        for ( int i = 0; i < num_kwargs; ++i ) {
            Resolved_Arg *kwarg = kwargs[i];
            for ( int j = 0; j < sym_type(sym)->type_proc.num_params; ++j ) {
                Type_Field *param = sym_type(sym)->type_proc.params[j];

                if ( arg_name(kwarg) == param->name ) {
                    map_put(&tester_nargs, param->name, kwarg);
                }
            }
        }

        char *attr = val_str(arg_val(varargs[0]));
        Val **result = 0;
        for ( int i = 0; i < value->len; ++i ) {
            Val *elem = val_elem(value, i);
            Sym *elem_sym = scope_attr(elem->scope, attr);

            Val *ret = proc->callback(
                    sym_val(elem_sym), value, args, num_args, &tester_nargs,
                    tester_narg_keys, buf_len(tester_narg_keys), kwargs, num_kwargs,
                    tester_varargs, num_varargs-1);

            if ( !val_bool(ret) ) {
                buf_push(result, elem);
            }
        }

        return val_list(result, buf_len(result));
    }

    return val_str("");
}

internal_proc PROC_CALLBACK(proc_seq_reverse) {
    if ( value->len == 1 ) {
        return value;
    }

    Val *result = 0;
    if ( value->kind == VAL_STR ) {
        char *str = "";
        int i = 1;
        size_t size = utf8_str_size((char *)value->ptr);
        while ( i <= size ) {
            int offset = (int)size - i;
            char c = ((char *)value->ptr)[offset];
            str = strf("%s%c", str, c);
            i++;
        }

        result = val_str(str);
    } else {
        Val **vals = 0;
        int i = 1;
        while ( i <= value->len ) {
            Val *v = val_elem(value, (int)value->len - i);
            buf_push(vals, v);
            i++;
        }

        result = val_list(vals, buf_len(vals));
    }

    return result;
}

internal_proc PROC_CALLBACK(proc_seq_select) {
    assert(num_varargs > 0);
    Sym *sym = scope_attr(&tester_scope, val_str(arg_val(varargs[0])));
    assert(sym_val(sym)->kind == VAL_PROC);
    Val_Proc *proc = (Val_Proc *)sym_val(sym)->ptr;

    Resolved_Arg **tester_varargs = (Resolved_Arg **)varargs + 1;
    Resolved_Expr **tester_args = (Resolved_Expr **)args + 1;
    Map tester_nargs = {};
    char **tester_narg_keys = 0;

    /* @INFO: args in nargs speichern bei übereinstimmung. das erste argument
     *        wird übersprungen, da es sich um den namen des testers handelt.
     */
    for ( int i = 0; i < num_args-1; ++i ) {
        Resolved_Expr *arg = tester_args[i];
        if ( sym_type(sym)->type_proc.num_params > i ) {
            Type_Field *param = sym_type(sym)->type_proc.params[i];
            map_put(&tester_nargs, param->name,
                    resolved_arg(expr_pos(arg), param->name, expr_type(arg),
                        expr_val(arg)));
        }
    }

    /* @INFO: kwargs in nargs speichern bei übereinstimmung */
    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *kwarg = kwargs[i];
        for ( int j = 0; j < sym_type(sym)->type_proc.num_params; ++j ) {
            Type_Field *param = sym_type(sym)->type_proc.params[j];

            if ( arg_name(kwarg) == param->name ) {
                map_put(&tester_nargs, param->name, kwarg);
            }
        }
    }

    Val **result = 0;
    for ( int i = 0; i < value->len; ++i ) {
        Val *elem = val_elem(value, i);

        Val *ret = proc->callback(
                elem, value, args, num_args, &tester_nargs,
                tester_narg_keys, buf_len(tester_narg_keys), kwargs, num_kwargs,
                tester_varargs, num_varargs-1);

        if ( val_bool(ret) ) {
            buf_push(result, elem);
        }
    }

    return val_list(result, buf_len(result));
}

internal_proc PROC_CALLBACK(proc_seq_selectattr) {
    if ( num_varargs == 1 ) {
        char *attr = val_str(arg_val(varargs[0]));
        Val **vals = 0;
        for ( int i = 0; i < value->len; ++i ) {
            Val *v = val_elem(value, i);
            Sym *sym = scope_attr(v->scope, attr);

            if ( val_bool(sym_val(sym)) ) {
                buf_push(vals, v);
            }
        }

        return val_list(vals, buf_len(vals));
    } else if ( num_varargs > 1 ) {
        Sym *sym = scope_attr(&tester_scope, val_str(arg_val(varargs[1])));
        assert(sym_val(sym)->kind == VAL_PROC);
        Val_Proc *proc = (Val_Proc *)sym_val(sym)->ptr;

        Resolved_Arg **tester_varargs = (Resolved_Arg **)varargs + 2;
        Resolved_Expr **tester_args = (Resolved_Expr **)args + 2;
        Map tester_nargs = {};
        char **tester_narg_keys = 0;

        /* @INFO: args in nargs speichern bei übereinstimmung. das erste argument
        *        wird übersprungen, da es sich um den namen des testers handelt.
        */
        for ( int i = 0; i < num_args-2; ++i ) {
            Resolved_Expr *arg = tester_args[i];
            if ( sym_type(sym)->type_proc.num_params > i ) {
                Type_Field *param = sym_type(sym)->type_proc.params[i];
                map_put(&tester_nargs, param->name,
                        resolved_arg(expr_pos(arg), param->name, expr_type(arg),
                            expr_val(arg)));
            }
        }

        /* @INFO: kwargs in nargs speichern bei übereinstimmung */
        for ( int i = 0; i < num_kwargs; ++i ) {
            Resolved_Arg *kwarg = kwargs[i];
            for ( int j = 0; j < sym_type(sym)->type_proc.num_params; ++j ) {
                Type_Field *param = sym_type(sym)->type_proc.params[j];

                if ( arg_name(kwarg) == param->name ) {
                    map_put(&tester_nargs, param->name, kwarg);
                }
            }
        }

        char *attr = val_str(arg_val(varargs[0]));
        Val **result = 0;
        for ( int i = 0; i < value->len; ++i ) {
            Val *elem = val_elem(value, i);
            Sym *elem_sym = scope_attr(elem->scope, attr);

            Val *ret = proc->callback(
                    sym_val(elem_sym), value, args, num_args, &tester_nargs,
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

internal_proc PROC_CALLBACK(proc_seq_sort) {
    Val *result = val_copy(value);

    b32 reverse = val_bool(arg_val(narg("reverse")));
    b32 case_sensitive = val_bool(arg_val(narg("case_sensitive")));
    Val *attr = arg_val(narg("attribute"));

    if ( value->kind == VAL_STR ) {
    } else {
        quicksort((Val **)result->ptr, (Val **)result->ptr + result->len-1, reverse, case_sensitive);
    }

    return result;
}
/* }}} */
/* @INFO: dict methoden {{{ */
internal_proc PROC_CALLBACK(proc_dict_attr) {
    Scope *scope = value->scope;
    Scope *prev_scope = scope_set(scope);

    Val *name = arg_val(narg("name"));
    Sym *sym = sym_get(val_str(name));
    Val *result = sym_val(sym);

    scope_set(prev_scope);

    return result;
}

internal_proc PROC_CALLBACK(proc_dict_dictsort) {
    b32 case_sensitive = val_bool(arg_val(narg("case_sensitive")));
    char *by = val_str(arg_val(narg("by")));
    b32 reverse = val_bool(arg_val(narg("reverse")));

    /* @AUFGABE: eventuell umbauen um ein pair zurückzugeben. dabei kann auch
     *           einfacher auf case_sensitive eingegangen werden
     */
    Sym **syms = 0;
    {
        Scope *scope = value->scope;
        for ( int i = 0; i < scope->num_syms; ++i ) {
            buf_push(syms, scope->sym_list[i]);
        }

        quicksort(syms, syms + buf_len(syms)-1, case_sensitive, by, reverse);
    }

    Scope *scope = scope_new(0, "dictsort_dict");
    Scope *prev_scope = scope_set(scope);
    for ( int i = 0; i < buf_len(syms); ++i ) {
        Sym *sym = syms[i];
        sym_push_var(sym_name(sym), sym_type(sym), sym_val(sym));
    }
    scope_set(prev_scope);

    return val_dict(scope);
}
/* }}} */
/* @INFO: float methoden {{{ */
internal_proc PROC_CALLBACK(proc_float_round) {
    s32 precision = val_int(arg_val(narg("precision")));
    char *method  = val_str(arg_val(narg("method")));
    f32 val = val_float(value);

    f32 result = 0;
    if ( method == intern_str("common") ) {
        result = round(val);
    } else if ( method == intern_str("ceil") ) {
        result = ceil(val);
    } else {
        result = floor(val);
    }

    return val_float(result);
}
/* }}} */
/* @INFO: int methoden {{{ */
internal_proc PROC_CALLBACK(proc_int_abs) {
    s32 i = abs(val_int(value));

    return val_int(i);
}

internal_proc PROC_CALLBACK(proc_int_filesizeformat) {
    char *suff_bin[] = {
        "Bytes", "kiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"
    };

    char *suff_dec[] = {
        "Bytes", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"
    };

    b32 binary = val_bool(arg_val(narg("binary")));
    int c = (binary) ? 1024 : 1000;
    char **suff = (binary) ? suff_bin : suff_dec;

    s64 size = val_int(value);
    s32 mag = (s32)(log(size)/log(c));
    f32 calc_size = ( mag ) ? ((f32)size/(mag*c)) : (f32)size;

    char *result = strf("%.3f %s", calc_size, suff[mag]);

    return val_str(result);
}
/* }}} */
/* @INFO: list methoden {{{ */
internal_proc PROC_CALLBACK(proc_list_append) {
    Val *elem = arg_val(narg("elem"));

    Val **vals = 0;
    for ( int i = 0; i < value->len; ++i ) {
        buf_push(vals, ((Val **)value->ptr)[i]);
    }

    buf_push(vals, elem);
    exec_stmt_set(value, val_list(vals, buf_len(vals)));

    return 0;
}

internal_proc PROC_CALLBACK(proc_list_slice) {
    s32 line_count = val_int(arg_val(narg("line_count")));
    Val *fill_with = arg_val(narg("fill_with"));

    s32 it_count = (s32)ceil((f32)value->len / line_count);
    Val **result = 0;
    for ( int i = 0; i < line_count; ++i ) {
        Val **vals = 0;

        for ( int j = 0; j < it_count; ++j ) {
            s32 idx = i + line_count*j;
            if ( idx >= value->len ) {
                buf_push(vals, fill_with);
            } else {
                buf_push(vals, val_elem(value, i + line_count*j));
            }
        }

        buf_push(result, val_list(vals, buf_len(vals)));
    }

    return val_list(result, buf_len(result));
}

internal_proc PROC_CALLBACK(proc_list_sum) {
    Val *attr = arg_val(narg("attribute"));
    s32 start = val_int(arg_val(narg("start")));

    s32 result = 0;

    for ( int i = start; i < value->len; ++i ) {
        Val *elem = val_elem(value, i);

        if ( attr->kind != VAL_NONE ) {
            Sym *sym = scope_attr(elem->scope, val_str(attr));
            elem = sym_val(sym);
        }

        result += val_int(elem);
    }

    return val_int(result);
}
/* }}} */
/* @INFO: string methoden {{{ */
internal_proc PROC_CALLBACK(proc_string_capitalize) {
    char *val = val_str(value);
    char *cap = utf8_char_toupper(val);
    size_t cap_size = utf8_char_size(cap);
    char *remainder = val + cap_size;

    char *result = strf("%.*s%s", cap_size, cap, remainder);

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_string_center) {
    int   width    = val_int(arg_val(narg("width")));
    char *fillchar = val_str(arg_val(narg("fillchar")));

    if ( value->len >= width ) {
        return value;
    }

    s32 padding = (s32)((width - value->len) / 2);

    char *result = "";
    for ( int i = 0; i < padding; ++i ) {
        result = strf("%s%s", result, fillchar);
    }
    result = strf("%s%s", result, val_print(value));
    for ( int i = 0; i < padding; ++i ) {
        result = strf("%s%s", result, fillchar);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_string_escape) {
    char *result = "";
    char *ptr = (char *)value->ptr;

    for ( int i = 0; i < value->len; ++i ) {
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
    f32 f = strtof(val_str(value), &end);
    Val *result = (end == value->ptr) ? arg_val(narg("default")) : val_float(f);

    return result;
}

internal_proc PROC_CALLBACK(proc_string_format) {
    char *format = val_str(value);
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

                    if ( arg_val(arg)->kind != VAL_STR ) {
                        fatal(arg_pos(arg).name, arg_pos(arg).line, "der datentyp des übergebenen arguments %s muss string sein", arg_name(arg));
                    }

                    result = strf("%s%s", result, val_print(arg_val(arg)));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg_pos(arg).name, arg_pos(arg).line, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else if ( format[0] == 'd' ) {
                if ( num_arg < num_varargs ) {
                    Resolved_Arg *arg = varargs[num_arg];

                    if ( arg_val(arg)->kind != VAL_INT ) {
                        fatal(arg_pos(arg).name, arg_pos(arg).line, "der datentyp des übergebenen arguments %s muss int sein", arg_name(arg));
                    }

                    result = strf("%s%s", result, val_print(arg_val(arg)));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg_pos(arg).name, arg_pos(arg).line, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
                    } else {
                        warn(0, 0, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente");
                    }
                    break;
                }

                num_arg++;
            } else if ( format[0] == 'f' ) {
                if ( num_arg < num_varargs ) {
                    Resolved_Arg *arg = varargs[num_arg];

                    if ( arg_val(arg)->kind != VAL_FLOAT ) {
                        fatal(arg_pos(arg).name, arg_pos(arg).line, "der datentyp des übergebenen arguments %s muss float sein", arg_name(arg));
                    }

                    result = strf("%s%.*f", result, size, val_float(arg_val(arg)));
                } else {
                    if ( num_varargs ) {
                        Resolved_Arg *arg = varargs[0];
                        warn(arg_pos(arg).name, arg_pos(arg).line, "anzahl formatierungsparameter ist größer als die anzahl der übergebenen argumente: %d", num_varargs);
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
    s32 width = val_int(arg_val(narg("width")));
    b32 first = val_bool(arg_val(narg("first")));
    b32 blank = val_bool(arg_val(narg("blank")));

    char *result = "";
    char *ptr = (char *)value->ptr;

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
    char *end = 0;
    s32 i = strtol(val_str(value), &end, val_int(arg_val(narg("base"))));
    Val *result = (end == value->ptr) ? arg_val(narg("default")) : val_int(i);

    return result;
}

internal_proc PROC_CALLBACK(proc_string_lower) {
    char *str = val_str(value);
    char *result = utf8_str_tolower(str);

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_string_truncate) {
    size_t len = MIN(value->len, val_int(arg_val(narg("length"))));
    s32 leeway = val_int(arg_val(narg("leeway")));

    u64 diff = abs((s64)(len - value->len));
    if ( diff <= leeway ) {
        return value;
    }

    char *result = "";
    char *ptr = val_str(value);

    b32 killwords = val_bool(arg_val(narg("killwords")));
    if ( !killwords ) {
        char *ptrend = utf8_char_goto(ptr, value->len-1);
        u32 char_count = 0;
        for ( size_t i = value->len-1; i >= len; --i ) {
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

    char *end = val_str(arg_val(narg("end")));
    size_t end_len = utf8_strlen(end);
    for ( int i = 0; i < end_len; ++i ) {
        result = strf("%s%.*s", result, utf8_char_size(end), end);
        end += utf8_char_size(end);
    }

    return val_str(result);
}

internal_proc PROC_CALLBACK(proc_string_upper) {
    char *result = utf8_str_toupper(val_str(value));

    return val_str(result);
}
/* }}} */

