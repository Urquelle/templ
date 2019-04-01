#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "common.cpp"
#include "os.cpp"
#include "lex.cpp"
#include "ast.cpp"
#include "doc.cpp"
#include "parser.cpp"
#include "frame.cpp"
#include "instr.cpp"
#include "resolve.cpp"
#include "eval.cpp"
#include "gen.cpp"

internal_proc void
init() {
    init_resolver();
}

int
main(int argc, char **argv) {
    if ( argc < 2 ) {
        printf("templ.exe <filename>\n");
    }

    init();
    Doc *doc = parse_file(argv[1]);
    resolve(doc);
    eval(doc);
    gen();
    file_write("test.html", gen_result, strlen(gen_result));

    return 0;
}
