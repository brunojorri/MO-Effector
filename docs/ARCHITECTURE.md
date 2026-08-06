# Arquitetura nativa

## Visão geral

O MO Effector Native é um efeito C++ SmartFX aplicado a uma única layer. O motor calcula as instâncias procedurais e escreve diretamente no `PF_EffectWorld` entregue pelo After Effects.

```text
Effect Controls
      ↓
RenderParams
      ↓
makeInstances()
      ↓
Connections → clones internos ou Source Layers
      ↓
PF_EffectWorld (8/16/32 bpc)
```

## Componentes

- `include/mo/Core.hpp`: API portátil, parâmetros e estruturas das instâncias.
- `src/Core.cpp`: formações, effectors, conexões, rasterização e alpha blending.
- `plugin/MOEffector.cpp`: integração com o After Effects, parâmetros, SmartFX e overlays.
- `plugin/MOEffectorPiPL.r`: identidade, versão e flags do plugin.
- `tests/CoreTests.cpp`: validações independentes do host.
- `dist/`: binário Release e checksum publicados.

## Pipeline

1. Os parâmetros do Effect Controls são convertidos em `RenderParams`.
2. `makeInstances()` cria posição, escala, rotação, opacidade, profundidade e cor de cada clone.
3. Variation, Wiggle, Step e Effectors modificam as instâncias.
4. Connections são calculadas sobre as posições finais e desenhadas atrás dos clones.
5. Primitivas internas ou Source Layers são rasterizadas diretamente no buffer do host.

## Desempenho

- Nenhuma layer é criada por clone.
- Smart Pre-Render/Smart Render e Multi-Frame Rendering são suportados.
- UInt8, UInt16 e Float32 são processados diretamente.
- Nearest e Distance limitam a análise espacial a 2.000 clones e 20.000 conexões para proteger a interatividade.

## Compatibilidade

O Match Name permanece `com.brunojorri.MOEffectorNative`. Disk IDs existentes não são reorganizados quando novos parâmetros são acrescentados, preservando projetos e presets nativos.

A antiga versão CEP permanece no histórico Git, mas não é binariamente compatível com o efeito C++ porque utilizava Shape Layers, expressões e um pseudo-effect.
