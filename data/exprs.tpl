<div>exprs.tpl</div>

{{ true }}
{{ false }}
{{ none }}
{{ user.name }} ist {{ user.age }} jahre alt und wohnt in {{ user.address.city }}
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
{#{ a in ["blub", "blab", "blob"] }#}
