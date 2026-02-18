# -*- coding: utf-8 -*-
import os

# --- Configuração ---
# Arquivos de origem
HTML_IN = "web/index.html"
CSS_IN = "web/style.css"
JS_IN = "web/script.js"

# Arquivo de saída unificado
HTML_OUT = "data/index_bundle.html"

# --- Leitura dos Conteúdos ---
print("Lendo arquivos de origem...")
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
    exit(1)

# --- Injeção do CSS ---
print("Incorporando CSS...")
css_link_tag = '<link rel="stylesheet" href="style.css">'
style_tag = f"<style>{css_content}</style>"
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
script_tag = f"<script>{js_content}</script>"
html_content = html_content.replace(js_script_tag, script_tag)
print(" -> JavaScript incorporado no final do <body>.")


# --- Escrita do Arquivo Final ---
print(f"Escrevendo arquivo unificado em {HTML_OUT}...")
try:
    with open(HTML_OUT, 'w', encoding='utf-8') as f:
        f.write(html_content)
    print("SUCESSO!")
    print(f"Arquivo '{HTML_OUT}' foi criado e está pronto para ser usado no ESP32.")
    print("Você pode fazer o upload da pasta 'data' completa, que agora contém o bundle.")
except IOError as e:
    print(f"ERRO: Não foi possível escrever o arquivo de saída - {e}")
    exit(1)
