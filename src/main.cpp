#include "templ.cpp"

int
main(int argc, char **argv) {
    if ( argc < 2 ) {
        printf("templ.exe <filename>\n");
    }

    templ_init(MB(100), MB(100), MB(100));

    Templ_Var **vars = 0;

    Templ_Var *address = templ_var("address");
    templ_var_set(address, "city", "berlin");
    templ_var_set(address, "street", "siegerstr. 2");

    Templ_Var *user = templ_var("user");
    templ_var_set(user, "name", "Noob");
    templ_var_set(user, "age", 25);
    templ_var_set(user, "address", address);

    buf_push(vars, user);

    Parsed_Templ *templ = templ_compile_file(argv[1]);
    templ_render(templ, vars, buf_len(vars));

    file_write("test.html", gen_result, strlen(gen_result));

    return 0;
}
