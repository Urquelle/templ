<html>
    {% set default_title = "templ" %}
    {% block head %}
        <head>
            <title>{% block title %}{% endblock %}</title>
        </head>
    {% endblock %}
    <body>
        {% include "header.html" %}

        <div class="main">
        {% block main %}
            main block
        {% endblock %}
        </div>

        {% include "footer.html" %}

        {% include ["a.html", "b.html"] ignore missing without context %}
    </body>
</html>
