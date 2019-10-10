{% extends "main.tpl" if 1 is eq 1 %}

{% block title %}
    test - {{ default_title }}
{% endblock %}

{% block main %}
    {{ super() }}

    {% set a = "test" %}
    <div class="bla">{{ a }}</div>

    {% set b = 2 %}
    {% if b is eq 2 %}
        {{ user.name if b is eq 2 else "protogermane" }}
    {% endif %}

    {% for it in 0..5 %}
        <div>{{ it }}</div>
    {% endfor %}
{% endblock main %}

{% block foo %}
    <div>hier kommt foo hin</div>
{% endblock %}
