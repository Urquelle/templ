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

{% set text_lde = "abcdefghijklmnopqrstuvwxyzäüöß" %}
{% set text_lru = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя" %}
{% set text_lbasic_lat = "æåçèéêëìíîïðñòóôõöøùúûüýþāăąćĉċčďđēĕėęěĝğġģĥħĩīĭįıĳĵķÿ" %}
{% set text_lsupp_lat = "ĺļľłńņňŋ" %}

{% filter upper %}
    {{ text_lde }}
    {{ text_lru }}
    {{ text_lbasic_lat }}
    {{ text_lsupp_lat }}
{% endfilter %}

{% set text_ude = "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÜÖẞ" %}
{% set text_uru = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" %}
{% set text_ubasic_lat = "ÆÅÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞĀĂĄĆĈĊČĎĐĒĔĖĘĚĜĞĠĢĤĦĨĪĬĮİĲĴĶŸ" %}
{% set text_usupp_lat = "ĹĻĽŁĿŃŅŇŊ" %}

{% filter lower %}
    {{ text_ude }}
    {{ text_uru }}
    {{ text_ubasic_lat }}
    {{ text_usupp_lat }}
{% endfilter %}

{{ '' | default("bla") }}
{{ false | default("true") }}
{{ false | default("true", true, foo = "bar") }}
{{ '<div>"&escape"</div>' | e }}
{{ "name %s alter %d und %.2f" | format(user.name, user.age, 7.53) | upper }}
{{ "lorem ipsum dolor" | truncate(length = 9, end = " ???") }}

{% for key, val in {'b': 'zzz', 'z': 'ccc', 'c': 'fff', 'f': 'aaa', 'a': 'bbb'} | dictsort(reverse = false, by = "value") %}
    {{ key }} = {{ val }}
{% endfor %}
