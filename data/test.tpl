{% extends "main.tpl" if 1 is eq 1 else "main2.tpl" %}

{% block title %}
    test - {{ default_title }}
{% endblock %}

{% block main %}
    {{ super() }}

    {% set a = "test" %}
    <div class="bla">{{ a }}</div>

    {% set b = 2 %}
    {% if b is eq 2 %}
        {{ user.name if b is eq 2 else "rumpelstilzchen" }}
    {% endif %}

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
