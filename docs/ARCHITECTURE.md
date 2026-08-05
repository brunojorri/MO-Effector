# Arquitetura

## Componentes

- `client/`: interface CEP em HTML, CSS e JavaScript.
- `host/MOEngine.jsx`: automação do After Effects e geração das expressões.
- `host/MOClonerControls.xml`: definição do pseudo-effect registrado no `PresetEffects.xml`.
- `CSXS/manifest.xml`: manifesto da extensão CEP.

## Modelo de layers

- `MO Cloner Controls`: Null com pseudo-effect e referências estruturais.
- `MO Source N`: fontes originais desativadas durante o funcionamento.
- `MO Clone N`: duplicatas renderizáveis com expressões.
- `MO Effector N`: Shape Guide Layers não renderizáveis.
- `MO Cloner Path`: guia Bézier opcional e não renderizável.

## Compatibilidade de parâmetros

Novos grupos do pseudo-effect são acrescentados ao final da definição. Essa regra preserva os índices internos dos parâmetros já existentes e evita quebrar projetos antigos.

## Segurança do instalador

O instalador cria um backup individual de cada `PresetEffects.xml`, substitui somente o bloco identificado por `Pseudo/MO Cloner Controls` e instala o CEP em um diretório específico do usuário.
