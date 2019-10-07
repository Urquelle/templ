<html>
    <head>
        {% set default_title = "tmpl" %}
        <title>{% block title %}{% endblock %}</title>
    </head>
    <body>
        <div class="main">
        {% block main %}
            super-block-dass-du-da-bist
        {% endblock %}
        </div>
    </body>
</html>
