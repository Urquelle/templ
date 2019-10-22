# Jinja2 C++

## ausdrücke

nachfolgend ist eine liste der ausdrücke (expressions), die soweit unterstützt werden

### literale

    "tolles wetter"
    'tolles wetter'

zeichenketten werden sowohl mit doppelten, als auch mit einfachen anführungszeichen unterstützt.

    42
    42.0

ganzzahlen und fließkommazahlen werden unterstützt.

    ['europa', 'asien', 'australien']

listen von ausdrücken werden unterstützt. listen können variablen zugewiesen, oder zur 
verwendung in `for` schleifen direkt angegeben werden.

    ('x', 'y')

tupel werden unterstützt. überflüssiges komma nach dem letzten element wird geschluckt.

    ['name': 'adam', 'alter': '30']

dictionaries werden noch nicht in vollem umfang unterstützt.

    true
    false

boolische angaben werden unterstützt.

### mathematische ausdrücke

    3+5*7/2

folgende mathematische operatoren können verwendet werden.

    +
    -
    *
    /
    %
