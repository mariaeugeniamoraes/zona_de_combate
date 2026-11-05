#ifndef JANELA_H
#define JANELA_H

#include "raylib.h"

// =====================================
// TELA VIRTUAL
// O jogo inteiro e desenhado numa tela fixa de 960x540
// e depois ampliado para o tamanho real da janela,
// mantendo a proporcao (com faixas pretas se precisar).
// Assim o jogo funciona em qualquer tamanho e em tela cheia.
// =====================================
#define JANELA_LARGURA 960
#define JANELA_ALTURA  540

// CRIA A JANELA (redimensionavel) E A TELA VIRTUAL
void iniciarJanela(const char *titulo);

// CHAMAR NO COMECO DE CADA VOLTA DO LOOP
// (F11 alterna tela cheia e o mouse e convertido para a tela virtual)
void atualizarJanela(void);

// SUBSTITUEM O BeginDrawing/ClearBackground E O EndDrawing
void comecarQuadro(void);
void terminarQuadro(void);

// PARA MODULOS QUE USAM A PROPRIA RenderTexture:
// chamar logo depois do EndTextureMode() deles para voltar a desenhar na tela virtual
void retomarQuadro(void);

// LIBERA A TELA VIRTUAL E FECHA A JANELA
void fecharJanela(void);

#endif