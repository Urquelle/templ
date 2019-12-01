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

PROC_CALLBACK(proc_exec_macro) {
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

PROC_CALLBACK(proc_dict) {
    Scope *scope = scope_new(0, "dict");
    Scope *prev_scope = scope_set(scope);

    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *arg = kwargs[i];
        sym_push_var(arg->name, arg->type, arg->val);
    }

    scope_set(prev_scope);

    return val_dict(scope);
}

PROC_CALLBACK(proc_cycle) {
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
PROC_CALLBACK(proc_cycler) {
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

PROC_CALLBACK(proc_cycler_next) {
    Cycler *cycler = (Cycler *)operand->user_data;

    cycler->idx = (cycler->idx + 1) % cycler->num_args;

    return cycler->args[cycler->idx]->val;
}

PROC_CALLBACK(proc_cycler_reset) {
    Cycler *cycler = (Cycler *)operand->user_data;
    cycler->idx = 0;

    return 0;
}

struct Joiner {
    int counter;
    Val *val;
};
PROC_CALLBACK(proc_joiner) {
    Joiner *j = ALLOC_STRUCT(&templ_arena, Joiner);
    j->val = narg("sep")->val;
    j->counter = 0;

    Val *result = val_proc(0, 0, type_str, proc_joiner_call);
    result->user_data = j;

    return result;
}

PROC_CALLBACK(proc_joiner_call) {
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

PROC_CALLBACK(proc_loop) {
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

PROC_CALLBACK(proc_range) {
    int start = val_int(narg("start")->val);
    int stop  = val_int(narg("stop")->val);
    int step  = val_int(narg("step")->val);

    return val_range(start, stop, step);
}

global_var char *global_lorem_ipsum = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
PROC_CALLBACK(proc_lipsum) {
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

PROC_CALLBACK(proc_namespace) {
    Scope *scope = scope_new(0, "namespace");
    Scope *prev_scope = scope_set(scope);

    for ( int i = 0; i < num_kwargs; ++i ) {
        Resolved_Arg *arg = kwargs[i];
        sym_push_var(arg->name, arg->type, arg->val);
    }

    scope_set(prev_scope);

    return val_dict(scope);
}

PROC_CALLBACK(proc_list_append) {
    assert(expr->kind == EXPR_FIELD);
    assert(expr->expr_field.base->kind == EXPR_NAME);

    Sym *sym = sym_get(expr->expr_field.base->expr_name.name);
    Val *val = sym->val;
    Val *elem = narg("elem")->val;

    Val **vals = 0;
    for ( int i = 0; i < val->len; ++i ) {
        buf_push(vals, ((Val **)val->ptr)[i]);
    }

    buf_push(vals, elem);
    sym->val = val_list(vals, buf_len(vals));

    return val_none();
}

PROC_CALLBACK(proc_string_capitalize) {
    Resolved_Expr *base = expr->expr_field.base;
    char *val = 0;

    char *format = 0;
    if ( base->kind == EXPR_STR ) {
        val = val_str(base->val);
    } else {
        val = val_str(exec_expr(base));
    }

    char *cap = utf8_char_toupper(val);
    size_t cap_size = utf8_char_size(cap);
    char *remainder = val + cap_size;

    char *result = strf("%.*s%s", cap_size, cap, remainder);

    return val_str(result);
}

PROC_CALLBACK(proc_string_format) {
    Resolved_Expr *base = expr->expr_field.base;
    Val *result = 0;

    char *format = 0;
    if ( base->kind == EXPR_STR ) {
        format = val_str(base->val);
    } else {
        Val *val = exec_expr(base);
        format = val_str(val);
    }

    char *out = algo_format(format, varargs, num_varargs);
    result = val_str(out);

    return result;
}

