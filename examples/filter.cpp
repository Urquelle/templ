#include "templ.cpp"

PROC_CALLBACK(custom_int_times100) {
    using namespace templ::devapi;
    int result = val_int(value)*100;

    return val_int(result);
}

int
main(int argc, char **argv) {
    using namespace templ::api;
    using namespace templ::devapi;

    templ_init();
    templ_register_int_proc("times100", custom_int_times100, 0, 0, type_int);

    Templ *templ = templ_compile_string(R"END(
        {{ 15.times100() }}
        {{ 15 | times100 }}
    )END");

    char *result = templ_render(templ, 0);

    return 0;
}

