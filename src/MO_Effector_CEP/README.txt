MO Effector 0.22.1 — extensão CEP para After Effects

Novidades da versão 0.12:
- MO Effector agora é uma Shape Layer circular visível e não renderizável (Guide Layer).
- O círculo azul representa Radius; o verde representa Inner Radius.
- Scale X/Y e Rotation da guia alteram visualmente e matematicamente a zona de influência.
- UPGRADE EFFECTOR GUIDE converte sistemas antigos baseados em Null.

Correção 0.12.1:
- A influência agora é medida explicitamente a partir do anchor point central do Effector.
- Position, Scale X/Y, Rotation e parenting preservam a correspondência com a guia visual.

Correção 0.12.2:
- Removida a chamada incompatível toCompVec que fazia os clones se sobreporem.
- A transformação da zona de influência agora usa pontos toComp compatíveis com o After Effects.

Novidades da versão 0.13:
- Quick Source cria Circle, Square ou Polygon no centro da composição.
- Tamanho do source e quantidade de lados do polígono configuráveis no painel.
- O novo source já fica selecionado e pronto para criar o cloner.
- Interface reorganizada com iconografia e ações compactas.

Novidades e correções da versão 0.14:
- Cálculo de influência simplificado e estável, centralizado no anchor point do Effector.
- Clean remove clones, Effector e controles, preservando e reativando o source.
- A layer de controle agora se chama MO Cloner Controls na timeline.

Correção 0.14.1:
- Position calcula o falloff a partir da posição-base individual de cada clone.
- Eliminada a referência circular entre o falloff e a própria expressão de Position.
- O tamanho da influência acompanha explicitamente o Transform Scale do Effector circular.

Novidades da versão 0.15:
- Globe 2.5D com distribuição Fibonacci uniforme sobre uma esfera simulada.
- Rotation e Spin independentes nos eixos X, Y e Z.
- Escala de frente/fundo e opacidade traseira criam profundidade sem layers 3D.
- Interface Globe usa Clone Count e Globe Radius com defaults próprios.

Novidades da versão 0.16:
- Effector Shape: Circle, Box, Linear X e Linear Y.
- Falloff: Linear, Smooth, Ease In, Ease Out, Gaussian e Constant.
- Invert e Power para refinamento da curva de influência.
- Guias visuais externas e internas adaptam-se automaticamente ao formato escolhido.

Novidades da versão 0.17:
- Até quatro Effectors no mesmo cloner.
- Add Effector e Remove Last diretamente no painel.
- Combine Modes: Maximum, Add, Multiply e Subtract.
- Opacity individual de cada Shape Layer funciona como Weight.

Correção 0.17.1:
- Slots vazios não são mais avaliados pelas expressões.
- MO Effector Count controla de forma segura quantas zonas estão ativas.
- Removida a dependência do efeito MO Weight que causava erros em sistemas existentes.

Novidades da versão 0.18:
- Color Effector universal baseado no efeito Fill de cada clone.
- Base Color, Effector Color e Amount seguem as mesmas zonas de influência visuais.
- Random Color e Seed criam variação determinística por clone.
- Globe Depth Color interpola cores independentes entre a parte traseira e frontal.
- Desativar Enable Color preserva a aparência original da fonte.

Novidades da versão 0.19:
- Interface compacta com módulos recolhíveis e ações centradas em iconografia.
- Path Cloner cria uma curva Bezier editável como Guide Layer.
- Clones são redistribuídos ao vivo quando os vértices e tangentes do caminho mudam.
- Path Start, End, Offset, Reverse e Loop refinam a distribuição.
- Orient to Path e Rotation Offset alinham cada clone à direção da curva.
- O botão Path em Tools seleciona rapidamente a guia para edição.

Novidades da versão 0.20:
- Enable Color começa ativo em novos sistemas e no painel.
- Multi-Source aceita de duas a oito camadas-fonte no mesmo cloner.
- Distribuição das fontes em Sequence ou Random com Seed reproduzível.
- Source Offset permite deslocar a sequência diretamente no Effect Controls.
- Sync reconstrói automaticamente os clones quando a fonte atribuída muda.
- Clean restaura e seleciona todas as fontes originais do sistema.

Novidades da versão 0.21:
- Globe Pro com distribuições Fibonacci, Latitude Rings e Random Sphere.
- Globe Width e Height transformam a esfera em elipsoides controláveis.
- Perspective reforça a sensação de profundidade na posição e na escala.
- Globe Twist torce a distribuição ao redor da superfície.
- Surface Orient alinha a rotação 2D dos clones à curvatura projetada.
- Rings, Seed e Orient Offset completam o refinamento do globo.

Novidades da versão 0.22:
- Biblioteca de presets integrada ao painel compacto.
- Seis presets internos: Organic Grid, Wave Cascade, Soft Effector Pulse, Globe Orbit, Neon Depth e Stagger Fan.
- Presets personalizados são salvos localmente com nome definido pelo usuário.
- Save captura parâmetros, Combine Mode e layout relativo de até quatro Effectors.
- Apply preserva Sources e geometria do Path, reconstrói Effectors e sincroniza clones.
- Delete remove apenas presets personalizados; os presets internos permanecem protegidos.

Correção 0.22.1:
- Crédito discreto "By Bruno Jorri" adicionado ao rodapé do painel.
- O crédito abre o perfil oficial no Instagram no navegador padrão.

Instalação pública (Windows PowerShell):
irm https://raw.githubusercontent.com/brunojorri/mo-effector/main/install.ps1 | iex

Depois de instalar, reinicie o After Effects e abra Window > Extensions > MO Effector.
