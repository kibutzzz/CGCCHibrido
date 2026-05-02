# Tarefa M3 — Adicionando Texturas

**Vencimento:** terça-feira, 12 mai. 2026, 23:59

## Descrição

Dando continuidade ao nosso visualizador, o objetivo desta tarefa é implementar a leitura das coordenadas de texturas presentes no arquivo `.OBJ`, armazenando-as como atributo dos vértices e as informações sobre a textura contidas no arquivo `.MTL` (no momento, apenas o nome da textura a ser carregada).

Depois disso, seguir o material de apoio para permitir o desenho dos objetos texturizados.

## Código base

O professor disponibilizou um código base para leitura do arquivo OBJ (com explicações no `.md`):
- https://github.com/guilhermechagaskurtz/CGCCHibrido/tree/main/Code%20snippets

## Material de apoio

- `M3-TexturasOpenGL.pdf` — Implementando o uso de texturas com OpenGL (passo a passo)
- Link do projeto auxiliar: HelloTriangle - Textures (disponível no Moodle, seção 16)

## O que implementar

1. Ler coordenadas de textura (`vt`) do arquivo `.OBJ` e armazená-las como atributo dos vértices
2. Ler o arquivo `.MTL` e extrair o nome da textura a ser carregada
3. Carregar a textura usando `stb_image` e enviá-la para a GPU via `glTexImage2D`
4. Configurar o VAO/VBO para incluir as coordenadas de textura (novo atributo `location = 2`)
5. Atualizar os shaders para passar e usar as coordenadas de textura
6. Renderizar os objetos texturizados
