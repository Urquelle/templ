#include <assert.h>

#include "templ.cpp"

void
test_regexp() {
    using namespace templ::api;

    Arena arena = {};
    arena_init(&arena, KB(10));

    Regex r = regex_parse("a?bc", &arena);
    Regex_Result result = regex_test(&r, "abc", &arena);

    assert(result.success);
    result = regex_test(&r, "bc", &arena);
    assert(result.success);
    result = regex_test(&r, "zbc", &arena);
    assert(!result.success);

    r = regex_parse("a(b.)*cd", &arena);
    result = regex_test(&r, "ab!b$cd", &arena);
    assert(result.success);
    result = regex_test(&r, "ab!cd", &arena);
    assert(result.success);
    result = regex_test(&r, "acd", &arena);
    assert(result.success);
    result = regex_test(&r, "ac", &arena);
    assert(!result.success);

    r = regex_parse("a.*c", &arena);
    result = regex_test(&r, "acc", &arena);
    assert(result.success);
}

int
main(int argc, char **argv) {
    test_regexp();

    return 0;
}

