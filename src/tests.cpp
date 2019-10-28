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
    Val *set = args[0]->val;

    if ( !set || set->kind != VAL_LIST ) {
        return val_bool(false);
    }

    b32 found = false;
    for ( int i = 0; i < set->len; ++i ) {
        Val *it = val_item(set, i);

        if ( *it == *val ) {
            found = true;
            break;
        }
    }

    return val_bool(found);
}

internal_proc TEST_CALLBACK(test_iterable) {
    b32 result = VAL_ITERABLE_START <= val->kind && val->kind <= VAL_ITERABLE_END;

    return val_bool(result);
}

