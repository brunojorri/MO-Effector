# Hierarquia de interface — v1.0

## Avaliação

A hierarquia conceitual está boa para o usuário: primeiro se define **o que será desenhado**, depois **como será distribuído**, em seguida **como parecerá** e, por fim, **como será animado ou afetado**. A divisão recomendada é:

1. **Source** — primitiva interna ou layers de origem;
2. **Formation** — distribuição, centro e espaçamento, com `Advanced` para rotação do módulo, anéis concêntricos, Tilt, rotação/velocidade orbital do Z Circle e profundidade;
3. **Appearance** — tamanho, rotação, cor e opacidade, com `Random` no mesmo grupo;
4. **Motion** — Variation, Variation Speed, Individual Wiggle e Step / Stagger;
5. **Effectors** — contêiner único com Effector 1, cor e Effector 2;
6. **Connections** — modo de conexão e aparência, com `Line Effector` interno;
7. **Mode Specific** — Globe, Bezier Path e Z Circle.

## Decisão de compatibilidade

Os novos parâmetros foram anexados ao final da sequência interna. Isso preserva os índices gravados nos projetos e presets das versões anteriores. Reordená-los fisicamente agora poderia fazer um projeto antigo atribuir um valor ao controle errado.

Por isso, na v1.0:

- os grupos opcionais começam recolhidos e os subtópicos relacionados ficam aninhados;
- os nomes são curtos e orientados à tarefa;
- os Effectors e o Bezier Path possuem overlays visuais no Composition Viewer;
- os controles continuam animáveis, salváveis em presets e integrados ao After Effects.

## Iconografia

O painel padrão **Effect Controls** é construído pelo próprio After Effects. O SDK permite parâmetros nativos e desenho de overlays no viewer, mas não oferece uma substituição simples e segura de sliders, popups e tópicos por uma grade livre de botões iconográficos.

Uma interface predominantemente iconográfica exigiria um painel customizado separado. Esse painel poderá ser uma evolução futura e atuar apenas como uma camada de apresentação, mantendo este mesmo efeito C++ como motor de renderização.

## Próxima evolução de UI recomendada

Criar um painel compacto opcional com abas ou ícones para **Source**, **Formation**, **Motion** e **Effectors**. O painel deve escrever nos mesmos parâmetros do efeito nativo; assim, o projeto continua funcionando mesmo quando o painel não estiver aberto ou instalado.
