{% if [1, 2, 3] is iterable %}
{% endif %}

{% if "a" is in ["c", "b", "a"] %}
{% endif %}

{% if 5 is number %}
{% endif %}

{% if 2 is odd %}
{% endif %}

{% set a = "abcd" %}
{% set b = a %}
{% if a is not sameas b %}
{% endif %}

{% if [1, 2, 3] is sequence %}
{% endif %}

{% if 1 is string %}
{% endif %}

