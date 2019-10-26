#include "templ.cpp"

int
main(int argc, char **argv) {
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

    Parsed_Templ *templ = templ_compile_string("hallo {{ user.name }}");
    char *result = templ_render(templ, vars, buf_len(vars));

    file_write("test.html", result, strlen(result));

    return 0;
}
