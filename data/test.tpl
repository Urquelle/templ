{% extends "bla.tpl" %}

<div class="blö">töst</div>

{% block foo %}
    <a name="foo">123</a>
    {% block title %}
        <div>title</div>
    {% end %}
{% end %}

{% filter upper | escape %}
    irgendein text fuer den filter und so
{% end %}

{{ user.name | upper | truncate 20 "..." }}
{# kommentar #}
{% for it in 1..10 %}
    {% if a == b %}
        <div>{{ a }}</div>
    {% else if a < b %}
        <div>{{ b }}</div>
    {% end %}
{% end %}
