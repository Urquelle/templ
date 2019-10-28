internal_proc TEST_CALLBACK(test_callable) {
    b32 result = is_callable(args[0]->type);

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_defined) {
    b32 result = args[0]->sym != 0;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_divisibleby) {
    b32 result = (val_int(val) % val_int(args[0]->val)) == 0;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_eq) {
    assert(val);
    assert(num_args == 1);

    Val *result = val_bool(*val == *args[0]->val);

    return result;
}

internal_proc TEST_CALLBACK(test_escaped) {
    implement_me();

    return val_bool(false);
}

internal_proc TEST_CALLBACK(test_even) {
    b32 result = (val_int(val) & 0x1) != 0x1;

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_ge) {
    b32 result = val_int(val) >= val_int(args[0]->val);

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_gt) {
    b32 result = val_int(val) > val_int(args[0]->val);

    return val_bool(result);
}

internal_proc TEST_CALLBACK(test_in) {
    implement_me();

    return val_bool(false);
}

