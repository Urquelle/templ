{% set a = "test" %}
<div class="bla">{{ a }}</div>

{% block foo %}
    <a name="foo">123</a>
{% end %}

{{ user.name }} - {{ user.age }}

{% for it in 0..5 %}
    <div>{{ it }}</div>
{% end %}
