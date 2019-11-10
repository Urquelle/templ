<div>stmts.tpl</div>

{# set anweisung #}
{%  set a = 10  %}
{%- set a = 20  %}
{%  set a = 30 -%}
{%- set a = 40 -%}

{%  set a = 10  %}
{%+ set a = 20  %}
{%  set a = 30 +%}
{%+ set a = 40 +%}

{# if mit elif #}
{% if 3 <= 5 %}
    {{ "3 <= 5" }}
{% elif 3 > 5 %}
    {{ 5 * 10 }}
{% endif %}

{% set b = 1 %}
{% if not not true %}
    {{ "heinrich" }}
{% else %}
    {{ "rumpelstilzchen" }}
{% endif %}

{# raw anweisung #}
{% raw %}
    {% set foo = "bar" %}
    und dann
{% endraw %}

{# with anweisung #}
{% with a = a, b = 3 %}
    {% set x = 5 %}
    x = {{ x }}
    a = {{ a }}
{% endwith %}

{# if mit else #}
{% set var_none = none %}
{% if var_none is none %}
    var_none ist none
{% else %}
    var_none ist nicht none
{% endif %}

{% if x is not defined %}
    x ist nicht definiert
{% else %}
    x = {{ x }}
{% endif %}

{# listenoperationen #}
{% set b = [1, 2, 3] %}
b[1] = {{ b[1] }}
{% set b[1] = 5 %}
b[1] = {{ b[1] }}

{# tupel #}
{% set tuple = ("europa" | upper, "asien", "amerika") %}
{% for kontinent in tuple if kontinent == "EUROPA" %}
    idx: {{ loop.index }} idx0: {{ loop.index0 }}
    revidx: {{ loop.revindex }} revidx0: {{ loop.revindex0 }}
    first: {{ loop.first }} last: {{ loop.last }}
    length: {{ loop.length }} cycle: {{ loop.cycle('odd', 'even') }}
    <div>kontinent:</div><div>{{ kontinent }}</div>
{% endfor %}

{# liste mit tupeln #}
{% set c = [(1, 2), (3, 4)] %}
{% set c[1] = (7, 8) %}
{# sollte 7 rauskommen #}
c[1][0] {{ c[1][0] }}

{# unicode #}
{% set d = "シ个abcd" %}
{% raw %}{% set d = "シ个abcd" %}{% endraw %} = {{ d }}
{% set d[3] = "个" %}
{{ "{% set d[3] = " }} "个" {{ "%}" }} = {{ d }}

{# for schleife mit continue und break #}
{% for i in 1..10 recursive %}
    {% if i == 2 %}{% continue %}{% endif %}
    {% if i == 5 %}{% break %}{% endif %}
    {{ i }}
{% endfor %}

{% for eins, zwei in [("eins", "zwei"), ("drei", "vier")] %}
    eins = {{ eins }} und zwei = {{ zwei }}
{% endfor %}

{% for it in "d".."i" %}
    <div>{{ it }}</div>
{% endfor %}

<h2>for mit array</h2>
{% for it in ['grün', 'blau', 'weiss'] %}
    <li>{{ it }}</li>
{% endfor %}

{% for it in 0..5 %}
    <div>{{ it }}</div>
{% else %}
    <div>menge ist leer</div>
{% endfor %}

{% for it in 0..0 %}
    <div>{{ it }}</div>
{% else %}
    <div>menge ist leer</div>
{% endfor %}

<h2>set anweisung</h2>
{% set a = "test" %}
<div class="bla">{{ a }}</div>
{% set a = "tset" %}
<div class="alb">{{ a }}</div>
{% set A = "blub" %}
<div>{{ A }}</div>

{# range mit benamten parametern #}
{% for it in range(stop=7, start=1, step=2) %}
    {{ it }}
{% endfor %}

{# sysproc lipsum #}
{% set lorem = lipsum() %}
{{ lorem }}

{# mehrfachzuweisung #}
{% set elem1, elem2 = [1, 2] %}
elem1 = {{ elem1 }}
elem2 = {{ elem2 }}

{# dict #}
{% set di = { 'a' : 'b', 'c' : 'd' } %}
{{ di['a'] }}
{% for key, value in di %}
    key = {{ key }} val = {{ value }}
{% endfor %}

{% set cyc = cycler("eins", "zwei", "polizei") %}
{{ cyc.current }}
{{ cyc.next() }}
{{ cyc.next() }}
