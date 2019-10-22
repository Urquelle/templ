global_var Arena templ_arena;

struct Templ_Var {
    char *name;
    Scope *scope;
};

internal_proc Templ_Var *
templ_var(char *name) {
    Templ_Var *result = ALLOC_STRUCT(&templ_arena, Templ_Var);

    result->name  = name;
    result->scope = scope_new(0, name);

    return result;
}

internal_proc void
templ_var_set(Templ_Var *var, char *key, char *value) {
    Scope *prev_scope = scope_set(var->scope);
    sym_push_var(key, type_str, val_str(value));
    scope_set(prev_scope);
}

internal_proc void
templ_var_set(Templ_Var *var, char *key, int value) {
    Scope *prev_scope = scope_set(var->scope);
    sym_push_var(key, type_int, val_int(value));
    scope_set(prev_scope);
}

internal_proc void
templ_var_set(Templ_Var *var, char *key, float value) {
    Scope *prev_scope = scope_set(var->scope);
    sym_push_var(key, type_float, val_float(value));
    scope_set(prev_scope);
}

internal_proc void
templ_var_set(Templ_Var *var, char *key, Templ_Var *value) {
    Scope *prev_scope = scope_set(var->scope);
    Sym *sym = sym_push_var(key, type_dict);
    sym->scope = value->scope;
    scope_set(prev_scope);
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
templ_render(Parsed_Templ *templ, Templ_Var **vars = 0, size_t num_vars = 0) {
    for ( int i = 0; i < num_vars; ++i ) {
        Templ_Var *var = vars[i];
        Sym *sym = sym_push_var(var->name, type_dict);
        sym->scope = var->scope;
    }

    Resolved_Templ *result = resolve(templ);
    exec(result);
}

internal_proc void
templ_main(int argc, char **argv) {
    if ( argc < 2 ) {
        printf("templ.exe <filename>\n");
    }

    arena_init(&templ_arena, MB(100));
    init();

    Templ_Var **vars = 0;

    Templ_Var *address = templ_var("address");
    templ_var_set(address, "city", "berlin");
    templ_var_set(address, "street", "siegerstr. 2");

    Templ_Var *user = templ_var("user");
    templ_var_set(user, "name", "Noob");
    templ_var_set(user, "age", 25);
    templ_var_set(user, "address", address);

    buf_push(vars, user);

    Parsed_Templ *templ = templ_compile(argv[1]);
    templ_render(templ, vars, buf_len(vars));

    file_write("test.html", gen_result, strlen(gen_result));
}

