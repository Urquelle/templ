{% extends "template.tpl" %}

{% block title %}
    test - {{ default_title }}
{% endblock %}

{% block main %}
    {{ super() }}

    {% include "literals.tpl" without context %}
    {% include "exprs.tpl"    without context %}
    {% include "stmts.tpl"    without context %}
    {% include "utf8.tpl"     without context %}
    {% include "filter.tpl"   without context %}
    {% include "tests.tpl"    without context %}
    {% include "macros.tpl"   without context %}

    {% for it in range(1, 3) %}
        {{ it }}
    {% endfor %}

    <h2>templ vars</h2>
    {{ user.name }} ist {{ user.age }} jahre alt und wohnt in {{ user.address.city }}

    <h2>tuple test</h2>
    {% set tuple = ("europa" | upper, "asien", "amerika") %}
    {% for kontinent in tuple %}
        idx: {{ loop.index }} idx0: {{ loop.index0 }}
        revidx: {{ loop.revindex }} revidx0: {{ loop.revindex0 }}
        first: {{ loop.first }} last: {{ loop.last }}
        length: {{ loop.length }} cycle: {{ loop.cycle('odd', 'even') }}
        <div>kontinent:</div><div>{{ kontinent }}</div>
    {% endfor %}

    <h2>test default</h2>
    {{ '' | default("bla") }}
    {{ '<div>escape</div>' | e }}

    {{ "name %s alter %d und %.2f" | format(user.name, user.age, 7.53) | upper }}

    {% set elem1, elem2 = [1, 2] %}
    elem1 = {{ elem1 }}
    elem2 = {{ elem2 }}

    {% for eins, zwei in [("eins", "zwei"), ("drei", "vier")] %}
        eins = {{ eins }} und zwei = {{ zwei }}
    {% endfor %}

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
