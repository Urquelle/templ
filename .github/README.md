<p align="center">
    <img src="https://github.com/NoobSaibot/templ/blob/dev/static/logo_250.png" />
    <br /><br />
    <a href="https://github.com/NoobSaibot/templ/releases/latest">
        <img src="https://img.shields.io/github/v/release/NoobSaibot/templ" />
    </a>
    <a href="https://isocpp.org/">
        <img src="https://img.shields.io/badge/language-C++-blue.svg" />
    </a>
    <a href="https://raw.githubusercontent.com/NoobSaibot/templ/dev/LICENSE">
        <img src="https://img.shields.io/github/license/NoobSaibot/templ" />
    </a>
    <a href="https://en.wikipedia.org/wiki/Made_in_Germany">
        <img src="https://img.shields.io/badge/made%20in-germany-red" />
    </a>
</p>

<hr />

<table>
    <tr>
        <th>status</th>
        <th>statistik</th>
    </tr>
    <tr>
        <td>
            <table>
                <tr>
                    <td>travis build:</td>
                    <td align="right">
                        <a href="https://travis-ci.org/NoobSaibot/templ">
                            <img src="https://travis-ci.org/NoobSaibot/templ.svg?branch=dev" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>appveyor build:</td>
                    <td align="right">
                        <a href="https://ci.appveyor.com/project/NoobSaibot/templ/branch/dev">
                            <img src="https://ci.appveyor.com/api/projects/status/d5vpfm3mtao94aow/branch/dev?svg=true" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>github actions:</td>
                    <td align="right">
                        <img src="https://github.com/NoobSaibot/templ/workflows/windows-build/badge.svg" />
                    </td>
                </tr>
            </table>
        </td>
        <td>
            <table>
                <tr>
                    <td>offene aufgaben:</td>
                    <td align="right">
                        <a href="https://github.com/NoobSaibot/templ/issues">
                            <img src="https://img.shields.io/github/issues/NoobSaibot/templ" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>code qualität:</td>
                    <td align="right">
                        <a href="https://www.codacy.com/manual/NoobSaibot/templ?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=NoobSaibot/templ&amp;utm_campaign=Badge_Grade">
                            <img src="https://api.codacy.com/project/badge/Grade/f4e97144ea6d43b3a38fc34e9b5e50b7" />
                        </a>
                    </td>
                </tr>
                <tr>
                    <td>code zeilen:</td>
                    <td align="right"><img src="https://tokei.rs/b1/github/NoobSaibot/templ" /></td>
                </tr>
            </table>
        </td>
    </tr>
</table>

## einfaches beispiel

nachfolgend ist ein einfaches beispiel, indem eine datenstruktur erstellt wird, die in den kontext der
template engine gestellt wird und ein einfaches string template gerendert wird.

```c++
#include "templ.cpp"

int
main(int argc, char **argv) {
    using namespace templ::api;

    templ_init(MB(100), MB(100), MB(100));

    Templ_Vars vars = templ_vars();
    Templ_Var *name = templ_var("name", val_str("noob"));
    templ_vars_add(&vars, name);

    Parsed_Templ *templ = templ_compile_string("hallo {{ name }}");
    char *result = templ_render(templ, &vars);

    if ( status_is_not_error() ) {
        os_file_write("test.html", result, os_strlen(result));
    } else {
        fprintf(stderr, "fehler aufgetreten in der übergebenen zeichenkette: %s\n", status_message());
        status_reset();
    }

    templ_reset();

    return 0;
}
```

## weitere beispiele

im [data](https://github.com/NoobSaibot/templ/tree/dev/data) verzeichnis befinden sich einige jinja2 templates, die weitestgehend alle angaben nutzen, die von `templ` aktuell unterstützt werden.

## unicode

templ unterstützt sowohl ascii als auch unicode in utf-8 kodierung sowohl für variablen namen, als auch
für die string literale.

https://github.com/NoobSaibot/templ/blob/db3c1ff178b72e1a3ca377d18d5b82a6264e37d3/data/utf8.tpl#L1-L8

## ausdrücke

nachfolgend ist eine liste der ausdrücke (expressions), die soweit unterstützt werden

### literale

```jinja2
"tolles wetter"
'tolles wetter'
```

zeichenketten werden sowohl mit doppelten, als auch mit einfachen anführungszeichen unterstützt.

```jinja2
42
42.0
```

ganzzahlen und fließkommazahlen werden unterstützt.

```jinja2
['europa', 'asien', 'australien']
```

listen von ausdrücken werden unterstützt. listen können variablen zugewiesen, oder zur 
verwendung in `for` schleifen direkt angegeben werden.

```jinja2
('x', 'y')
```

tupel werden unterstützt. überflüssiges komma nach dem letzten element wird geschluckt.

```jinja2
{'name': 'adam', 'alter': '30'}
```

dictionaries werden noch nicht in vollem umfang unterstützt.

```jinja2
true
false
```

boolische angaben werden unterstützt.

### mathematische ausdrücke

```jinja2
3+5*7/2
```

folgende mathematische operatoren können verwendet werden.

```jinja2
+
-
*
**
/
//
%
```

## anweisungen

folgende anweisungen werden derzeit unterstützt.

### if

```jinja2
{% if <bedingung> %}
    <anweisungen>
{% elif <bedingung> %}
    <anweisungen>
{% else %}
    <anweisungen>
{% endif %}
```

als *bedingung* können ausdrücke verwendet werden, die als resultat einen boolischen wert ergeben.

    true
    false
    1 < 2
    a is eq "foo"
    vorname == "arminius" and nachname == "der cherusker"

### for

```jinja2
{% for <iterator> in <menge> %}
    <anweisungen>
{% else %}
    <anweisungen>
{% endfor %}
```

### block

```jinja2
{% block <name> %}
{% endblock <name> %}
```

### include

```jinja2
{% include "<template>" <if ausdruck> %}
```

### import

```jinja2
{% import "<template>" as <sym> %}
{% from "<template>" import <sym1> as <alias1> %}
```

### extends

```jinja2
{% extends "<template>" %}
```

### filter

```jinja2
{% filter <name1> | <name2> %}
    <anweisungen>
{% endfilter %}
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
