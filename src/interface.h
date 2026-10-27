#ifndef INTERFACE_H
#define INTERFACE_H

#include "raylib.h"
#include "jogo.h"
#include "personagens.h"

// =====================================
// HUD DA LUTA
// Barras de vida, energia, cronometro,
// numero do round e vitorias de cada jogador.
// =====================================

// Atualiza o HUD uma vez por frame durante a luta.
void atualizarHUD(const Partida *partida, float dt);

// Desenha o HUD depois da arena e dos personagens.
// A energia vai de 0 a 100.
void desenharHUD(
    const Partida *partida,
    int personagemJogador1,
    int personagemJogador2,
    float energiaJogador1,
    float energiaJogador2
);

// Dados visuais dos personagens.
// Os índices são definidos em personagens.h.
const char *nomePersonagem(int personagem);
Color corPersonagem(int personagem);

#endif