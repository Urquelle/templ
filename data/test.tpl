{% extends "bla.tpl" %}

<div class="blö">töst</div>

{% block foo %}
    <a name="foo">123</a>
    {% block title %}
        <div>title</div>
    {% end %}
{% end %}

{% filter uppercase | escape "html" %}
    irgendein text fuer den filter und so
{% end %}

{{ a ? +b : c }}
{{ foozle.bla | uppercase | ellipsis 20 "..." }}
{# kommentar #}
{% for bla in 1..10 %}
    {{ x.y }}
    {% if a == b %}
        <div>{{ a }}</div>
    {% else if a < b %}
        <div>{{ b }}</div>
    {% end %}
{% end %}
