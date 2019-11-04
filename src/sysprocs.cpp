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
