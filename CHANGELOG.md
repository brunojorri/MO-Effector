# Changelog

## 1.0.1 — Time-based animation fix

- Declara corretamente ao After Effects que a saída varia com o tempo.
- `Animation Speed`, `Individual Wiggle`, Globe Speed e Z Circle Speed agora atualizam continuamente durante o playback, mesmo sem keyframes.

## 1.0.0 — Native C++

- Substituição completa da implementação CEP pelo plugin nativo C++.
- Renderização de todos os clones em uma única layer.
- Circle, Square e Polygon com antialiasing analítico.
- Grid, Linear, Circle concêntrico, Scatter, Globe 2.5D, Bezier Path e Z Circle.
- Source Layer, Multi-Source, Palette, Variation, Wiggle e Step / Stagger.
- Dois Effectors de clones e Line Effector dedicado.
- Connections nos modos Sequence, Nearest e Distance.
- SmartFX, Multi-Frame Rendering e buffers 8/16/32 bpc.
- Instalador remoto com validação SHA-256.

## 0.22.1 — CEP, aposentado

- Última versão da antiga extensão CEP baseada em Shape Layers e expressões.
- O histórico permanece no Git, mas os arquivos CEP não fazem mais parte da branch principal.
