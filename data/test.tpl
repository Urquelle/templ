{% extends "main.tpl" %}

{% block title %}
    test - {{ default_title }}

    {% set a = "test" %}
    <div class="bla">{{ a }}</div>
{% end %}

{{ user.name | upper }} - {{ user.age }}

{% for it in 0..5 %}
    <div>{{ it }}</div>
{% end %}
