
#ifndef JOGO_H
#define JOGO_H

// ESTADOS DO JOGO
typedef enum {
    MENU,
    SELECAO,
    LUTA,
    VITORIA,
    RANKING
} EstadoJogo;

// ESTRUTURA PARA O RANKING
typedef struct {
    char nome[30];
    int vitorias;
} RegistroRanking;

// ESTRUTURA PARA OS DADOS DA PARTIDA
typedef struct {
    int roundAtual;
    int vitoriasJogador1;
    int vitoriasJogador2;
    int vidaJogador1;
    int vidaJogador2;
    float tempoRestante;
} Partida;

// FUNCOES DAS TELAS
void desenharMenu(void);

void desenharSelecao(
    int personagemJogador1,
    int personagemJogador2
);

void desenharVitoria(
    int vencedorPartida,
    int vitoriasJogador1,
    int vitoriasJogador2
);

void desenharRanking(void);

// FUNCAO DO RANKING
void salvarVitoria(int personagemVencedor);

// FUNCOES DA PARTIDA
void iniciarPartida(Partida *partida);
void reiniciarRound(Partida *partida);

#endif
