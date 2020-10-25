#include <assert.h>

#include "templ.cpp"

templ::api::Arena *global_arena;

REGEX_ALLOCATOR(regex_alloc_custom) {
    using namespace templ::api;

    void *result = arena_alloc(global_arena, size);

    return result;
}

void
test_regexp() {
    using namespace templ::api;

    Arena arena = {};
    arena_init(&arena, KB(10));

    global_arena = &arena;

    Regex r = regex_parse("a?bc");
    Regex_Result result = regex_test(&r, "abc");

    assert(result.success);
    result = regex_test(&r, "bc");
    assert(result.success);
    result = regex_test(&r, "zbc");
    assert(!result.success);

    r = regex_parse("a(b.)*cd");
    result = regex_test(&r, "ab!b$cd");
    assert(result.success);
    result = regex_test(&r, "ab!cd");
    assert(result.success);
    result = regex_test(&r, "acd");
    assert(result.success);
    result = regex_test(&r, "ac");
    assert(!result.success);

    r = regex_parse("a.*c");
    result = regex_test(&r, "acc");
    assert(result.success);
}

int
main(int argc, char **argv) {
    test_regexp();

    return 0;
}

