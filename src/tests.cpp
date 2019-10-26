internal_proc TEST_CALLBACK(test_callable) {
    implement_me();
    return false;
}

internal_proc TEST_CALLBACK(test_defined) {
    implement_me();
    return false;
}

internal_proc TEST_CALLBACK(test_divisibleby) {
    implement_me();
    return false;
}

internal_proc TEST_CALLBACK(test_eq) {
    assert(val);
    assert(num_args == 1);

    Val *result = val_bool(*val == *args[0]->val);

    return result;
}

internal_proc TEST_CALLBACK(test_escaped) {
    implement_me();
    return false;
}

internal_proc TEST_CALLBACK(test_even) {
    implement_me();
    return false;
}

internal_proc TEST_CALLBACK(test_ge) {
    implement_me();
    return false;
}

internal_proc TEST_CALLBACK(test_gt) {
    implement_me();
    return false;
}

internal_proc TEST_CALLBACK(test_in) {
    implement_me();
    return false;
}

