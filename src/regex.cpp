struct Regex;
struct Regex_Result;

Regex_Result regex_test(Regex *r, char *str);

#define REGEX_ALLOCATOR(name) void * name(size_t size)
typedef REGEX_ALLOCATOR(Regex_Alloc);

REGEX_ALLOCATOR(regex_alloc_default) {
    void *result = malloc(size);

    return result;
}

Regex_Alloc *regex_alloc = regex_alloc_default;

enum Regex_Rule_Kind {
    REGEX_RULE_KIND_NONE,
    REGEX_RULE_KIND_WILDCARD,
    REGEX_RULE_KIND_GROUP,
    REGEX_RULE_KIND_ELEMENT,
};

enum Regex_Quantifier {
    REGEX_QUANTIFIER_NONE,
    REGEX_QUANTIFIER_ONE,
    REGEX_QUANTIFIER_ZERO_OR_ONE,
    REGEX_QUANTIFIER_ZERO_OR_MORE,
};

struct Regex_Rule {
    Regex_Rule_Kind     kind;
    Regex_Quantifier    quantifier;
    char  * value;
    Regex * r;
};

struct Regex_Result {
    b32 success;
    u32 count;
};

enum Regex_Entry_State {
    REGEX_ENTRY_NONE,
    REGEX_ENTRY_BACKTRACKABLE,
    REGEX_ENTRY_NON_BACKTRACKABLE,
};
struct Regex_Entry {
    Regex_Entry_State   state;
    Regex_Rule        * current_rule;
    u32               * consumed;
    size_t              num_consumed;
};

struct Regex_Stack {
    Queue  entries;
};

struct Regex {
    Queue    rules;

    Regex  * curr;
    Regex  * prev;
};

Regex_Entry *
regex_entry(Regex_Entry_State state, Regex_Rule *rule, u32 *consumed, size_t num_consumed) {
    Regex_Entry *result = (Regex_Entry *)regex_alloc(sizeof(Regex_Entry));

    result->state        = state;
    result->current_rule = rule;
    result->consumed     = consumed;
    result->num_consumed = num_consumed;

    return result;
}

Regex_Rule *
regex_rule(Regex_Rule_Kind kind, Regex_Quantifier quantifier, char *value = NULL) {
    Regex_Rule *result = (Regex_Rule *)regex_alloc(sizeof(Regex_Rule));

    result->kind       = kind;
    result->quantifier = quantifier;
    result->value      = value;

    return result;
}

void
regex_push_rule(Regex *r, Regex_Rule *rule) {
    ASSERT(r);

    queue_push(&r->rules, rule);
}

void
regex_start_group(Regex *r) {
    Regex *group = (Regex *)regex_alloc(sizeof(Regex));

    *group       = {};
    group->prev = r;
    group->curr = group;

    r->curr = group;

    Regex_Rule *rule = regex_rule(REGEX_RULE_KIND_GROUP, REGEX_QUANTIFIER_ONE);
    rule->r = group;

    regex_push_rule(r, rule);
}

void
regex_close_group(Regex *r) {
    r->prev->curr = r->curr->prev;
}

Regex *
regex_last(Regex *r) {
    return r->curr;
}

void
regex_push_entry(Regex_Stack *stack, Regex_Entry *entry) {
    ASSERT(stack);

    queue_push(&stack->entries, entry);
}

Regex_Entry *
regex_pop_entry(Regex_Stack *stack) {
    Regex_Entry *result = (Regex_Entry *)queue_pop(&stack->entries);

    return result;
}

Regex_Result
regex_rule_matches(Regex_Rule *rule, char *str, int index) {
    Regex_Result result = {};

    if ( utf8_str_len(str) <= index ) {
        return result;
    }

    if ( rule->kind == REGEX_RULE_KIND_WILDCARD ) {
        result.success = true;
        result.count   = 1;

        return result;
    }

    if ( rule->kind == REGEX_RULE_KIND_ELEMENT ) {
        char left[5];
        char right[5];

        size_t char_size = utf8_char_size(rule->value);
        for ( int i = 0; i < char_size; ++i ) {
            left[i] = rule->value[i];
        }
        left[char_size] = 0;

        char_size = utf8_char_size(str+index);
        for ( int i = 0; i < char_size; ++i ) {
            right[i] = (str+index)[i];
        }
        right[char_size] = 0;

        result.success = utf8_str_eq(left, right);
        result.count   = (result.success) ? 1 : 0;

        return result;
    }

    if ( rule->kind == REGEX_RULE_KIND_GROUP ) {
        return regex_test(rule->r, str+index);
    }

    return result;
}

b32
regex_backtrack(Queue *q, Regex_Stack *stack, Regex_Rule **current_rule, u32 *count) {
    b32 result = false;

    queue_unshift(q, *current_rule);

    while ( stack->entries.num_elems ) {
        Regex_Entry *entry = regex_pop_entry(stack);

        if ( entry->state == REGEX_ENTRY_BACKTRACKABLE ) {
            if ( entry->num_consumed == 0 ) {
                queue_unshift(q, entry->current_rule);
                continue;
            }

            u32 n = buf_pop(entry->consumed);
            *count -= n;
            regex_push_entry(stack, regex_entry(entry->state, entry->current_rule, entry->consumed, entry->num_consumed));
            result = true;

            break;
        }

        queue_unshift(q, entry->current_rule);
        for ( int c = 0; c <= entry->num_consumed; c++ ) {
            u32 n = entry->consumed[c];
            *count -= n;
        }
    }

    if (result) {
        *current_rule = (Regex_Rule *)queue_shift(q);
    }

    return result;
}

//
// API
//

user_api Regex
regex_parse(char *str) {
    Regex result = {};

    result.curr = &result;

    char *c = str;
    while ( *c ) {
        Regex *curr = regex_last(&result);

        if ( *c == '.' ) {
            regex_push_rule(curr, regex_rule(REGEX_RULE_KIND_WILDCARD, REGEX_QUANTIFIER_ONE));
        } else if ( *c == '?' ) {
            if ( !curr->rules.num_elems ) {
                return result;
            }

            Regex_Rule *last_rule = (Regex_Rule *)queue_entry(&curr->rules, curr->rules.num_elems-1);

            if ( last_rule->quantifier != REGEX_QUANTIFIER_ONE ) {
                return result;
            }

            last_rule->quantifier = REGEX_QUANTIFIER_ZERO_OR_ONE;
        } else if ( *c == '*' ) {
            if ( !curr->rules.num_elems ) {
                return result;
            }

            Regex_Rule *last_rule = (Regex_Rule *)queue_entry(&curr->rules, curr->rules.num_elems-1);

            if ( last_rule->quantifier != REGEX_QUANTIFIER_ONE ) {
                return result;
            }

            last_rule->quantifier = REGEX_QUANTIFIER_ZERO_OR_MORE;
        } else if ( *c == '+' ) {
            if ( !curr->rules.num_elems ) {
                return result;
            }

            Regex_Rule *last_rule = (Regex_Rule *)queue_entry(&curr->rules, curr->rules.num_elems-1);

            if ( last_rule->quantifier != REGEX_QUANTIFIER_ONE ) {
                return result;
            }

            Regex_Rule *copy = {};

            copy->kind       = last_rule->kind;
            copy->quantifier = REGEX_QUANTIFIER_ZERO_OR_MORE;
            copy->value      = last_rule->value;
            copy->r          = last_rule->r;

            regex_push_rule(curr, copy);
        } else if ( *c == '(' ) {
            regex_start_group(curr);
        } else if ( *c == ')' ) {
            regex_close_group(curr);
        } else if ( *c == '\\' ) {
            if ( utf8_str_len(c) < 2 ) {
                return result;
            }

            regex_push_rule(curr, regex_rule(REGEX_RULE_KIND_ELEMENT, REGEX_QUANTIFIER_ONE, c+1));
            c += 1;
        } else {
            regex_push_rule(curr, regex_rule(REGEX_RULE_KIND_ELEMENT, REGEX_QUANTIFIER_ONE, c));
        }

        c += utf8_char_size(c);
    }

    return result;
}

user_api Regex_Result
regex_test(Regex *r, char *str) {
    Regex_Result result = {};
    Regex_Stack stack = {};

    Queue q = r->rules;
    Queue *queue = &q;

    u32 count = 0;
    Regex_Rule *rule = (Regex_Rule *)queue_shift(queue);

    while ( rule ) {
        switch ( rule->quantifier ) {
            case REGEX_QUANTIFIER_ONE: {
                Regex_Result res = regex_rule_matches(rule, str, count);

                if ( !res.success ) {
                    int count_before_backtracking = count;
                    b32 could_backtrack = regex_backtrack(queue, &stack, &rule, &count);

                    if ( !could_backtrack) {
                        result.success = false;
                        result.count   = count;

                        return result;
                    }

                    continue;
                }

                count += res.count;

                u32 *consumed = 0;
                buf_push(consumed, res.count);
                Regex_Entry *entry = regex_entry(
                            REGEX_ENTRY_NON_BACKTRACKABLE,
                            rule,
                            consumed,
                            buf_len(consumed));

                queue_push(&stack.entries, entry);
                rule = (Regex_Rule *)queue_shift(queue);

                continue;
            } break;

            case REGEX_QUANTIFIER_ZERO_OR_ONE: {
                if ( count >= utf8_str_len(str) ) {
                    Regex_Entry *entry = regex_entry(
                            REGEX_ENTRY_NON_BACKTRACKABLE,
                            rule,
                            0, 0
                    );

                    queue_push(&stack.entries, entry);
                    rule = (Regex_Rule *)queue_shift(queue);

                    continue;
                }

                Regex_Result res = regex_rule_matches(rule, str, count);
                count += res.count;

                u32 *consumed = 0;
                buf_push(consumed, res.count);
                Regex_Entry *entry = regex_entry(
                        ( res.success && res.count > 0 )
                            ? REGEX_ENTRY_BACKTRACKABLE
                            : REGEX_ENTRY_NON_BACKTRACKABLE,
                        rule,
                        consumed, buf_len(consumed)
                );

                queue_push(&stack.entries, entry);
                rule = (Regex_Rule *)queue_shift(queue);

                continue;
            } break;

            case REGEX_QUANTIFIER_ZERO_OR_MORE: {
                Regex_Entry *entry = regex_entry(
                    REGEX_ENTRY_BACKTRACKABLE,
                    rule,
                    0, 0
                );

                for (;;) {
                    if ( count >= utf8_str_len(str) ) {
                        if ( stack.entries.num_elems == 0 ) {
                            buf_push(entry->consumed, 0);
                            entry->num_consumed = buf_len(entry->consumed);
                            entry->state = REGEX_ENTRY_NON_BACKTRACKABLE;
                        }

                        queue_push(&stack.entries, entry);
                        rule = (Regex_Rule *)queue_shift(queue);

                        break;
                    }

                    Regex_Result res = regex_rule_matches(rule, str, count);
                    if ( !res.success || res.count == 0 ) {
                        if ( stack.entries.num_elems == 0 ) {
                            buf_push(entry->consumed, 0);
                            entry->num_consumed = buf_len(entry->consumed);
                            entry->state = REGEX_ENTRY_NON_BACKTRACKABLE;
                        }

                        queue_push(&stack.entries, entry);
                        rule = (Regex_Rule *)queue_shift(queue);

                        break;
                    }

                    buf_push(entry->consumed, res.count);
                    entry->num_consumed = buf_len(entry->consumed);

                    count += res.count;
                }

                continue;
            } break;

            default: {
                ASSERT(!"nicht unterstützte operation");
            } break;
        }
    }

    result.success = true;
    result.count   = count;

    return result;
}

