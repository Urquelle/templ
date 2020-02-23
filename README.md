<p align="center">
    <img src="https://github.com/Urquelle/templ/blob/dev/misc/logo_250.png" />
    <br /><br />
    <a href="https://github.com/Urquelle/templ/releases/latest">
        <img src="https://img.shields.io/github/v/release/Urquelle/templ" />
    </a>
    <a href="https://isocpp.org/">
        <img src="https://img.shields.io/badge/language-C++-blue.svg" />
    </a>
    <a href="https://raw.githubusercontent.com/Urquelle/templ/dev/LICENSE">
        <img src="https://img.shields.io/github/license/Urquelle/templ" />
    </a>
    <a href="https://en.wikipedia.org/wiki/Made_in_Germany">
        <img src="https://img.shields.io/badge/made%20in-germany-red" />
    </a>
</p>

<hr />

<table>
    <tr>
        <th>build</th>
        <th>statistics</th>
        <th>meta</th>
    </tr>
    <tr>
        <td>
            <table>
                <tr>
                    <td>travis:</td>
                    <td align="right">
                        <a href="https://travis-ci.org/Urquelle/templ">
                            <img src="https://travis-ci.org/Urquelle/templ.svg?branch=dev" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>appveyor:</td>
                    <td align="right">
                        <a href="https://ci.appveyor.com/project/Urquelle/templ/branch/dev">
                            <img src="https://ci.appveyor.com/api/projects/status/d5vpfm3mtao94aow/branch/dev?svg=true" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>github:</td>
                    <td align="right">
                        <img src="https://github.com/Urquelle/templ/workflows/windows-build/badge.svg" />
                    </td>
                </tr>
            </table>
        </td>
        <td>
            <table>
                <tr>
                    <td><a href="https://github.com/Urquelle/templ/issues">issues</a>:</td>
                    <td align="right">
                        <a href="https://github.com/Urquelle/templ/issues">
                            <img src="https://img.shields.io/github/issues/Urquelle/templ" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td><a href="https://www.codacy.com/manual/Urquelle/templ?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Urquelle/templ&amp;utm_campaign=Badge_Grade">quality</a>:</td>
                    <td align="right">
                        <a href="https://www.codacy.com/manual/Urquelle/templ?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Urquelle/templ&amp;utm_campaign=Badge_Grade">
                            <img src="https://api.codacy.com/project/badge/Grade/f4e97144ea6d43b3a38fc34e9b5e50b7" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>loc:</td>
                    <td align="right"><img src="https://tokei.rs/b1/github/Urquelle/templ" /></td>
                </tr>
            </table>
        </td>
        <td>
            <table>
                <tr>
                    <td><a href="https://github.com/Urquelle/templ/commits/dev">commits</a>:</td>
                    <td align="right">
                        <a href="https://github.com/Urquelle/templ/commits/dev">
                            <img src="https://img.shields.io/github/commits-since/Urquelle/templ/latest/dev" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>editor:</td>
                    <td align="right">
                        <a href="https://neovim.io/">
                            <img src="https://img.shields.io/badge/powered%20by-neovim-green" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>platform:</td>
                    <td align="right"><img src="https://img.shields.io/badge/platform-win%20%7C%20linux-blue" /></td>
                </tr>
            </table>
        </td>
    </tr>
</table>

table of contents
=================

   * [simple c++ example](#simple-c-example)
   * [jinja template examples](#jinja-template-examples)
   * [json](#json)
   * [unicode](#unicode)
   * [expressions](#expressions)
      * [literals](#literals)
   * [statements](#statements)
      * [block](#block)
      * [do](#do)
      * [extends](#extends)
      * [filter](#filter)
      * [for](#for)
      * [if](#if)
      * [import](#import)
      * [include](#include)
      * [macro](#macro)
      * [raw](#raw)
      * [set](#set)
      * [set block](#set_block)
   * [filter](#filter-1)
   * [tests](#tests)
   * [introspection](#introspection)
   * [custom procs](#custom-procs)
      * [global](#global)
      * [type](#type)
      * [tester](#tester)

## simple c++ example

in the following example a datastructure is created and put into the engine's context. the subsequent call of `templ_render` is given the created datastructure to render the string template.

```c++
#include "templ.cpp"

int
main(int argc, char **argv) {
    using namespace templ::api;

    templ_init(MB(100), MB(100), MB(100));

    Templ_Vars vars = templ_vars();
    Templ_Var *name = templ_var("name", "noob");
    templ_vars_add(&vars, name);

    Templ *templ = templ_compile_string("hello {{ name }}");
    char *result = templ_render(templ, &vars);

    os_file_write("test.html", result, utf8_strlen(result));
    if ( status_is_error() ) {
        for ( int i = 0; i < status_num_errors(); ++i ) {
            Status *error = status_error_get(i);
            fprintf(stderr, "%s in %s line %lld\n", status_message(error),
                status_filename(error), status_line(error));
        }

        for ( int i = 0; i < status_num_warnings(); ++i ) {
            Status *warning = status_warning_get(i);
            fprintf(stderr, "%s in %s line %lld\n", status_message(warning),
                status_filename(warning), status_line(warning));
        }

        status_reset();
    }

    templ_reset();

    return 0;
}
```

## jinja template examples

[data](https://github.com/Urquelle/templ/tree/dev/data) folder contains a couple jinja templates with statements that are supported by the implementation so far.

```jinja
{% extends "template.tpl" if true %}

{% block title %}
    main - {{ default_title }}
{% endblock %}

{% block main %}
    {{ super() }}

    {% include "literals.tpl" without context %}
    {% include "exprs.tpl"    with    context %}
    {% include "stmts.tpl"    without context %}
    {% include "utf8.tpl"     without context %}
    {% include "filter.tpl"   with    context %}
    {% include "tests.tpl"    without context %}
    {% include "macros.tpl"   without context %}
{% endblock main %}

{% block custom %}
    <div>custom content</div>
{% endblock %}
```

## json

there's a simple, and built-in support for json which is implemented in the
[src/json.cpp](https://github.com/Urquelle/templ/blob/dev/src/json.cpp). the
`json_parse` method, will parse a given json string, and return a `Json`
structure.

`Json` structure can be fed to `templ_var` method and get a `Templ_Var *` instance in return, which can
be used in `templ_render` context.

```cpp
    Json json = json_parse(R"foo([
        {
            "name": "noob",
            "age" : "25",
            "address": {
                "city": "frankfurt",
                "street": "siegerstr. 2"
            }
        },
        {
            "name": "reinhold",
            "age" : "23",
            "address": {
                "city": "leipzig",
                "street": "mozartstr. 20"
            }
        }
    ])foo");

    Templ *templ = templ_compile_string("{{ users[0].name }}: {{ users[0].address.city }} -- {{ users[1].name }}: {{ users[1].address.city }}");

    Templ_Var *users = templ_var("users", json);
    Templ_Vars vars = templ_vars();

    templ_vars_add(&vars, users);

    char *result = templ_render(templ, &vars);
```

## unicode

`templ` supports unicode with the utf-8 encoding for string literals as well as names. be aware though that right now only limited amount of transformation in filters is supported.

below is a list of character ranges which have lower-/uppercase conversion support.

* [basic-latin](https://en.wikipedia.org/wiki/Basic_Latin_%28Unicode_block%29)
* [latin-1 supplement](https://en.wikipedia.org/wiki/Latin-1_Supplement_%28Unicode_block%29)
* [latin extended-a](https://en.wikipedia.org/wiki/Latin_Extended-A)
* [cyrillic](https://en.wikipedia.org/wiki/Cyrillic_alphabets)

```
ABCDEFGHIJKLMNOPQRSTUVWXYZÄÜÖẞ
АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ
ÆÅÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞĀĂĄĆĈĊČĎĐĒĔĖĘĚĜĞĠĢĤĦĨĪĬĮİĲĴĶŸ
ŁŃŅŇ

abcdefghijklmnopqrstuvwxyzäüöß
абвгдеёжзийклмнопрстуфхцчшщъыьэюя
æåçèéêëìíîïðñòóôõöøùúûüýþāăąćĉċčďđēĕėęěĝğġģĥħĩīĭįıĳĵķÿ
łńņň
```

characters that are not supported will be printed back as they are.

[table](https://www.utf8-chartable.de/unicode-utf8-table.pl) with all character blocks.

### template

```jinja
{% set シ个 = "原ラ掘聞" %}
{{ シ个 }}
{% set приветствие = "здравствуйте" %}
{{ приветствие }}
{{ "🤩✨🥰" * 10 }}
```

### output

```jinja
原ラ掘聞
здравствуйте
🤩✨🥰🤩✨🥰🤩✨🥰🤩✨🥰🤩✨🥰🤩✨🥰🤩✨🥰🤩✨🥰🤩✨🥰🤩✨🥰
```

## expressions

below is a list of expressions that are supported right now

### literals

#### string literals

string literals are supported with quotation marks as well as with apostrophe.

```jinja
"string literal"
'also string literal'
```
#### numbers

integer and floating point numbers are supported.

```jinja
42
42.0
```

#### lists

list literals start with an opening bracket and contain a comma separated list of elements, which in turn have to be a valid jinja expression.

```jinja
['europe', 'asia', 'australia']
```

lists can be assigned as a value to a variable and be used in a `for` statement as both, literals and variables.

```jinja
{% for it in ['europe', 'asia', 'australia'] %}
    {{ it }}
{% endfor %}

{% set continents = ['europe', 'asia', 'australia'] %}
{% for it in continents %}
    {{ it }}
{% endfor %}
```

#### tuple

tuple are basically lists with the exception of being read-only.

```jinja
('x', 'y')
```

#### dictionaries

dictionaries are supported as expressions in assignments and `in` expression in `for` loops.

```jinja
{% set d = {'name': 'adam', 'age': '30'} %}

{% for it in {'name': 'eve', 'age': '25'} %}
    ...
{% endfor %}
```

#### booleans

boolean values

```jinja
true
false
```

#### math

```jinja
3+5*7/2
```

below is a list of supported math operations

```jinja
+
-
*
**
/
//
%
```

## statements

list of supported statements

### block

blocks are supported with an optional `name` in the `endblock` statement. as the inheritance of templates is also supported, parent block's content can be overwritten entirely, or be included alongside your own content with the `super()` method.

```jinja
{% block <name> %}
{% endblock <name> %}
```
### do

```jinja
{% do <expression> %}
```

### extends

```jinja2
{% extends "<template>" <if expr> %}
```

### filter

```jinja2
{% filter <name1> | <name2> %}
    <anweisungen>
{% endfilter %}
```

### for

`for` statement supports multiple return values, `else` branch, **almost** all `loop` variables, `break` and `continue` statements.

```jinja
{% for <iterator> in <menge> %}
    <anweisungen>
{% else %}
    <anweisungen>
{% endfor %}
```

the following loop variables can be used inside a `for` loop:

* loop.index
* loop.index0
* loop.revindex
* loop.revindex0
* loop.first
* loop.last
* loop.length
* loop.cycle
* loop.depth
* loop()


### if

flow control statement `if` is supported with `elif` and `else` branches.

```jinja
{% if <condition> %}
    <statements>
{% elif <condition> %}
    <statements>
{% else %}
    <statements>
{% endif %}
```

you can use any valid jinja and supported expressions as *condition* that have a boolean value as result.

    true
    false
    1 < 2
    a is eq "foo"
    firstname == "arminius" and lastname == "der cherusker"

### import

```jinja2
{% import "<template>" as <sym> %}
{% from "<template>" import <sym1> as <alias1> %}
```

### include

additional templates can be included into a template. `include` statement supports `if` expression, and the additional annotations `with context`, `without context`, `ignore missing`.

```jinja2
{% include "<template>" <if ausdruck> %}
```

### macro

```jinja2
{% macro <name>(parameter, ...) %}
{% endmarcro %}
```

### raw

```jinja2
{% raw %}
{% endraw %}
```

### set

```jinja
{% set <lvalue expression> = <rvalue expression> %}
```

### set block

```jinja
{% set <lvalue expression> %}
    <statements>
{% endset %}
```

## filter

ongoing process of implementing the vast amount of filters. the following filters are implemented in [dev](https://github.com/Urquelle/templ/tree/dev):

* abs
* attr
* batch
* capitalize
* center
* default
* dictsort
* escape
* filesizeformat
* first
* float
* format
* lower
* max
* min
* reject
* rejectattr
* reverse
* select
* selectattr
* slice
* sum
* truncate
* upper

## tests

most of the tests present in the jinja2 spec are already implemented in [dev](https://github.com/Urquelle/templ/tree/dev).

* callable
* defined
* devisibleby
* equal
* even
* ge
* gt
* in
* iterable
* le
* lt
* mapping
* ne
* none
* number
* odd
* sameas
* sequence
* string
* undefined

## introspection

lightweight introspection is built in. for it to work you have to provide a meta json file, which describes the data layout of the given raw pointer.

in the example below the `User` struct

```cpp
struct Address {
    char *city;
};

struct User {
    int age;
    char *name;
    Address address;
};
```

is described with the following json

```json
[
    {
        "name"  : "age",
        "offset": 0,
        "kind"  : 1
    },
    {
        "name"  : "name",
        "offset": 8,
        "kind"  : 0
    },
    {
        "name"  : "address",
        "offset": 16,
        "kind"  : 5,
        "format": [{
            "name"  : "city",
            "offset": 0,
            "kind"  : 0
        }]
    }
]
```

to use the c++ data in template you first have to create a `Templ_Var *` instance, which can be done as follows:

```cpp
User user = { 20, "alex", { "paris" } };
Templ_Var *user = templ_var("user", &user, json_parse(json_format_string));
...
```

the `kind` field in the meta json file has to be the int value from the `Json_Node_Kind` enum

```cpp
enum Json_Node_Kind {
    JSON_STR,
    JSON_INT,
    JSON_FLOAT,
    JSON_BOOL,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_NULL,
};
```

## custom procs

templ supports registering of custom procedures which then can be executed in a template. you can
register three types of procedures.

### global

global procs are standalone and not bound to any context. they can be used everywhere you can use builtin procs as well.

```cpp
PROC_CALLBACK(custom_hello) {
    using namespace templ::devapi;

    return val_str("hello, world");
}

int main() {
    using namespace templ::api;
    using namespace templ::devapi;

    templ_init(MB(100), MB(100), MB(100));
    templ_register_proc("hello", custom_hello, 0, 0, type_str);
    ...
}
```

and then later in the template:

```jinja
{{ hello() }}
{% set var = hello() }}
```

### type

you can also register procedures that are bound to a certain type. for that the following api calls can be used, that
you can import separately by `using namespace templ::devapi`:

```cpp
templ::templ_register_any_proc;    // register procedure for every datatype
templ::templ_register_seq_proc;    // register procedure for sequence datatypes only
templ::templ_register_num_proc;    // register procedure for numeric datatypes only
templ::templ_register_bool_proc;   // register procedure for bool
templ::templ_register_dict_proc;   // register procedure for dictionary
templ::templ_register_float_proc;  // register procedure for float
templ::templ_register_int_proc;    // register procedure for int
templ::templ_register_range_proc;  // register procedure for range
templ::templ_register_list_proc;   // register procedure for list
templ::templ_register_string_proc; // register procedure for string
```

note: all type procs can also be used as filters on that type, so `{{ "abc".my_custom_proc() }}` is the same as {{ "abc" | my_custom_proc }}

### tester

tester are used in the `is` expression as a procedure to test against a given value, and have to evaluate to bool.
