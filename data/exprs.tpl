<div>exprs.tpl</div>

{{ users[0].name }} ist {{ users[0].age }} jahre alt und wohnt in {{ users[0].address.city }}
-1*5%2 = {{ -1*5%2 }}
20/7 = {{ 20/7 }}
20//7 = {{ 20//7 }}
"hallo" = {{ "hallo" }}
"xy" * 10 = {{ "x\"y" * 10 }}
3 ** 2 = {{ 3 ** 2 }}
[(1, 2), (3, 4)][1][0] = {{ [(1, 2), (3, 4)][1][0] }}
"aシbcd"[1] = {{ "aシbcd"[1] }}
{{ "🤩✨🥰" * 10 }}
{{ 10 * "🤩✨🥰" }}
{{ "abc" ~ users[0].name ~ "def" }}
{#{ a in ["blub", "blab", "blob"] }#}
{{ (2 - 5).abs() }}
{{ hello().upper() }}
