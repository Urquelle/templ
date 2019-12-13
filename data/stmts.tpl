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
    depth: {{ loop.depth }} depth0: {{ loop.depth0 }}
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
    {% if loop.depth == 1 %}{{ loop([1, 2, 3]) }}{% endif %}
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

{% set j = joiner("<->") %}
1: {{ j() }}
2: {{ j() }}

{% set proc_dict_val = dict(name = "mustermann", vorname = "max") %}
person heißt {{ proc_dict_val.vorname }} {{ proc_dict_val.name }}

{% set ns = namespace(bla = "bla", blö = "blö") %}
{{ ns.bla }}
{{ ns.blö }}

{% set xxx = [1, 2, 3] %}
{% do xxx.append(4) %}
{% for it in xxx %}
    --{{ it }}--
{% endfor %}

{% set yyy | upper %}
    {{ "hallo" }}
    {{ "wie geht's" }}
{% endset %}
{{ yyy }}

{% set test_format = "my name is %s" %}
{{ test_format.format("heinz") }}

{% set test_ausgabe = "my name is %s".format("hannes") %}
{{ test_ausgabe }}

{% set test_lst = ["eins %d", "zwei"] %}
{{ test_lst[0].format(1) }}

{{ "stadt".capitalize() }}

{% macro call_macro() %}
    <div class="parent">
    {% for it in 1..6 %}
        <div class="child-{{ it }}">{{ caller() }}</div>
    {% endfor %}
    </div>
{% endmacro %}
{% call call_macro() %}call content{% endcall %}

{% set user = {"name": "ferdinand"} %}
{% macro call_user_macro() %}
    {{ caller(user) }}
{% endmacro %}

{% call(user) call_user_macro() %}
    -- {{ user.name }}
{% endcall %}
