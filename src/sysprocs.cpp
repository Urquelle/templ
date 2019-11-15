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
    Scope *scope = (Scope *)operand->user_data;
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
    Sym *sym = sym_get(intern_str("loop"));
    Scope *scope = (Scope *)sym->val->ptr;
    Scope *prev_scope = scope_set(scope);
    Sym *sym_idx = sym_get(intern_str("index"));

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
    Resolved_Arg *arg = narg("sep");

    Joiner *j = ALLOC_STRUCT(&templ_arena, Joiner);
    j->val = arg->val;
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
    return val_str("@IMPLEMENTIEREN: loop()");
}

PROC_CALLBACK(proc_range) {
    Resolved_Arg *arg = narg("start");
    int start = val_int(arg->val);

    arg = narg("stop");
    int stop  = val_int(arg->val);

    arg = narg("step");
    int step = val_int(arg->val);

    return val_range(start, stop, step);
}

global_var char *global_lorem_ipsum = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
PROC_CALLBACK(proc_lipsum) {
    Resolved_Arg *arg = narg("n");
    s32 n = val_int(arg->val);

    arg = narg("html");
    b32 html = val_bool(arg->val);

    arg = narg("min");
    s32 min = val_int(arg->val);

    arg = narg("max");
    s32 max = (s32)MIN(val_int(arg->val), utf8_strlen(global_lorem_ipsum));

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

