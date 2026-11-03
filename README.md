# ZONA DE COMBATE

### Um jogo de luta 2D desenvolvido em C com raylib

**Zona de Combate** é um jogo de luta em pixel art, desenvolvido como projeto acadêmico, que coloca dois jogadores frente a frente em batalhas com golpes, habilidades especiais e diferentes personagens.

Cada lutador possui um elemento próprio, e cada partida exige estratégia, reflexos e domínio dos controles. O objetivo é simples: **vencer dois rounds antes do adversário e conquistar seu lugar no ranking!**

---

##  Sobre o jogo

Inspirado nos jogos clássicos de luta em 2D, Zona de Combate combina combate local para dois jogadores, personagens animados, arenas temáticas e efeitos visuais.

O projeto foi desenvolvido utilizando a linguagem C e a biblioteca gráfica raylib, com aplicação prática de conceitos de programação e estruturas de dados.

###  Funcionalidades

-  Modo versus local para dois jogadores
-  Sistema de combate com socos, chutes e defesa
-  Movimentação, pulos e agachamento
-  Poderes especiais exclusivos para cada personagem
-  Barras de vida e energia
-  Cronômetro e sistema de rounds
-  Tela de vitória com animações e efeitos visuais
-  Ranking de personagens com vitórias salvas em arquivo
-  Sistema de pausa durante a partida
-  Cenários e personagens em pixel art

---

##  Conheça os lutadores

| Personagem | Elemento | Habilidade especial |
|---|---|---|
| ⚡ **Kai** | Raio | Ataque elétrico |
| 🔥 **Luna** | Fogo | Projétil de fogo |
| 💧 **Nero** | Água | Ataque aquático |
| 🌪️ **Ayra** | Vento | Rajada de vento |

Cada personagem possui animações próprias para movimentação, combate, vitória e derrota.

---

##  Arenas

O jogo possui dois cenários com identidades visuais diferentes.

###  Parque Abandonado

Um antigo parque de diversões, com roda-gigante, montanha-russa, carrossel e uma atmosfera misteriosa.

###  Coliseu

Uma arena inspirada nos antigos coliseus, com arquibancadas, bandeiras, iluminação e detalhes arquitetônicos.

Os cenários são construídos em pixel art e utilizam matrizes para representar elementos do mapa.

---

##  Controles

###  Jogador 1

| Tecla | Ação |
|---|---|
| `A` / `D` | Movimentar |
| `W` | Pular |
| `S` | Defender |
| `C` | Agachar |
| `F` | Socar |
| `G` | Chutar |
| `E` | Poder especial |

###  Jogador 2

| Tecla | Ação |
|---|---|
| `←` / `→` | Movimentar |
| `↑` | Pular |
| `↓` | Defender |
| `Shift direito` | Agachar |
| `K` | Socar |
| `L` | Chutar |
| `O` | Poder especial |

### Outros comandos

| Tecla | Ação |
|---|---|
| `P` | Pausar ou continuar |
| `ESC` | Voltar ao menu |
| `ENTER` | Confirmar e iniciar a luta |

**Atenção:** os poderes especiais consomem 50 pontos de energia.

---

##  Sistema de rounds e ranking

As partidas são disputadas em rounds. O primeiro jogador a conquistar duas vitórias vence o confronto.

Ao final da partida, o jogo apresenta:

- O personagem vencedor comemorando
- O personagem derrotado
- O placar final
- Animações e efeitos de celebração

As vitórias são registradas no arquivo `dados/ranking.txt`, permitindo manter a classificação dos personagens entre diferentes execuções.

O ranking apresenta os quatro lutadores organizados por quantidade de vitórias, em um pódio animado.

---

##  Tecnologias utilizadas

| Tecnologia | Utilização |
|---|---|
| **C** | Lógica principal e funcionamento do jogo |
| **raylib** | Renderização gráfica, animações e entrada pelo teclado |
| **Git** | Controle de versões |
| **GitHub** | Hospedagem e colaboração no código |

---

##  Conceitos de programação aplicados

O projeto utiliza conceitos fundamentais de programação em C, aplicados diretamente às mecânicas do jogo.

### Estruturas (`struct`)

Utilizadas para organizar dados de personagens, projéteis, partidas e efeitos visuais.

### Ponteiros

Utilizados para acessar estruturas, modificar dados e gerenciar elementos dinâmicos.

### Listas encadeadas

O sistema de efeitos visuais utiliza uma lista simplesmente encadeada, permitindo adicionar, percorrer e remover efeitos durante a partida.

### Alocação dinâmica de memória

Os efeitos visuais são criados dinamicamente utilizando `malloc()` e têm sua memória liberada por `free()` quando deixam de ser necessários.

### Matrizes

Os cenários utilizam uma matriz bidimensional para representar os tiles do mapa, incluindo superfícies, solo e limites.

### Manipulação de arquivos

O sistema de ranking utiliza operações de leitura e escrita em arquivos para manter o histórico de vitórias.

Esses conceitos foram utilizados de forma funcional, contribuindo para a organização, eficiência e funcionamento do jogo.

---

##  Como executar

### Pré-requisitos

- Sistema Linux com ambiente gráfico
- Compilador GCC
- Biblioteca raylib instalada
- `pkg-config` disponível no terminal

### 1. Clonar o repositório

```bash
git clone URL_DO_REPOSITORIO
cd zona_de_combate
```

*Substitua `URL_DO_REPOSITORIO` pelo endereço real do projeto e ajuste o nome da pasta, caso necessário.*

### 2. Compilar

Na pasta principal do projeto, execute:

```bash
gcc src/*.c -o jogo $(pkg-config --cflags --libs raylib) -lX11 -lm
```

### 3. Executar

```bash
./jogo
```

**Importante:** execute o jogo a partir da pasta principal, para que os arquivos de sprites e os dados do ranking sejam encontrados corretamente.

---

## Organização do projeto

```text
zona_de_combate/
│
├── src/
│   ├── main.c
│   ├── jogo.c
│   ├── jogo.h
│   ├── personagens.c
│   ├── personagens.h
│   ├── cenarios.c
│   ├── cenarios.h
│   ├── efeito.c
│   ├── efeito.h
│   ├── interface.c
│   ├── interface.h
│   ├── menu.c
│   ├── menu.h
│   ├── telas.c
│   └── telas.h
│
├── assets/
│   └── personagens/
│
├── dados/
│   └── ranking.txt
│
└── README.md
```

*Estrutura baseada nos módulos utilizados no desenvolvimento. Alguns arquivos ou diretórios podem variar conforme a versão final do repositório.*

---

##  Demonstração

O funcionamento do jogo pode ser acompanhado no vídeo de demonstração:

**[Assistir à gameplay](INSERIR_LINK_DO_VIDEO)**

O vídeo apresenta as principais funcionalidades, incluindo seleção de personagens, combate, habilidades especiais, sistema de rounds, tela de vitória e ranking.

---

##  Equipe de desenvolvimento

Projeto desenvolvido em grupo para a disciplina de programação em C.

**Integrantes:**

- Integrante 1
- Integrante 2
- Integrante 3

*Substituir pelos nomes completos dos integrantes da equipe.*

---

##  Objetivo acadêmico

O principal objetivo do projeto é aplicar conceitos estudados durante a disciplina por meio da construção de um jogo interativo e funcional.

Além do desenvolvimento da jogabilidade, o projeto busca demonstrar a utilização de estruturas de dados, ponteiros, alocação dinâmica, matrizes e persistência em arquivos, promovendo a integração entre teoria e prática.

---

###  ESCOLHA SEU LUTADOR. DOMINE SEU ELEMENTO. CONQUISTE A VITÓRIA.

**Zona de Combate — Onde cada round conta!**
