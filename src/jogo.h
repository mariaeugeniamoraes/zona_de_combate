#ifndef JOGO_H
#define JOGO_H

typedef enum {
    MENU,
    SELECAO,
    LUTA,
    VITORIA,
    RANKING
} EstadoJogo;

void desenharMenu(void);
#endif