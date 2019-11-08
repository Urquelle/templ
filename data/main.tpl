{% extends "template.tpl" if true %}

{% block title %}
    main - {{ default_title }}
{% endblock %}

{% block main %}
    {{ super() }}

    {% include "literals.tpl" without context %}
    {% include "exprs.tpl"    with    context %}
    {% include "stmts.tpl"    without context %}
    {% include "utf8.tpl"     without context %}
    {% include "filter.tpl"   with    context %}
    {% include "tests.tpl"    without context %}
    {% include "macros.tpl"   without context %}
{% endblock main %}

{% block custom %}
    <div>custom content</div>
{% endblock %}
