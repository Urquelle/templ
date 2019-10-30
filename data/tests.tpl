{% if [1, 2, 3] is iterable %}
{% endif %}

{% if "a" is in ["c", "b", "a"] %}
{% endif %}

{% if 5 is number %}
{% endif %}

{% if 2 is odd %}
{% endif %}

{# sameas test #}
{% set sameastest_a = "abcd" %}
{% set sameastest_b = sameastest_a %}

{% if sameastest_a is not sameas sameastest_b %}
{% endif %}

{% if [1, 2, 3] is sequence %}
{% endif %}

