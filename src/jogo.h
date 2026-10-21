#ifndef JOGO_H
#define JOGO_H

typedef enum {
    MENU,
    SELECAO,
    LUTA,
    VITORIA,
    RANKING
} EstadoJogo;


typedef struct {
    char nome[30];
    int vitorias;
} RegistroRanking;

void desenharMenu(void);
void salvarVitoria(int personagemVencedor);
void desenharRanking(void);
void desenharVitoria(int vencedorPartida, int vitoriasJogador1, int vitoriasJogador2);
void desenharSelecao(int personagemJogador1, int personagemJogador2);

#endif