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
#include "templ.cpp"

int
main(int argc, char **argv) {
    templ_main(argc, argv);

    return 0;
}
