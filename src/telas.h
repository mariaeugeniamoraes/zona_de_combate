#ifndef TELAS_H
#define TELAS_H

#include "raylib.h"

// =====================================
// TELAS EM PIXEL ART: SELECAO E RANKING
// Desenhadas numa tela pequena de 320x180 e ampliadas
// para a janela (3x em 960x540), no mesmo estilo do menu
// e das arenas.
//
// Os personagens seguem a numeracao do jogo:
// 1 = KAI, 2 = LUNA, 3 = NERO, 4 = AYRA (0 = ainda nao escolheu)
// =====================================

// TELA DE ESCOLHA DE PERSONAGENS
void desenharSelecaoPixel(int personagemJogador1, int personagemJogador2);

// TELA DE RANKING (le o arquivo dados/ranking.txt ao entrar na tela)
void desenharRankingPixel(void);

// LIBERA AS TEXTURAS DAS TELAS (chamar antes do CloseWindow)
void descarregarTelas(void);

#endif