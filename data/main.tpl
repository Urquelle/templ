<html>
    <head>
        {% set default_title = "tmpl" %}
        <title>{% block title %}{% end %}</title>
    </head>
    <body>
        <div class="main">
        {% block main %}
        {% end %}
        </div>
    </body>
</html>
