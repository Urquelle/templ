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
{{ "center me" | center(25) }}

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

{{ "name %s alter %d und %.2f" | format(users[0].name, users[0].age, 7.53) | upper }}
{{ "name { name }, alter { alter } und {}" | format(name = users[0].name, alter = users[0].age, 7.53) | upper }}
{{ "name {0}, alter {1} und {1}" | format(users[0].name, users[0].age, 7.53) | upper }}
{{ "{zahl:.2%}" | format(zahl=3.1415) }}

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

{% for group in users | groupby("age") %}
    age {{ group.grouper }}
    <hr>
    {% for user in group.list %}
        <li>{{ user.name }} :{{ user.address.city }}</li>
    {% endfor %}
{% endfor %}

{% for grouper, list in users | groupby("age") %}
    age {{ grouper }}
    <hr>
    {% for user in list %}
        <li>{{ user.name }} :{{ user.address.city }}</li>
    {% endfor %}
{% endfor %}

{% filter indent(first=true, blank=true) %}
first line
     
  last line🤹
{% endfilter %}

{{ "53" | int }}
{{ "abc" | int }}
{{ "abc" | int(10) }}
{{ "110" | int(base=2) }}

{% set lastlist = [1, 2, 3] %}
{{ lastlist | last }}
{{ {'a': 'aaa', 'b': 'bbb'} | last }}

{{ lastlist | length }}

{% for c in "last l🤹ine" | list %}
    --{{ c }}--
{% endfor %}

{% for it in 1 | list %}
    --{{ it }}--
{% endfor %}

{% for it in [1, 2, 3] %}
    --{{ it }}--
{% endfor %}

{% for it in {'vorname': 'friedrich', 'nachname': 'barbarossa' } %}
    --{{ it }}--
{% endfor %}

{{ ['a', 'b', 'c'] | join }}
{{ ['a', 'z'] | join(d="🤹") }}
{{ users | join(attribute="name", d="*") }}

{% for it in users | map(attribute="name") %}
    --{{ it }}--
{% endfor %}

--{{ [1, 2, 3] | max }}--
--{{ ['a', 'D', 'c'] | max(case_sensitive=false) }}--

{% set max_user = users | max(case_sensitive=false, attribute="name") %}
--{{ max_user.name }}--

--{{ [1, 2, 3] | min }}--
--{{ ['a', 'D', 'c'] | min(case_sensitive=false) }}--

{% set min_user = users | min(case_sensitive=false, attribute="name") %}
--{{ min_user.name }}--

{{ users | pprint }}
{{ range | pprint }}

--{{ [1, 2, 3, 4, 5] | random }}--
--{{ [1, 2, 3, 4, 5] | random }}--
--{{ [1, 2, 3, 4, 5] | random }}--
--{{ "abcdef" | random }}--

-- reject --
{% for it in [1, 2, 3] | reject("divisibleby", 3) %}
    --{{ it }}--
{% endfor %}

{% for it in [1, 2, 3, 4, 5] | reject("odd") %}
    --{{ it }}--
{% endfor %}

{% for it in [1, 2, 3, 4, 5] | reject("even") %}
    --{{ it }}--
{% endfor %}

{% for user in users | rejectattr("name", "eq", "noob") %}
    --{{ user.name }}--
{% endfor %}

{% for user in users | rejectattr("age", "gt", 20) %}
    --{{ user.name }}: {{ user.age }}--
{% endfor %}

-- select --
{% for it in [1, 2, 3] | select("divisibleby", 3) %}
    --{{ it }}--
{% endfor %}

{% for it in [1, 2, 3, 4, 5] | select("odd") %}
    --{{ it }}--
{% endfor %}

{% for it in [1, 2, 3, 4, 5] | select("even") %}
    --{{ it }}--
{% endfor %}

{% for user in users | selectattr("name", "eq", "noob") %}
    --{{ user.name }}--
{% endfor %}

{% for user in users | selectattr("age", "gt", 20) %}
    --{{ user.name }}: {{ user.age }}--
{% endfor %}

{{ "schifffahrtsgesellschaft" | replace("schiff", "segel") }}
{{ "waldstraßewald" | replace("wald", "see", 1) }}

{% for it in [1, 2, 3] | reverse %}
    --{{ it }}--
{% endfor %}

{{ "abcdef" | reverse }}

-- batch --
{% for b in [1, 2, 3, 4, 5] | batch(2) %}
    --{{ loop.index }}--
    {{ b[0] }} * {{ b[1] }}
{% endfor %}
-- slice --
{% for b in [1, 2, 3, 4, 5] | slice(2) %}
    --{{ loop.index }}--
    {{ b[0] }} * {{ b[1] }} * {{ b[2] }}
{% endfor %}

-- sum --
{{ [1, 2, 3, 4, 5] | sum(start=3) }}
{{ users | sum(attribute="age") }}

-- sort --
{% set sss = [1, 3, 7, 5, 2] %}
{% for it in sss | sort(reverse=true) %}
    --{{ it }}--
{% endfor %}

-- title --
{{ "eine überschrift" | title }}

{{ 2545 | convert(prefix="16b") }}
{{ 5531 | convert(to="roman") }}
{{ 32   | convert(to="bin", prefix="2b") }}

{{ users | sort(attribute="name", reverse=true) | tojson(indent=3) }}

{{ "   abc   " | trim }}

{{ "abc abc <a href=\"index.html\">index</a>" | striptags }}
{{ "abc abc <a href='index.html'>index</a>" | striptags }}
{{ 'abc abc <a href=\'index.html\'>index</a>' | striptags }}
{{ 'abc abc <a href="index.html">index</a>' | striptags }}

-- unique --
{{ [1, 5, 2, 1, 3] | unique | sort | pprint }}
-- unique case sensitive --
{{ ["c", "a", "z", "Z", "c", "b"] | unique(case_sensitive=true) | pprint }}
-- unique case insensitive --
{{ ["c", "a", "z", "Z", "c", "b"] | unique | pprint }}
{{ users | unique(attribute="name") | pprint }}
-- wc --
{{ "veni vidi vici" | wordcount }}

-- urlencode --
{{ "test@test.de" | urlencode }}

-- urlize --
{{ "url1 http://www.test.de with text" | urlize }}
{{ "url2 http://www.test.de with text" | urlize(10) }}
{{ "url3 http://www.test.de with text" | urlize(15, true) }}
{{ "url4 http://www.test.de:8080/ with text" | urlize(15, true, target="_blank") }}
{{ "url5 http://www.test.de:8080/index.html with text" | urlize }}
{{ "url6 http://www.test.de:8080/index.html?a=b&c=d with text" | urlize }}
{{ "url7 http://www.test.de:8080/?a=b with text" | urlize }}
{{ "url8 http://www.test.de:8080?a=b with text" | urlize }}
{{ "url9 http://user@www.test.de:8080?a=b with text" | urlize }}
{{ "url10 http://user:pass@www.test.de:8080?a=b with text" | urlize }}
{{ "url11 http://:pass@www.test.de:8080?a=b with text" | urlize }}
{{ "url12 http://user:@www.test.de:8080?a=b with text" | urlize }}
{{ "url13 http://:@www.test.de:8080?a=b with text" | urlize }}

-- wordwrap --
{{ lipsum() | wordwrap }}

-- xmlattr --
<xml{{ users[0] | xmlattr }}>
