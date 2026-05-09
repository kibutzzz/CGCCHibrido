# Vivencial M2 — Seleção e Transformações em Objetos 3D

**Disciplina:** Computação Gráfica — Unisinos (Modo Híbrido)

## Autores

- Leonardo Ramos

## Descrição

Visualizador 3D com múltiplos modelos OBJ, seleção por teclado e aplicação de transformações (rotação, translação e escala por eixo) no objeto selecionado.

## Como compilar

```bash
cd CGCCHibrido
cmake -B build
cmake --build build --target VivencialM2
```

## Como executar

```bash
./build/VivencialM2
```

## Controles

| Tecla | Ação |
|---|---|
| **Tab** | Alterna o objeto selecionado |
| **R** | Entra no modo **Rotação** |
| **T** | Entra no modo **Translação** |
| **S** | Entra no modo **Escala** (fora do modo translação) |
| **Esc** | Fecha a aplicação |

### Modo Rotação (R)

| Tecla | Ação |
|---|---|
| X | Ativa/desativa rotação no eixo X |
| Y | Ativa/desativa rotação no eixo Y |
| Z | Ativa/desativa rotação no eixo Z |

### Modo Translação (T)

| Tecla | Ação |
|---|---|
| A / D | Move no eixo X |
| W / S | Move no eixo Z |
| I / J | Move no eixo Y |

### Modo Escala (S)

| Tecla | Ação |
|---|---|
| X / Shift+X | Aumenta / diminui escala no eixo X |
| Y / Shift+Y | Aumenta / diminui escala no eixo Y |
| Z / Shift+Z | Aumenta / diminui escala no eixo Z |
| U / Shift+U | Aumenta / diminui escala uniforme (todos os eixos) |

## Modelos utilizados

- **Plane** — `1405 Plane.obj`
- **House** — `PUSHILIN_house.obj`
- **Lamborghini** — `Lamborghini_Aventador.obj`

Os modelos estão em `assets/Modelos3D/`. O loader suporta polígonos com qualquer número de vértices por face (triangulação em leque) e normaliza automaticamente a escala de cada modelo para caber em um cubo unitário.
