{% extends "main.tpl" %}

{% block title %}
    test - {{ default_title }}
{% end %}

{% block main %}
    {% set a = "test" %}
    <div class="bla">{{ a }}</div>

    {% set b = 2 %}
    {% if b is eq 2 %}
        {{ user.name }}
    {% end %}

    {% for it in 0..5 %}
        <div>{{ it }}</div>
    {% end %}
{% end %}
