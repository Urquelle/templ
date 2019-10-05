{% extends "main.tpl" %}

{% block title %}
    test - {{ default_title }}

    {% set a = "test" %}
    <div class="bla">{{ a }}</div>
{% end %}

{% set b = 2 %}
{% if b is eq 2 %}
    {{ user.address.street }}
{% end %}

{% for it in 0..5 %}
    <div>{{ it }}</div>
{% end %}
