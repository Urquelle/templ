{% extends "main.tpl" %}

{% block title %}
    test - {{ default_title }}
{% endblock %}

{% block main %}
    {{ super() }}

    {% set a = "test" %}
    <div class="bla">{{ a }}</div>

    {% set b = 2 %}
    {% if b is eq 2 %}
        {{ user.name }}
    {% endif %}

    {% for it in 0..5 %}
        <div>{{ it }}</div>
    {% endfor %}
{% endblock main %}

{% block foo %}
    <div>hier kommt foo hin</div>
{% endblock %}
