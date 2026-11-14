#ifndef SELECAO_ARENA_H
#define SELECAO_ARENA_H

#include "raylib.h"

// Valores especiais retornados pela seleção
#define SELECAO_ARENA_NADA -1
#define SELECAO_ARENA_VOLTAR -2

// Atualiza a seleção de arena
int atualizarSelecaoArena(void);

// Desenha a tela de seleção de arena
void desenharSelecaoArena(int personagem1, int personagem2);

#endif