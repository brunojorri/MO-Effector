# Arquitetura nativa

## Fluxo de render

1. O usuário aplica **MO Effector Native** a um Solid.
2. O After Effects entrega um único `PF_EffectWorld` ao plugin.
3. O adaptador converte os parâmetros nativos em `mo::RenderParams`.
4. `makeInstances()` calcula as transformações de todos os clones.
5. `render()` rasteriza as primitivas no buffer de saída.
6. O frame volta ao After Effects sem criar layers auxiliares.

## Separação de responsabilidades

- **Core portátil:** matemática, formações, effectors e rasterização. Não inclui headers da Adobe.
- **Adapter AE:** `EffectMain`, parâmetros, pixel formats, SmartFX e checkout de layers-fonte.
- **UI:** começa com parâmetros nativos no Effect Controls. Uma UI customizada pode ser adicionada depois sem alterar o renderizador.

## Evolução planejada

1. Grid, Linear, Circle e Scatter com primitivas internas.
2. Effectors com falloff e transformações por instância.
3. ~~Globe 2.5D e ordenação por profundidade.~~ Concluído na v0.4.
4. ~~Source Layer e Multi-Source para clonar pixels de outras layers.~~ Concluído nas versões 0.5 e 0.6.
5. ~~SmartFX 8/16/32 bpc e render multiframe seguro.~~ Concluído na v0.3.
6. GPU opcional, depois que a implementação CPU estiver visualmente validada.

## Compatibilidade com a versão CEP

Os nomes e intervalos de parâmetros serão mantidos sempre que fizer sentido. Presets CEP não serão binariamente compatíveis, mas um importador poderá converter os valores para o efeito nativo.
