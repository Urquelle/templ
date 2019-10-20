{#% set Öl = "fisch" %#}
<html>
    {% set default_title = "tmpl" %}
    {% block head %}
    <head>
        <title>{% block title %}{% endblock %}</title>
    </head>
    {% endblock %}
    <body>
        {% include "header.html" %}
        {% set z = ["a", "b", "c"] %}

        <div class="main">
        {% block main %}
            super-block-dass-du-da-bist
        {% endblock %}
        </div>

        {% include "footer.html" %}

        {% include ["a.html", "b.html"] ignore missing without context %}
    </body>
</html>
