{% extends "main.tpl" %}

{% import "macros.tpl" as macros %}
{% from "macros.tpl" import test1 as test_1, test2 %}

{% block title %}
    test - {{ default_title }}
{% endblock %}

{% block main %}
    <h2>super()</h2>
    {{ super() }}

    <h2>unicode</h2>
    {% set シ个 = "原ラ掘聞" %}
    {{ シ个 }}
    {% set приветствие = "здравствуйте" %}
    {{ приветствие }}

    <h2>literale</h2>
    {{ "string literal" }}
    {{ 10 }}
    {{ 5.3 }}

    <h2>templ vars</h2>
    {{ user.name }} ist {{ user.age }} jahre alt und wohnt in {{ user.address.city }}

    <h2>import macros</h2>
    {{ macros.test1() }}
    macro name: {{ macros.test1.name }}
    macro parameter:
    {% for param in macros.printf.arguments %}
        *{{ param }}*
    {% endfor %}

    <h2>import variablen</h2>
    {{ macros.macro_name }}

    {% set foo = 10 %}
    {% if foo <= 5 %}
        ###{{ foo }}###
    {% elif foo > 5 %}
        {{ foo * 10 }}
    {% endif %}

    <h2>tuple test</h2>
    {% set tuple = ("europa" | upper, "asien", "amerika") %}
    {% for kontinent in tuple %}
        idx: {{ loop.index }} idx0: {{ loop.index0 }}
        revidx: {{ loop.revindex }} revidx0: {{ loop.revindex0 }}
        first: {{ loop.first }} last: {{ loop.last }}
        length: {{ loop.length }} cycle: {{ loop.cycle('odd', 'even') }}
        <div>kontinent:</div><div>{{ kontinent }}</div>
    {% endfor %}

    <h2>filter statement</h2>
    {% filter upper %}
        {{ macros.macro_name }}
    {% endfilter %}

    <h2>raw</h2>
    {% raw %}
        {% set foo = "bar" %}
        und dann
    {% endraw %}

    {{ -1*5%2 | abs }}
    {{ 20/7 }}--{{ 20//7 }}
    {{ "hallo" | capitalize }}
    {{ "xy" * 10 }}
    {{ 3 ** 2 }}
    <h2>test default</h2>
    {{ '' | default("bla") }}
    {{ '<div>escape</div>' | e }}

    {{ "name %s alter %d und %.2f" | format(user.name, user.age, 7.53) | upper }}

    <h2>from import macro</h2>
    {{ test_1() }}
    {{ test2() }}

    <h2>macro</h2>
    {{ macros.printf(arg='10, 5', format="%d = %d") }}
    {{ macros.printf("zeugs %s zum formatieren") }}

    <h2>for mit zeichenketten</h2>
    {% for it in "d".."i" %}
        <div>{{ it }}</div>
    {% endfor %}

    <h2>for mit array</h2>
    {% for it in ['grün', 'blau', 'weiss'] %}
        <li>{{ it }}</li>
    {% endfor %}

    <h2>set anweisung</h2>
    {% set a = "test" %}
    <div class="bla">{{ a }}</div>
    {% set a = "tset" %}
    <div class="alb">{{ a }}</div>
    {% set A = "blub" %}
    <div>{{ A }}</div>

    {#{ a in ["blub", "blab", "blob"] }#}

    <h2>if anweisung</h2>
    {% set b = 1 %}
    {% if not not true %}
        {{ user.name if b is not eq 2 else "heinrich" }}
    {% else %}
        {{ "rumpelstilzchen" }}
    {% endif %}

    <h2>array literal</h2>
    {% for it in z %}
        <div>{{ it }}</div>
    {% endfor %}

    <h2>range</h2>
    {% for it in 0..5 %}
        <div>{{ it }}</div>
    {% else %}
        <div>menge ist leer</div>
    {% endfor %}

    {% for it in 0..0 %}
        <div>{{ it }}</div>
    {% else %}
        <div>menge ist leer</div>
    {% endfor %}
{% endblock main %}

{% block foo %}
    <div>hier kommt foo hin</div>
{% endblock %}
