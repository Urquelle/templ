PROC_CALLBACK(proc_super) {
    assert(global_super_block);
    assert(global_super_block->kind == STMT_BLOCK);

    char *old_gen_result = gen_result;
    char *temp = "";
    gen_result = temp;

    for ( int i = 0; i < global_super_block->stmt_block.num_stmts; ++i ) {
        exec_stmt(global_super_block->stmt_block.stmts[i], templ);
    }

    Val *result = val_str(gen_result);
    gen_result = old_gen_result;

    return result;
}

PROC_CALLBACK(proc_cycle) {
    s32 loop_index = val_int((Val *)proc_type->user_data);
    s32 arg_index  = loop_index % num_varargs;
    val_inc((Val *)proc_type->user_data);

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

    Map *map = &proc_type->type_proc.ret->type_dict.scope->syms;
    map_put(map, intern_str("current"), varargs[0]->val);

    Val *result = val_dict(0, 0);
    result->user_data = cycler;

    return result;
}

PROC_CALLBACK(proc_cycler_next) {
    /* @INFO: die next methode wird IMMER über <cycler>.next() aufgerufen, und somit
     *        immer im expr_call -> expr_field sein.
     */
    Resolved_Expr *base = expr->expr_call.expr->expr_field.base;
    Cycler *cycler = (Cycler *)base->val->user_data;

    /* @AUFGABE: wird der index vor- oder hinterher erhöht? */
    cycler->idx += 1;

    s32 loop_index = cycler->idx;
    s32 arg_index  = loop_index % cycler->num_args;
    Val *result = cycler->args[arg_index]->val;

    Scope *scope = base->type->type_dict.scope;
    map_put(&scope->syms, intern_str("current"), result);

    return result;
}

PROC_CALLBACK(proc_cycler_reset) {
    Resolved_Expr *base = expr->expr_call.expr->expr_field.base;
    Cycler *cycler = (Cycler *)base->val->user_data;
    cycler->idx = 0;

    Scope *scope = base->type->type_dict.scope;
    map_put(&scope->syms, intern_str("current"), cycler->args[cycler->idx]->val);

    return 0;
}

PROC_CALLBACK(proc_loop) {
    return val_str("");
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
