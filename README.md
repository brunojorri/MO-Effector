# MO Effector Native

Plugin procedural de cloners e effectors para Adobe After Effects, criado por **Bruno Jorri**.

A versão **1.0** é um efeito nativo C++: todos os clones são renderizados diretamente em uma única layer, sem criar dezenas ou centenas de Shape Layers e expressões. A antiga implementação CEP foi aposentada e removida deste repositório.

## Instalação em uma linha

Feche o After Effects, abra o **PowerShell** e execute:

```powershell
irm https://raw.githubusercontent.com/brunojorri/mo-effector/main/install.ps1 | iex
```

Confirme a solicitação de administrador. O instalador baixa o `.aex` oficial, valida o SHA-256 e instala em:

`Adobe After Effects 2026\Support Files\Plug-ins\MO Tools`

Depois, abra o After Effects e procure por **Effect > MO Tools > MO Effector Native**.

## Principais recursos

- Renderização nativa em uma única layer, compatível com 8/16/32 bpc e Multi-Frame Rendering.
- Primitivas Circle, Square e Polygon com antialiasing analítico.
- Grid, Linear, Circle concêntrico, Scatter, Globe 2.5D, Bezier Path e Z Circle.
- Source Layer e Multi-Source com até quatro fontes.
- Dois Effectors independentes com falloff, transforms e cor.
- Variation animada, Individual Wiggle, Step / Stagger e Random Size.
- Color Palette com distribuição cíclica ou aleatória.
- Connections nos modos Sequence, Nearest e Distance.
- Line Effector dedicado para animar opacidade e espessura das conexões.
- Overlays visuais no Composition Viewer.

## Uso rápido

1. Crie um Solid do tamanho da composição.
2. Aplique **MO Effector Native**.
3. Escolha a primitiva e a formação no painel Effect Controls.
4. Ajuste Clone Count, Appearance, Variation e Effectors.
5. Para clonar uma layer, abra `Layer Source`, escolha `Source Layer` e selecione a fonte.

Consulte o [manual](docs/MANUAL.md), a [arquitetura](docs/ARCHITECTURE.md) e o [código-fonte nativo](src/MO_Effector_Native/README.md).

## Compilar do código-fonte

Requisitos:

- Adobe After Effects SDK 25.6.61 ou compatível;
- Visual Studio com C++ Desktop Build Tools;
- variável `AE_SDK_ROOT` apontando para a raiz do SDK.

```powershell
powershell -ExecutionPolicy Bypass -File .\src\MO_Effector_Native\scripts\build-core.ps1
powershell -ExecutionPolicy Bypass -File .\src\MO_Effector_Native\scripts\build-plugin.ps1
```

O SDK da Adobe não é redistribuído neste repositório.

## Atualização

Execute novamente a linha de instalação. O instalador substitui somente o plugin em `MO Tools` e também remove, quando encontrada, a antiga extensão CEP do MO Effector.

## Desinstalação

```powershell
irm https://raw.githubusercontent.com/brunojorri/mo-effector/main/scripts/uninstall.ps1 | iex
```

## Compatibilidade

- Windows 10/11 x64.
- Adobe After Effects 2026.

## Autoria

[By Bruno Jorri](https://www.instagram.com/brunojorri_work/)

Código publicado para preservação, instalação e continuidade do projeto. © 2026 Bruno Jorri. Todos os direitos reservados.
