global_var Arena instr_arena;

enum Reg_Kind {
    RAX,
    RBX,
};

global_var int64_t regs[64];

enum Instr_Kind {
    INSTR_NONE,
    INSTR_NOP,
    INSTR_PRINT,
    INSTR_ADD,
    INSTR_SET,
    INSTR_LOOP,
    INSTR_LABEL,
    INSTR_IF,
};

struct Instr {
    Instr_Kind kind;

    int64_t reg1;
    int64_t reg2;
    int64_t reg3;

    union {
        struct {
            char *val;
        } instr_print;

        struct {
            /* @TODO: bedingung speichern */
            Instr **instr;
            size_t num_instr;
        } instr_if;

        struct {
            int min;
            int max;
            Instr **instr;
            size_t num_instr;
        } instr_loop;

        struct {
            char *name;
            Instr **instr;
            size_t num_instr;
        } instr_label;
    };
};

global_var Instr instr_nop = { INSTR_NOP };

global_var Instr ** global_instr;

internal_proc Instr *
instr_new(Instr_Kind kind) {
    Instr *result = ALLOC_STRUCT(&instr_arena, Instr);

    result->kind = kind;

    return result;
}

internal_proc char *
copy_str(char *val) {
    size_t size = strlen(val);
    char *ptr = (char *)xmalloc(size+1);
    memcpy(ptr, val, size);
    ptr[size] = 0;

    return ptr;
}

internal_proc Instr *
instr_print(char *val) {
    Instr *result = instr_new(INSTR_PRINT);

    result->instr_print.val = copy_str(val);

    return result;
}

internal_proc Instr *
instr_add() {
    Instr *result = instr_new(INSTR_ADD);

    return result;
}

internal_proc Instr *
instr_set() {
    Instr *result = instr_new(INSTR_SET);

    return result;
}

internal_proc Instr *
instr_if(Instr **instr, size_t num_instr) {
    Instr *result = instr_new(INSTR_IF);

    return result;
}

internal_proc Instr *
instr_loop(int min, int max, Instr **instr, size_t num_instr) {
    Instr *result = instr_new(INSTR_LOOP);

    result->reg1           = RBX;
    result->instr_loop.min = min;
    result->instr_loop.max = max;
    result->instr_loop.instr = instr;
    result->instr_loop.num_instr = num_instr;

    return result;
}

internal_proc Instr *
instr_label(char *name, Instr **instr, size_t num_instr) {
    Instr *result = instr_new(INSTR_LABEL);

    result->instr_label.name = name;
    result->instr_label.instr = instr;
    result->instr_label.num_instr = num_instr;

    return result;
}

internal_proc void
push_instr(Instr *instr) {
    buf_push(global_instr, instr);
}

