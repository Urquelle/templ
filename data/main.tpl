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
    {% include "filter.tpl" %}
    {% include "tests.tpl"    without context %}
    {% include "macros.tpl"   without context %}

    <h2>templ vars</h2>
    {{ user.name }} ist {{ user.age }} jahre alt und wohnt in {{ user.address.city }}

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
