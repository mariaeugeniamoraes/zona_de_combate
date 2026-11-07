#ifndef CONTROLES_H
#define CONTROLES_H

#include "raylib.h"

// =====================================
// TELA DE CONTROLES EM PIXEL ART
// Mostra as teclas dos dois jogadores, para que serve
// cada acao e as teclas gerais (pausa, menu, tela cheia).
// Desenhada numa tela de 320x180 e ampliada 3x.
// =====================================

// DESENHA A TELA DE CONTROLES (chamar entre comecarQuadro e terminarQuadro)
void desenharControlesPixel(void);

// LIBERA A TEXTURA DA TELA (chamar antes de fecharJanela)
void descarregarControles(void);

#endif
