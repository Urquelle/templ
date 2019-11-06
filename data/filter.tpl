{% filter upper %}
    test des filters upper
{% endfilter %}

{{ '' | default("bla") }}
{{ false | default("true") }}
{{ false | default("true", true) }}
{{ '<div>escape</div>' | e }}
{{ "öname %s alter %d und %.2f" | format(user.name, user.age, 7.53) | upper }}
{{ "lorem ipsum dolor" | truncate(length = 9, end = " ???") }}
