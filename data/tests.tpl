{% if var is defined %}
{% endif %}

{% if [1, 2, 3] is iterable %}
{% endif %}

{% if "a" is in ["c", "b", "a"] %}
{% endif %}

{% if 1 is mapping %}
    1 ist ein mapping
{% elif [1, 2, 3] is mapping %}
    liste ist ein mapping
{% else %}
    nichts ist ein mapping
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

{% if var is undefined %}
    var ist nicht definiert
{% else %}
    var ist definiert
{% endif %}

