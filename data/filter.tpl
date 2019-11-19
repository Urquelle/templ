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
{% set text_llat = "æåçèéêëìíîïðñòóôõöøùúûüýþāăąćĉċčďđēĕėęěĝğġģĥħĩīĭįıĳĵķ" %}
{% filter upper %}
    {{ text_lde }}
    {{ text_lru }}
    {{ text_llat }}
{% endfilter %}

{% set text_ude = "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÜÖẞ" %}
{% set text_uru = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" %}
{% set text_llat = "ÆÅÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞĀĂĄĆĈĊČĎĐĒĔĖĘĚĜĞĠĢĤĦĨĪĬĮİĲĴĶ" %}
{% filter lower %}
    {{ text_ude }}
    {{ text_uru }}
    {{ text_llat }}
{% endfilter %}

{{ '' | default("bla") }}
{{ false | default("true") }}
{{ false | default("true", true, foo = "bar") }}
{{ '<div>escape</div>' | e }}
{{ "name %s alter %d und %.2f" | format(user.name, user.age, 7.53) | upper }}
{{ "lorem ipsum dolor" | truncate(length = 9, end = " ???") }}
