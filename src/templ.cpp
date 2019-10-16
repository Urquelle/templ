struct Templ_Vars {
    char **keys;
    Map map;
};

internal_proc Templ_Vars *
templ_vars_new() {
    Templ_Vars *result = (Templ_Vars *)xcalloc(1, sizeof(Templ_Vars));

    result->keys = 0;

    return result;
}

internal_proc void
templ_vars_add(Templ_Vars *vars, char *key, char *value) {
    map_put(&vars->map, key, value);
    buf_push(vars->keys, key);
}

internal_proc char *
templ_vars_get(Templ_Vars *vars, char *key) {
    return (vars) ? (char *)map_get(&vars->map, key) : 0;
}

internal_proc size_t
templ_vars_len(Templ_Vars *vars) {
    return (vars) ? buf_len(vars->keys) : 0;
}

internal_proc char *
templ_vars_get_key(Templ_Vars *vars, int index) {
    return (vars) ? vars->keys[index] : 0;
}

internal_proc void
init() {
    init_resolver();
}

internal_proc Parsed_Templ *
templ_compile(char *filename) {
    Parsed_Templ *result = parse_file(filename);

    return result;
}

internal_proc void
templ_render(Parsed_Templ *templ, Templ_Vars *vars = 0) {
    for ( int i = 0; i < templ_vars_len(vars); ++i ) {
        char *key = templ_vars_get_key(vars, i);
        char *value = templ_vars_get(vars, key);

        sym_push_var(key, type_str, val_str(value));
    }

    Resolved_Templ *result = resolve(templ);
    exec(result);
}

internal_proc void
templ_main(int argc, char **argv) {
    if ( argc < 2 ) {
        printf("templ.exe <filename>\n");
    }

    Templ_Vars *vars = templ_vars_new();
    templ_vars_add(vars, "name", "fooname");

    init();
    Parsed_Templ *templ = templ_compile(argv[1]);
    templ_render(templ, vars);

    file_write("test.html", gen_result, strlen(gen_result));
}

