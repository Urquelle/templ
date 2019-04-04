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
            for ( int i = instr->instr_loop.min; i < instr->instr_loop.max; ++i ) {
                *instr->instr_loop.it = i;
                for ( int j = 0; j < instr->instr_loop.num_instr; ++j ) {
                    gen_instr(instr->instr_loop.instr[j]);
                }
            }
        } break;

        case INSTR_IF: {
            /* @TODO: bedingung auswerten */
            for ( int i = 0; i < instr->instr_if.num_instr; ++i ) {
                gen_instr(instr->instr_if.instr[i]);
            }
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
