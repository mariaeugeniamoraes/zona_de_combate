#ifndef CENARIOS_H
#define CENARIOS_H

#include "raylib.h"

// DIMENSOES DA TELA
#define LARGURA_TELA 960
#define ALTURA_TELA 540

// CONFIGURACAO DO MAPA
#define TAMANHO_TILE 30
#define COLUNAS_MAPA (LARGURA_TELA / TAMANHO_TILE)
#define LINHAS_MAPA (ALTURA_TELA / TAMANHO_TILE)

// POSICAO DO CHAO
#define ALTURA_CHAO 450
#define LINHA_CHAO (ALTURA_CHAO / TAMANHO_TILE)

// TIPOS DE BLOCOS DO MAPA
typedef enum {
    TILE_VAZIO,
    TILE_SUPERFICIE,
    TILE_SOLO,
    TILE_LIMITE
} TipoTile;

// ARENAS DISPONIVEIS
typedef enum {
    ARENA_PARQUE,
    ARENA_COLISEU,
    TOTAL_ARENAS
} ArenaId;

// FUNCOES DOS CENARIOS
void carregarArena(ArenaId arena);
ArenaId obterArenaAtual(void);
const char *nomeArena(ArenaId arena);

int tileNaPosicao(float x, float y);
int posicaoSolida(float x, float y);

// DESENHA A ARENA EM PIXEL ART (320x180 ampliado 3x)
void desenharArena(void);

// LIBERA A TEXTURA USADA PELO DESENHO DAS ARENAS (chamar antes do CloseWindow)
void descarregarCenarios(void);

#endif