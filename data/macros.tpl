{% import "main.macros" as macros %}
{% from "main.macros" import test1 as test_1, test2 %}

<h2>import macros</h2>
{{ macros.test1() }}
macro name: {{ macros.test1.name }}
macro parameter:
{% for param in macros.printf.arguments %}
    param name: {{ param }}
{% endfor %}

<h2>import variablen</h2>
{{ macros.macro_name }}

<h2>from import macro</h2>
{{ test_1() }}
{{ test2() }}

<h2>macro</h2>
{{ macros.printf(arg='10, 5', format="%d = %d") }}
{{ macros.printf("zeugs %s zum formatieren") }}
