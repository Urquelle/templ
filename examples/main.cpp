#include "templ.cpp"

PROC_CALLBACK(custom_hello) {
    using namespace templ::devapi;

    return val_str("hello, world");
}

PROC_CALLBACK(custom_int_times100) {
    using namespace templ::devapi;
    int result = val_int(value)*100;

    return val_int(result);
}

int
main(int argc, char **argv) {
    using namespace templ::api;
    using namespace templ::devapi;

    templ_init(MB(100), MB(100), MB(100));

    templ_register_proc("hello", custom_hello, 0, 0, type_str);
    templ_register_int_proc("times100", custom_int_times100, 0, 0, type_int);

    char *data;
    os_file_read("data.json", &data);
    Json json = json_parse(data);

    Templ *templ = templ_compile_file("main.tpl");

    Templ_Var *users = templ_var("users", json);
    Templ_Vars vars = templ_vars();

    templ_vars_add(&vars, users);

    char *result = templ_render(templ, &vars);
    os_file_write("out.html", result, utf8_str_size(result));

    return 0;
}
