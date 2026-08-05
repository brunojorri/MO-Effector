# MO Effector

Sistema procedural de cloners e effectors para Adobe After Effects, criado por **Bruno Jorri**.

O MO Effector transforma qualquer layer 2D em formações procedurais controladas por um painel CEP compacto. A versão atual é a **0.22.1**.

## Instalação em uma linha

Feche o After Effects, abra o **PowerShell** e execute:

```powershell
irm https://raw.githubusercontent.com/brunojorri/mo-effector/main/install.ps1 | iex
```

Confirme a solicitação de administrador. O instalador:

- baixa a versão mais recente;
- instala a extensão CEP no perfil do usuário;
- localiza as instalações compatíveis do After Effects;
- cria backup do `PresetEffects.xml` antes de alterá-lo;
- registra o pseudo-effect `MO Cloner Controls`;
- habilita o modo de desenvolvimento CEP.

Depois, reinicie o After Effects e abra **Window > Extensions > MO Effector**.

## Recursos

- Quick Source: Circle, Square e Polygon.
- Grid, Linear, Circle, Scatter, Globe e Path Cloner.
- Globe Pro com Fibonacci, Latitude Rings e Random Sphere.
- Até quatro Effectors visuais combináveis.
- Falloff Circle, Box, Linear X e Linear Y.
- Falloff Linear, Smooth, Ease, Gaussian e Constant.
- Position, Scale, Rotation, Opacity, Step Stagger e Noise.
- Color Effector, Random Color e Globe Depth Color.
- Multi-Source com Sequence, Random, Offset e Seed.
- Presets internos e biblioteca de presets personalizados.
- Sync, Center, Upgrade e Clean.

## Uso rápido

1. Crie ou selecione uma layer 2D.
2. Escolha a formação no painel.
3. Clique em **Create Cloner**.
4. Selecione `MO Cloner Controls` para editar os parâmetros.
5. Mova `MO Effector` no canvas para controlar a influência.

Consulte o [manual completo](docs/MANUAL.md) e a [arquitetura](docs/ARCHITECTURE.md).

## Atualização

Execute novamente a mesma linha de instalação. O instalador substitui a extensão e atualiza o registro preservando um novo backup.

## Desinstalação

```powershell
irm https://raw.githubusercontent.com/brunojorri/mo-effector/main/scripts/uninstall.ps1 | iex
```

## Compatibilidade

- Windows 10/11.
- Adobe After Effects com suporte a CEP e `PresetEffects.xml`.
- Testado durante o desenvolvimento no After Effects 2026.

## Autoria

[By Bruno Jorri](https://www.instagram.com/brunojorri_work/)

Código publicado para preservação, instalação e continuidade do projeto. © 2026 Bruno Jorri. Todos os direitos reservados.
