{% set a = 10 %}

{% if 3 <= 5 %}
    {{ "3 <= 5" }}
{% elif 3 > 5 %}
    {{ 5 * 10 }}
{% endif %}

{% raw %}
    {% set foo = "bar" %}
    und dann
{% endraw %}

{% with a = a, b = 3 %}
    {% set x = 5 %}
    x = {{ x }}
    a = {{ a }}
{% endwith %}

außerhalb: x = {{ x }}
