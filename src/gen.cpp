#define genf(...)   gen_result = strf("%s%s", gen_result, strf(__VA_ARGS__))
#define genlnf(...) gen_result = strf("%s\n", gen_result); gen_indentation(); genf(__VA_ARGS__)
#define genln()     gen_result = strf("%s\n", gen_result); gen_indentation()

global_var char *gen_result = "";
global_var int gen_indent   = 0;

internal_proc void
gen_indentation() {
    gen_result = strf("%s%*.s", gen_result, 4 * gen_indent, "         ");
}

internal_proc void
gen_instr(Instr *instr) {
    switch (instr->kind) {
        case INSTR_PRINT: {
            genf("%s", instr->instr_print.val);
        } break;

        case INSTR_NOP: {
            //
        } break;

        case INSTR_LOOP: {
            //
        } break;

        case INSTR_SET: {
            //
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
gen() {
    for ( int i = 0; i < buf_len(instructions); ++i ) {
        gen_instr(instructions[i]);
    }
}
