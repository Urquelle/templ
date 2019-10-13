{% extends "main.tpl" if 1 is eq 1 else "main2.tpl" %}

{% block title %}
    test - {{ default_title }}
{% endblock %}

{% block main %}
    <h2>super()</h2>
    {{ super() }}

    <h2>macro</h2>
    {{ printf("formatstring", "argumentzeugs") }}
    {{ printf("zeugs %s zum formatieren") }}

    <h2>for mit zeichenketten</h2>
    {% for it in "d".."i" %}
        <div>{{ it }}</div>
    {% endfor %}

    <h2>set anweisung</h2>
    {% set a = "test" %}
    <div class="bla">{{ a }}</div>

    <h2>if anweisung</h2>
    {% set b = 2 %}
    {% if b is eq 2 %}
        {{ user.name if b is eq 2 else "rumpelstilzchen" }}
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
