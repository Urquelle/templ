<h2>filter.tpl</h2>

{{ -5 | abs }}

{% set set = {'a': 10, 'b': "bla"} %}
{{ set | attr('b') }}

{% filter upper %}
    test des filters upper
{% endfilter %}

{{ '' | default("bla") }}
{{ false | default("true") }}
{{ false | default("true", true, foo = "bar") }}
{{ '<div>escape</div>' | e }}
{{ "öname %s alter %d und %.2f" | format(user.name, user.age, 7.53) | upper }}
{{ "lorem ipsum dolor" | truncate(length = 9, end = " ???") }}
