# Manual do MO Effector Native 1.0

## Começando

1. Crie um Solid do tamanho da composição.
2. Aplique **Effect > MO Tools > MO Effector Native**.
3. Escolha `Primitive`, `Formation` e `Clone Count`.
4. Ajuste o conjunto em `Appearance`, `Variation`, `Individual Wiggle`, `Effectors` e `Connections`.

Todas as mudanças são atualizadas imediatamente; não existe botão Sync ou criação de layers por clone.

## Source

- **Circle:** círculo com antialiasing.
- **Square:** quadrado, inclusive com rotação suavizada.
- **Polygon:** polígono de 3 a 32 lados.

Em `Layer Source`, altere `Source Mode` para usar uma layer da composição. `Multi-Source` permite até quatro fontes com distribuição Cycle ou Random.

## Formation

- **Grid:** linhas, colunas e espaçamentos independentes.
- **Linear:** clones centralizados em linha.
- **Circle:** um ou mais anéis concêntricos; `Circle Rings` fica em `Advanced`.
- **Scatter:** distribuição pseudoaleatória reproduzível.
- **Globe 2.5D:** esfera projetada com rotação, velocidade e escala de profundidade.
- **Bezier Path:** curva visual com alinhamento pela tangente.
- **Z Circle:** círculo inclinado com profundidade, órbita e velocidade automáticas.

`Module Rotation` gira a formação completa ao redor do Center.

## Appearance e Color Palette

Appearance controla Clone Size, Rotation, Color e Opacity. `Random Size` varia a escala individual de forma determinística. Color Palette oferece até quatro cores com distribuição Cycle ou Random.

## Motion

- **Variation:** jitter determinístico de Position, Scale, Rotation e Opacity; `Animation > Speed` cria movimento contínuo.
- **Individual Wiggle:** ruído suave por clone, com velocidade e amplitudes separadas.
- **Step / Stagger:** transformação sequencial com Progress, Falloff e Reverse.

## Effectors

`Effectors` contém Effector 1 e Effector 2. Cada um possui:

- Center e Shape;
- raios interno e externo;
- seis curvas de Falloff;
- Strength e Invert;
- Position, Scale, Rotation e Target Opacity;
- mistura opcional para Target Color.

Os overlays no viewer mostram a região espacial sem entrar no render.

## Connections e Line Effector

- **Sequence:** conecta os clones pela ordem e pode fechar o loop.
- **Nearest:** conecta a quantidade escolhida de vizinhos próximos.
- **Distance:** conecta todos os pares dentro de Max Distance.

As linhas são desenhadas atrás dos clones. O `Line Effector` controla espacialmente Target Opacity e Width Change.

Para revelar linhas, use Connections Opacity em 0% e Line Effector Target Opacity em 100%. Para apagá-las, faça o inverso e anime o Center ou Radius do Line Effector.

## Atualização e remoção

Execute novamente a instalação de uma linha para atualizar. Feche o After Effects antes de instalar ou desinstalar.
