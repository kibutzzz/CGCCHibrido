# Grau B

Cena 3D interativa com iluminação Phong, câmera em primeira pessoa e animação por waypoints.

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

| Tecla | Ação |
|---|---|
| `Esc` | Modo navegar (câmera livre) |
| `1` | Modo transformar objeto selecionado |
| `2` | Modo animação |
| `3` | Modo luz |
| `Tab` | Selecionar próximo objeto / luz |
| `WASD` | Mover câmera (navegar) ou objeto/luz (outros modos) |
| `I / J` | Mover objeto/luz no eixo Y |
| `U / O` | Rotacionar yaw |
| `K / ;` | Rotacionar pitch |
| `N / M` | Rotacionar roll |
| `+ / -` | Escala (transformar) ou intensidade (luz) |
| `Space` | Adicionar waypoint à frente (animação) |

## Cena

A cena é configurada via `grau-b/scene.json`. É possível editar posições, rotações, escala e luzes sem recompilar.
