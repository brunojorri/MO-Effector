# MO Effector Native

Reimplementação C++ do MO Effector para Adobe After Effects.

## Objetivo

A antiga versão CEP criava uma layer para cada clone. A versão nativa é aplicada a um único Solid e renderiza todas as instâncias diretamente no mesmo buffer de imagem. Isso reduz drasticamente o número de layers, expressões e avaliações por frame.

## Estado atual

- núcleo C++20 independente do SDK;
- primitivas Circle, Square e Polygon;
- formações Grid, Linear, Circle concêntrico, Scatter, Globe 2.5D, Bezier Path e Z Circle;
- rotação global do módulo de formação, sem alterar o centro escolhido;
- composição direta no buffer nativo ARGB do After Effects em 8, 16 e 32 bpc;
- antialiasing analítico nas bordas de Circle, Square e Polygon;
- dois Effectors nativos independentes, com quatro shapes, seis curvas de falloff e mistura de cor;
- Smart Pre-Render/Smart Render compatível com Multi-Frame Rendering;
- guia visual não destrutivo para os raios interno e externo do Effector;
- Source Layer e Multi-Source com até quatro fontes, ciclo ou distribuição aleatória;
- Bezier Path com espaçamento uniforme, alinhamento pela tangente e guia visual;
- Variation determinística por clone em posição, escala, rotação e opacidade, com animação contínua em loop;
- Individual Wiggle por clone com ruído suave, movimento contínuo opcional, velocidade e amplitudes separadas;
- Random Size determinístico para variar a escala individual dos clones;
- Connections renderizadas atrás dos clones nos modos Sequence, Nearest e Distance;
- Line Effector independente para animar espacialmente opacidade e espessura das conexões;
- Color Palette de até quatro cores com ciclo, random e tint opcional das fontes;
- Effector Color com cor-alvo e intensidade controladas pelo falloff espacial;
- Step / Stagger animável com progresso, falloff, reverse e transformações sequenciais;
- testes automatizados de distribuição, determinismo e render em 8/16/32 bpc;
- adaptador completo para o SDK 25.6.61 do After Effects;
- projeto Visual Studio que gera um `.aex` nativo para Windows x64.

## Testar o núcleo

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-core.ps1
```

O script encontra automaticamente o Visual Studio com suporte a C++, compila o núcleo e executa os testes.

## Compilar o plugin

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-plugin.ps1
```

Defina `AE_SDK_ROOT` apontando para a raiz do SDK. Opcionalmente, coloque-o em `sdk/ae25.6_61.64bit.AfterEffectsSDK` na raiz do checkout.

## Estrutura

- `include/mo/Core.hpp`: API portátil do motor procedural.
- `src/Core.cpp`: distribuição, rasterização e alpha blending.
- `tests/CoreTests.cpp`: testes executáveis sem o After Effects.
- `plugin/`: camada de integração SmartFX com o SDK da Adobe.
- `docs/`: arquitetura e mapa de migração.

## Organização dos controles

Os parâmetros continuam no painel nativo **Effect Controls**, preservando a compatibilidade dos projetos criados nas versões anteriores. Os grupos avançados e opcionais começam recolhidos:

- **Formation**: construção principal, com `Advanced` interno para rotação, `Circle Rings` e Z Circle;
- **Appearance**: aparência principal, com `Random` interno;
- **Variation**, sua seção interna `Animation`, **Individual Wiggle** e **Step / Stagger**: movimento e variação;
- **Effectors**: contêiner único com **Effector 1**, sua cor, e **Effector 2**;
- **Connections**: topologia, aparência das linhas e `Line Effector` interno;
- **Layer Source**, **Multi-Source** e **Color Palette**: conteúdo dos clones;
- **Globe**, **Bezier Path** e **Formation Advanced**: controles específicos de cada modalidade.

O painel padrão do After Effects não permite substituir livremente cada controle nativo por botões de ícone sem desenvolver uma interface customizada completa. Nesta versão, a melhoria visual usa grupos compactos/recolhidos e overlays gráficos para Effectors e Path, mantendo os controles animáveis e compatíveis com presets.

Veja a análise completa em [`docs/UI_HIERARCHY.md`](docs/UI_HIERARCHY.md).

## Estado do adaptador — v1.0

O adaptador escreve diretamente no `PF_EffectWorld`, sem criar um bitmap temporário por frame. A versão atual suporta 8/16/32 bpc, antialiasing analítico, SmartFX, Multi-Frame Rendering, dois Effectors de clones, Line Effector, conexões procedurais, overlays visuais, Circle com anéis concêntricos, Globe 2.5D, Z Circle com órbita animável e clonagem de até quatro Source Layers.

© 2026 Bruno Jorri. Todos os direitos reservados.
