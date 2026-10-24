#ifndef INTERFACE_H
#define INTERFACE_H

#include "raylib.h"
#include "jogo.h"

// =====================================
// HUD DA LUTA
// Barras de vida, energia, cronometro,
// numero do round e vitorias de cada jogador.
// =====================================

// ATUALIZA ANIMACOES DO HUD (chamar uma vez por frame durante a luta)
void atualizarHUD(const Partida *partida, float dt);

// DESENHA O HUD (chamar depois de desenhar a arena e os personagens)
// energia vai de 0 a 100
void desenharHUD(
    const Partida *partida,
    int personagemJogador1,
    int personagemJogador2,
    float energiaJogador1,
    float energiaJogador2
);

// DADOS VISUAIS DOS PERSONAGENS (1 = KAI, 2 = LUNA, 3 = NERO, 4 = AYRA)
const char *nomePersonagem(int personagem);
Color corPersonagem(int personagem);

#endif