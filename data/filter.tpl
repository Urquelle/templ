{% filter upper %}
    test des filters upper
{% endfilter %}

{{ '' | default("bla") }}
{{ '<div>escape</div>' | e }}
{{ "öname %s alter %d und %.2f" | format(user.name, user.age, 7.53) | upper }}
