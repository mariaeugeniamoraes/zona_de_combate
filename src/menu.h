#ifndef MENU_H
#define MENU_H

#include "raylib.h"

// =====================================
// MENU INICIAL EM PIXEL ART
// O menu e desenhado numa tela pequena de 320x180
// e depois ampliado (3x numa janela de 960x540),
// o que deixa tudo com cara de pixel art.
// Mostra os 4 lutadores (KAI, NERO, LUNA, AYRA)
// usando os sprites de "parado" de cada um.
//
// Controles: SETAS ou W/S para escolher, ENTER/ESPACO
// para confirmar, atalhos 1/2/3 e mouse.
// =====================================

typedef enum {
    OPCAO_NENHUMA = 0,
    OPCAO_JOGAR,
    OPCAO_RANKING,
    OPCAO_SAIR
} OpcaoMenu;

// LE O TECLADO/MOUSE E ANIMA O MENU (chamar uma vez por frame no estado MENU)
// Retorna a opcao confirmada, ou OPCAO_NENHUMA se nada foi escolhido.
OpcaoMenu atualizarMenuInicial(float dt);

// DESENHA O MENU (chamar entre BeginDrawing e EndDrawing)
void desenharMenuInicial(void);

// LIBERA AS TEXTURAS DO MENU (chamar antes do CloseWindow)
void descarregarMenuInicial(void);

#endif
