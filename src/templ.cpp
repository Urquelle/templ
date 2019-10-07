internal_proc void
init() {
    init_resolver();
}

internal_proc Resolved_Templ *
templ_compile(char *filename) {
    Parsed_Templ *parsed_templ = parse_file(filename);
    Resolved_Templ *result = resolve(parsed_templ);

    return result;
}

internal_proc void
templ_render(Resolved_Templ *templ) {
    exec(templ);
}

internal_proc void
templ_main(int argc, char **argv) {
    if ( argc < 2 ) {
        printf("templ.exe <filename>\n");
    }

    init();
    Resolved_Templ *resolved_templ = templ_compile(argv[1]);
    templ_render(resolved_templ);
    file_write("test.html", gen_result, strlen(gen_result));
}
