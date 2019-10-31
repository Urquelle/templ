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
