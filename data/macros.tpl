{% set macro_name = "macros für den alltag" %}

{% macro test1() %}
    <div>test1 macro inhalt</div>
{% endmacro %}

{% macro test2() %}
    <div>test2 macro inhalt</div>
{% endmacro %}

{% macro printf(format, arg = "lorem ipsum") %}
    {{ format }} - {{ arg }}
{% endmacro %}
