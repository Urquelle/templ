#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <locale>

#include "common.cpp"
#include "utf8.cpp"
#include "os.cpp"
#include "lex.cpp"
#include "ast.cpp"
#include "parser.cpp"
#include "resolve.cpp"
#include "exec.cpp"

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
templ_var_set(Templ_Var *var, char *key, s32 value) {
    Scope *prev_scope = scope_set(var->scope);
    sym_push_var(key, type_int, val_int(value));
    scope_set(prev_scope);
}

internal_proc void
templ_var_set(Templ_Var *var, char *key, f32 value) {
    Scope *prev_scope = scope_set(var->scope);
    sym_push_var(key, type_float, val_float(value));
    scope_set(prev_scope);
}

internal_proc void
templ_var_set(Templ_Var *var, char *key, b32 value) {
    Scope *prev_scope = scope_set(var->scope);
    sym_push_var(key, type_bool, val_bool(value));
    scope_set(prev_scope);
}

internal_proc void
templ_var_set(Templ_Var *var, char *key, Templ_Var *value) {
    Scope *prev_scope = scope_set(var->scope);
    Sym *sym = sym_push_var(key, type_dict);
    sym->scope = value->scope;
    scope_set(prev_scope);
}

internal_proc Parsed_Templ *
templ_compile_file(char *filename) {
    Parsed_Templ *result = parse_file(filename);

    return result;
}

internal_proc Parsed_Templ *
templ_compile_string(char *content) {
    Parsed_Templ *result = parse_string(content);

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
templ_init(size_t parse_arena_size, size_t resolve_arena_size,
        size_t exec_arena_size)
{
    arena_init(&templ_arena, MB(100));
    init_resolver();
}

