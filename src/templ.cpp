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

#define ALLOC_SIZE(arena, size) arena_alloc(arena, size)
#define ALLOC_STRUCT(arena, s)  (s *)arena_alloc(arena, sizeof(s))
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))
#define AST_DUP(x) ast_dup(x, num_##x * sizeof(*x))
#define CLAMP_MAX(x, max) MIN(x, max)
#define CLAMP_MIN(x, min) MAX(x, min)
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define PROC_CALLBACK(name)   Val * name(Val *operand, Resolved_Expr **args, size_t num_args, Map *nargs, char **narg_keys, size_t num_narg_keys, Resolved_Arg **kwargs, size_t num_kwargs, Resolved_Arg **varargs, size_t num_varargs)

#define erstes_if      if
#define genf(...)      gen_result = strf("%s%s", gen_result, strf(__VA_ARGS__))
#define genln()        gen_result = strf("%s\n", gen_result); gen_indentation()
#define genlnf(...)    gen_result = strf("%s\n", gen_result); gen_indentation(); genf(__VA_ARGS__)
#define global_var     static
#define internal_proc  static
#define illegal_path() assert(0)
#define implement_me() assert(0)
#define narg(name)     ((Resolved_Arg *)map_get(nargs, intern_str(name)))
#define user_api

#define GB(X) (MB(X)*1024)
#define KB(X) (  (X)*1024)
#define MB(X) (KB(X)*1024)
#define buf__hdr(b) ((Buf_Hdr *)((char *)(b) - offsetof(Buf_Hdr, buf)))
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : (*((void **)&(b)) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

namespace templ {

typedef bool     b32;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;

struct Expr;
struct Map;
struct Parsed_Templ;
struct Parser;
struct Resolved_Arg;
struct Resolved_Expr;
struct Resolved_Pair;
struct Resolved_Stmt;
struct Resolved_Templ;
struct Stmt;
struct Sym;
struct Type;
struct Type_Field;
struct Val;

typedef Parsed_Templ Templ;
typedef PROC_CALLBACK(Proc_Callback);

enum { True = 1, False = 0, None = -1 };

internal_proc PROC_CALLBACK(proc_super);
internal_proc PROC_CALLBACK(proc_loop);
internal_proc PROC_CALLBACK(proc_cycle);
internal_proc PROC_CALLBACK(proc_dict);
internal_proc PROC_CALLBACK(proc_cycler);
internal_proc PROC_CALLBACK(proc_cycler_next);
internal_proc PROC_CALLBACK(proc_cycler_reset);
internal_proc PROC_CALLBACK(proc_exec_macro);
internal_proc PROC_CALLBACK(proc_joiner);
internal_proc PROC_CALLBACK(proc_joiner_call);
internal_proc PROC_CALLBACK(proc_range);
internal_proc PROC_CALLBACK(proc_lipsum);
internal_proc PROC_CALLBACK(proc_namespace);

internal_proc PROC_CALLBACK(filter_abs);
internal_proc PROC_CALLBACK(filter_attr);
internal_proc PROC_CALLBACK(filter_batch);
internal_proc PROC_CALLBACK(filter_capitalize);
internal_proc PROC_CALLBACK(filter_center);
internal_proc PROC_CALLBACK(filter_default);
internal_proc PROC_CALLBACK(filter_dictsort);
internal_proc PROC_CALLBACK(filter_escape);
internal_proc PROC_CALLBACK(filter_filesizeformat);
internal_proc PROC_CALLBACK(filter_first);
internal_proc PROC_CALLBACK(filter_format);
internal_proc PROC_CALLBACK(filter_lower);
internal_proc PROC_CALLBACK(filter_truncate);
internal_proc PROC_CALLBACK(filter_upper);

internal_proc PROC_CALLBACK(test_callable);
internal_proc PROC_CALLBACK(test_defined);
internal_proc PROC_CALLBACK(test_divisibleby);
internal_proc PROC_CALLBACK(test_eq);
internal_proc PROC_CALLBACK(test_escaped);
internal_proc PROC_CALLBACK(test_even);
internal_proc PROC_CALLBACK(test_ge);
internal_proc PROC_CALLBACK(test_gt);
internal_proc PROC_CALLBACK(test_in);
internal_proc PROC_CALLBACK(test_iterable);
internal_proc PROC_CALLBACK(test_le);
internal_proc PROC_CALLBACK(test_lt);
internal_proc PROC_CALLBACK(test_mapping);
internal_proc PROC_CALLBACK(test_ne);
internal_proc PROC_CALLBACK(test_none);
internal_proc PROC_CALLBACK(test_number);
internal_proc PROC_CALLBACK(test_odd);
internal_proc PROC_CALLBACK(test_sameas);
internal_proc PROC_CALLBACK(test_sequence);
internal_proc PROC_CALLBACK(test_string);
internal_proc PROC_CALLBACK(test_undefined);

internal_proc void              exec_stmt(Resolved_Stmt *stmt);
internal_proc void              exec(Resolved_Templ *templ);
internal_proc Expr            * parse_expr(Parser *p, b32 do_parse_filter = true);
internal_proc Parsed_Templ    * parse_file(char *filename);
internal_proc char            * parse_name(Parser *p);
internal_proc Stmt            * parse_stmt(Parser *p);
internal_proc Stmt            * parse_stmt_var(Parser *p);
internal_proc Stmt            * parse_stmt_lit(Parser *p);
internal_proc char            * parse_str(Parser *p);
internal_proc Expr            * parse_tester(Parser *p);
internal_proc Resolved_Templ  * resolve(Parsed_Templ *d, b32 with_context = true);
internal_proc void              resolve_add_block(char *name, Resolved_Stmt *block);
internal_proc Resolved_Expr   * resolve_expr(Expr *expr);
internal_proc Resolved_Expr   * resolve_expr_cond(Expr *expr);
internal_proc Resolved_Expr   * resolve_filter(Expr *expr);
internal_proc Resolved_Stmt   * resolve_stmt(Stmt *stmt, Resolved_Templ *templ);
internal_proc Resolved_Expr   * resolve_tester(Expr *expr);
internal_proc Sym             * sym_push_var(char *name, Type *type, Val *val = 0);

global_var char               * gen_result = "";
global_var int                  gen_indent   = 0;
Resolved_Templ                * global_current_tmpl;
global_var Resolved_Stmt      * global_for_stmt;
global_var b32                  global_for_break;
global_var b32                  global_for_continue;
global_var Resolved_Stmt      * global_super_block;
global_var Resolved_Templ     * current_templ;

#include "os.cpp"
#include "utf8.cpp"
#include "common.cpp"

global_var Map                  global_blocks;
global_var Arena                parse_arena;
global_var Arena                resolve_arena;
global_var Arena                templ_arena;

global_var char *symname_loop = intern_str("loop");
global_var char *symname_index = intern_str("index");

#include "lex.cpp"
#include "ast.cpp"
#include "parser.cpp"
#include "resolve.cpp"
#include "filter.cpp"
#include "testers.cpp"
#include "sysprocs.cpp"
#include "exec.cpp"

struct Templ_Var {
    char *name;
    Val *val;
    Type *type;
};

internal_proc Templ_Var *
templ_object(char *name) {
    Templ_Var *result = ALLOC_STRUCT(&templ_arena, Templ_Var);
    Scope *scope = scope_new(0, name);

    result->name = intern_str(name);
    result->val  = val_dict(scope);
    result->type = type_dict(scope);

    return result;
}

user_api Templ_Var *
templ_var(char *name, char *val) {
    Templ_Var *result = ALLOC_STRUCT(&templ_arena, Templ_Var);

    result->name = intern_str(name);
    result->val  = val_str(val);
    result->type = type_str;

    return result;
}

user_api Templ_Var *
templ_var(char *name, s32 val) {
    Templ_Var *result = ALLOC_STRUCT(&templ_arena, Templ_Var);

    result->name = intern_str(name);
    result->val  = val_int(val);
    result->type = type_int;

    return result;
}

user_api Templ_Var *
templ_var(char *name, f32 val) {
    Templ_Var *result = ALLOC_STRUCT(&templ_arena, Templ_Var);

    result->name = intern_str(name);
    result->val  = val_float(val);
    result->type = type_float;

    return result;
}

user_api Templ_Var *
templ_var(char *name, b32 val) {
    Templ_Var *result = ALLOC_STRUCT(&templ_arena, Templ_Var);

    result->name = intern_str(name);
    result->val  = val_bool(val);
    result->type = type_bool;

    return result;
}

internal_proc Templ_Var *
templ_var(char *name, Templ_Var *val) {
    return val;
}

user_api void
templ_var_set(Templ_Var *var, Templ_Var *value) {
    Scope *prev_scope = scope_set((Scope *)var->val->ptr);
    sym_push_var(value->name, value->type, value->val);
    scope_set(prev_scope);
}

struct Templ_Vars {
    Templ_Var **vars;
    size_t num_vars;
};

user_api Templ_Vars
templ_vars() {
    Templ_Vars result = {};

    return result;
}

user_api void
templ_vars_add(Templ_Vars *vars, Templ_Var *var) {
    buf_push(vars->vars, var);
    vars->num_vars = buf_len(vars->vars);
}

user_api Templ *
templ_compile_file(char *filename) {
    Templ *result = parse_file(filename);

    return result;
}

user_api Templ *
templ_compile_string(char *content) {
    Templ *result = parse_string(content);

    return result;
}

user_api char *
templ_render(Templ *templ, Templ_Vars *vars = 0) {
    if ( vars ) {
        for ( int i = 0; i < vars->num_vars; ++i ) {
            Templ_Var *var = vars->vars[i];
            sym_push_var(var->name, var->type, var->val);
        }
    }

    Resolved_Templ *t = resolve(templ);
    exec(t);

    return gen_result;
}

user_api void
templ_reset() {
    resolve_reset();
    exec_reset();
}

user_api void
templ_init(size_t parse_arena_size, size_t resolve_arena_size,
        size_t exec_arena_size)
{
    arena_init(&templ_arena, MB(100));
    resolve_init();
}

namespace api {
    using templ::Status;
    using templ::Templ;
    using templ::Templ_Var;
    using templ::Templ_Vars;
    using templ::os_file_write;
    using templ::status_error_get;
    using templ::status_filename;
    using templ::status_is_error;
    using templ::status_is_not_error;
    using templ::status_is_not_warning;
    using templ::status_is_warning;
    using templ::status_line;
    using templ::status_message;
    using templ::status_num_errors;
    using templ::status_num_warnings;
    using templ::status_reset;
    using templ::status_warning_get;
    using templ::templ_compile_file;
    using templ::templ_compile_string;
    using templ::templ_init;
    using templ::templ_object;
    using templ::templ_reset;
    using templ::templ_var;
    using templ::templ_vars;
    using templ::templ_vars_add;
    using templ::utf8_strlen;
    using templ::utf8_str_size;
}

} /* namespace templ */

#undef ALLOC_SIZE
#undef ALLOC_STRUCT
#undef ALIGN_DOWN
#undef ALIGN_DOWN_PTR
#undef ALIGN_UP
#undef ALIGN_UP_PTR
#undef ALLOC_SIZE
#undef ALLOC_STRUCT
#undef AST_DUP
#undef CLAMP_MAX
#undef CLAMP_MIN
#undef FILTER_CALLBACK
#undef IS_POW2
#undef MAX
#undef MIN
#undef PROC_CALLBACK
#undef TEST_CALLBACK
#undef erstes_if
#undef genf
#undef genln
#undef genlnf
#undef global_var
#undef illegal_path
#undef implement_me
#undef internal_proc
#undef user_api

#undef _CRT_SECURE_NO_WARNINGS
