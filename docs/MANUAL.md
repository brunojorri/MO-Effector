# Manual do MO Effector

## Conceito

O sistema mantém uma ou mais layers-fonte desativadas, cria clones vinculados ao `MO Cloner Controls` e usa expressões para distribuir e animar cada clone. O `MO Effector` é uma Guide Layer: aparece no canvas, mas não no render.

## Formações

- **Grid:** linhas, colunas e espaçamento independentes.
- **Linear:** clones centralizados em uma linha.
- **Circle:** distribuição radial completa.
- **Scatter:** distribuição pseudoaleatória reproduzível.
- **Globe:** esfera 2.5D com profundidade simulada.
- **Path:** distribuição sobre uma curva Bézier editável.

## Effectors

O raio externo azul delimita a influência. O guia interno verde representa o Inner Radius. Position, Scale, Rotation e Opacity da Guide Layer controlam posição, forma e peso da zona.

Até quatro Effectors podem ser combinados por Maximum, Add, Multiply ou Subtract.

## Multi-Source

Selecione de duas a oito layers 2D, abra Quick Source, marque Multi-Source e escolha Sequence ou Random. Para alterar Source Mode, Offset ou Seed posteriormente, ajuste o Controller e clique em Sync.

## Path Cloner

O modo Path cria `MO Cloner Path`. Selecione o Controller e use **Tools > Path** para selecionar a guia. Edite vértices e tangentes normalmente no canvas.

## Presets

- **Apply:** aplica o preset e sincroniza os clones.
- **Save:** captura todos os parâmetros e o layout relativo dos Effectors.
- **Delete:** remove apenas presets personalizados.

Sources e a geometria do Path não são substituídos por presets.

## Manutenção

- **Sync:** atualiza Clone Count, Multi-Source e expressões.
- **Center:** centraliza formações compatíveis.
- **Upgrade:** reconstrói o guia principal e atualiza expressões antigas.
- **Clean:** remove o sistema e restaura todas as fontes.
