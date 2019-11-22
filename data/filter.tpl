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
{{ lipsum() | truncate(length = 9, end = " ???", killwords=true) }}

{% for key, val in {'b': 'zzz', 'z': 'ccc', 'c': 'fff', 'f': 'aaa', 'a': 'bbb'} | dictsort(reverse = false, by = "key") %}
    {{ key }} = {{ val }}
{% endfor %}

filesize {{ 43 | filesizeformat }}
filesize {{ 15243 | filesizeformat(true) }}
filesize {{ 34252342 | filesizeformat }}

{{ ["fff", "bbb", "sss"] | first }}
{{ "abc" | first }}

{{ "3.14" | float }}
{{ "abc" | float(3.14) }}
