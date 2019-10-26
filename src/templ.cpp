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

#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define CLAMP_MAX(x, max) MIN(x, max)
#define CLAMP_MIN(x, min) MAX(x, min)
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))

#define KB(X) (  (X)*1024)
#define MB(X) (KB(X)*1024)
#define GB(X) (MB(X)*1024)

#define AST_DUP(x) ast_dup(x, num_##x * sizeof(*x))

#define PROC_CALLBACK(name) Val * name(Resolved_Arg **args, size_t num_args)
#define FILTER_CALLBACK(name) Val * name(Val *val, Resolved_Arg **args, size_t num_args)
#define TEST_CALLBACK(name) Val * name(Val *val, Resolved_Expr **args, size_t num_args)

#define genf(...)   gen_result = strf("%s%s", gen_result, strf(__VA_ARGS__))
#define genlnf(...) gen_result = strf("%s\n", gen_result); gen_indentation(); genf(__VA_ARGS__)
#define genln()     gen_result = strf("%s\n", gen_result); gen_indentation()

#define implement_me() assert(0)
#define illegal_path() assert(0)

#define erstes_if if

#define internal_proc static
#define global_var    static

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : (*((void **)&(b)) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)

namespace templ {

#include "common.cpp"
#include "utf8.cpp"
#include "os.cpp"
#include "lex.cpp"
#include "ast.cpp"
#include "parser.cpp"
#include "resolve.cpp"
#include "filter.cpp"
#include "tests.cpp"
#include "exec.cpp"

global_var Arena templ_arena;

struct Templ_Var {
    char *name;
    Val *val;
    Scope *scope;
};

internal_proc Templ_Var *
templ_var(char *name) {
    Templ_Var *result = ALLOC_STRUCT(&templ_arena, Templ_Var);

    result->name  = name;
    result->val   = 0;
    result->scope = scope_new(0, name);

    return result;
}

internal_proc Templ_Var *
templ_var(char *name, Val *val) {
    Templ_Var *result = ALLOC_STRUCT(&templ_arena, Templ_Var);

    result->name  = name;
    result->val   = val;
    result->scope = 0;

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

internal_proc char *
templ_render(Parsed_Templ *templ, Templ_Var **vars = 0, size_t num_vars = 0) {
    for ( int i = 0; i < num_vars; ++i ) {
        Templ_Var *var = vars[i];

        if ( var->scope ) {
            Sym *sym = sym_push_var(var->name, type_dict);
            sym->scope = var->scope;
        } else {
            assert(var->val);
            Sym *sym = sym_push_var(var->name, type_any, var->val);
        }
    }

    Resolved_Templ *result = resolve(templ);
    exec(result);

    return gen_result;
}

internal_proc void
templ_init(size_t parse_arena_size, size_t resolve_arena_size,
        size_t exec_arena_size)
{
    arena_init(&templ_arena, MB(100));
    init_resolver();
}

} /* namespace templ */

#undef IS_POW2
#undef MIN
#undef MAX
#undef CLAMP_MAX
#undef CLAMP_MIN
#undef IS_POW2
#undef ALIGN_DOWN
#undef ALIGN_UP
#undef ALIGN_DOWN_PTR
#undef ALIGN_UP_PTR
#undef implement_me
#undef illegal_path
#undef erstes_if
#undef internal_proc
#undef global_var
#undef ALLOC_SIZE
#undef ALLOC_STRUCT
#undef AST_DUP

#undef PROC_CALLBACK
#undef FILTER_CALLBACK
#undef TEST_CALLBACK

#undef genf
#undef genlnf
#undef genln

#undef _CRT_SECURE_NO_WARNINGS
