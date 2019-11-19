<h2>filter.tpl</h2>

{# abs #}
{{ -5 | abs }}

{# attr #}
{% set set = {'a': 10, 'b': "bla"} %}
{{ set | attr('b') }}
{{ set | attr('x') }}

{# capitalize #}
{% set a = "abc" %}
{{ a | capitalize }}
{{ a }}

{# center #}
{{ "center me" | center }}

{% filter upper %}
    test des filters upper
    äöü
    сюя
{% endfilter %}

{{ '' | default("bla") }}
{{ false | default("true") }}
{{ false | default("true", true, foo = "bar") }}
{{ '<div>escape</div>' | e }}
{{ "öname %s alter %d und %.2f" | format(user.name, user.age, 7.53) | upper }}
{{ "lorem ipsum dolor" | truncate(length = 9, end = " ???") }}
