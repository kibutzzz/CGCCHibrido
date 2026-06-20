# Grau B

Cena 3D interativa com iluminação Phong, câmera em primeira pessoa e animação por waypoints.

## Demo

https://github.com/kibutzzz/CGCCHibrido/blob/main/grau-b/grau-b.mp4

## Compilar

Na raiz do repositório `CGCCHibrido`:

```bash
cmake -B build
cmake --build build --target GrauB
```

## Executar

```bash
./build/GrauB
```

## Controles

### Troca de modo

| Tecla | Modo |
|---|---|
| `Esc` | Navegar |
| `1` | Transformar |
| `2` | Animação |
| `3` | Luz |
| `Q` | Sair |

### Modo navegar (`Esc`)

| Tecla | Ação |
|---|---|
| `WASD` | Mover câmera |
| Mouse | Olhar |
| Scroll | Zoom (FOV) |

### Modo transformar (`1`)

| Tecla | Ação |
|---|---|
| `Tab` | Selecionar próximo objeto |
| `WASD` | Mover no plano XZ |
| `I / J` | Mover no eixo Y |
| `U / O` | Rotacionar yaw |
| `K / ;` | Rotacionar pitch |
| `N / M` | Rotacionar roll |
| `+ / -` | Escala uniforme |

### Modo animação (`2`)

| Tecla | Ação |
|---|---|
| `Tab` | Selecionar próximo objeto |
| `Space` | Adicionar waypoint 10u à frente da câmera |

### Modo luz (`3`)

| Tecla | Ação |
|---|---|
| `Tab` | Selecionar próxima luz |
| `WASD` | Mover no plano XZ |
| `I / J` | Mover no eixo Y |
| `+ / -` | Ajustar intensidade |

## Cena

A cena é configurada via `grau-b/scene.json`. É possível editar posições, rotações, escala e luzes sem recompilar.
