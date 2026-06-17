# Tarefa M6 — Definindo Trajetórias para Objetos

## Demonstração

![demo](demo.mp4)

## Descrição

Implementação de trajetórias cíclicas para objetos de uma cena 3D com iluminação Phong e câmera em primeira pessoa.

Cada objeto pode ter uma lista de waypoints. A cada frame, o objeto avança linearmente entre o waypoint atual e o próximo, voltando ao início ao chegar no último (ciclo contínuo).

## Controles

| Tecla | Ação |
|-------|------|
| W / A / S / D | Mover câmera |
| Mouse | Olhar ao redor |
| Scroll | Zoom |
| Tab | Selecionar próximo objeto |
| P | Adicionar waypoint na posição atual da câmera |
| Esc | Sair |

## Como compilar e executar

```bash
cmake -B build
cmake --build build --target TarefaM6
./build/TarefaM6
```

## Cena

- **Suzanne** — órbita circular com 4 waypoints
- **Casa** — caminho quadrado com 4 waypoints
- **Plano** — estático (sem animação)

Novos waypoints podem ser adicionados em tempo de execução com a tecla `P`.
