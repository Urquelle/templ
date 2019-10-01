{% extends "main.tpl" %}

{% block title %}
    test - {{ default_title }}
{% end %}

{% set a = "test" %}
<div class="bla">{{ a }}</div>

{{ user.name | upper }} - {{ user.age }}

{% for it in 0..5 %}
    <div>{{ it }}</div>
{% end %}
