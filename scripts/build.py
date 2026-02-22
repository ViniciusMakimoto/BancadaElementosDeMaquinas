# -*- coding: utf-8 -*-
import os
import htmlmin
import rjsmin as jsmin
import gzip
import base64
from rcssmin import cssmin

# --- Configuração ---
# Arquivos de origem na pasta 'web'
HTML_IN = "web/index.html"
CSS_IN = "web/style.css"
JS_IN = "web/script.js"
ICON_IN = "web/favicon.ico"

# Arquivo de saída na pasta 'data' para o ESP32 (agora comprimido)
HTML_OUT = "data/index.html.gz"

# --- Leitura dos Conteúdos ---
print("Lendo arquivos de origem da pasta 'web'...")
try:
    with open(HTML_IN, 'r', encoding='utf-8') as f:
        html_content = f.read()
    print(f" -> {HTML_IN} lido com sucesso.")

    with open(CSS_IN, 'r', encoding='utf-8') as f:
        css_content = f.read()
    print(f" -> {CSS_IN} lido com sucesso.")

    with open(JS_IN, 'r', encoding='utf-8') as f:
        js_content = f.read()
    print(f" -> {JS_IN} lido com sucesso.")

except FileNotFoundError as e:
    print(f"ERRO: Arquivo não encontrado - {e.filename}")
    print("Certifique-se que os arquivos index.html, style.css e script.js estão na pasta 'web'.")
    exit(1)

# --- Minificação dos Conteúdos ---
print("Minificando conteúdos...")
try:
    minified_css = cssmin(css_content)
    print(" -> CSS minificado.")
    minified_js = jsmin.jsmin(js_content)
    print(" -> JavaScript minificado.")
except Exception as e:
    print(f"ERRO: Falha ao minificar - {e}")
    exit(1)

# --- Injeção do CSS ---
print("Incorporando CSS...")
css_link_tag = '<link rel="stylesheet" href="style.css">'
style_tag = f"<style>{minified_css}</style>"
# Garante que a substituição ocorra dentro do <head>
head_split = html_content.split('</head>')
if len(head_split) == 2:
    head_part = head_split[0].replace(css_link_tag, style_tag)
    html_content = head_part + '</head>' + head_split[1]
    print(" -> CSS incorporado no <head>.")
else:
    print("AVISO: Tag </head> não encontrada. O CSS não foi incorporado.")

# --- Injeção do JavaScript ---
print("Incorporando JavaScript...")
js_script_tag = '<script src="script.js"></script>'
script_tag = f"<script>{minified_js}</script>"
html_content = html_content.replace(js_script_tag, script_tag)
print(" -> JavaScript incorporado no final do <body>.")

# --- Injeção do Favicon (Base64) ---
if os.path.exists(ICON_IN):
    print("Embutindo favicon.ico...")
    try:
        with open(ICON_IN, "rb") as f:
            b64_icon = base64.b64encode(f.read()).decode('utf-8')
        icon_tag = f'<link rel="icon" type="image/x-icon" href="data:image/x-icon;base64,{b64_icon}">'
        if '</head>' in html_content:
            html_content = html_content.replace('</head>', icon_tag + '</head>')
            print(" -> Favicon embutido no <head>.")
    except Exception as e:
        print(f"AVISO: Falha ao embutir favicon - {e}")

# --- Minificação do HTML Final ---
print("Minificando HTML final...")
final_html = htmlmin.minify(html_content, remove_comments=True, remove_empty_space=True)
print(" -> HTML minificado.")

# --- Compressão Gzip ---
print("Comprimindo o arquivo HTML final...")
try:
    compressed_html = gzip.compress(final_html.encode('utf-8'))
    print(" -> HTML comprimido com Gzip.")
except Exception as e:
    print(f"ERRO: Falha ao comprimir o arquivo - {e}")
    exit(1)

# --- Escrita do Arquivo Final ---
print(f"Escrevendo arquivo final para o ESP32 em {HTML_OUT}...")
try:
    with open(HTML_OUT, 'wb') as f:
        f.write(compressed_html)
    print("\nSUCESSO!")
    print(f"Arquivo '{HTML_OUT}' foi gerado e está pronto para o upload no ESP32.")
    print("A pasta 'data' agora contém o 'index.html.gz' final e a pasta 'lib' do Chart.js.")
    print("A pasta 'web' com os arquivos de desenvolvimento não será enviada ao dispositivo.")
except IOError as e:
    print(f"ERRO: Não foi possível escrever o arquivo de saída - {e}")
    exit(1)