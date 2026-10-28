#ifndef PERSONAGENS_H
#define PERSONAGENS_H

#include "raylib.h"

// Identificadores dos personagens
#define PERSONAGEM_LUNA 0
#define PERSONAGEM_AYRA 1
#define PERSONAGEM_KAI  2
#define PERSONAGEM_NERO 3

// Identificadores das animações
#define ANIMACAO_PARADA     0
#define ANIMACAO_CAMINHANDO 1
#define ANIMACAO_SOCANDO    2
#define ANIMACAO_CHUTANDO   3
#define ANIMACAO_DEFENDENDO 4
#define ANIMACAO_PULANDO    5
#define ANIMACAO_AGACHANDO  6
#define ANIMACAO_DANO       7
#define ANIMACAO_DERROTADA  8
#define ANIMACAO_VITORIA    9
#define ANIMACAO_ESPECIAL   10

#define TOTAL_PERSONAGENS 4
#define TOTAL_ANIMACOES 11

// Compatibilidade com o codigo antigo da Luna
#define LUNA_PARADA     ANIMACAO_PARADA
#define LUNA_CAMINHANDO ANIMACAO_CAMINHANDO
#define LUNA_SOCANDO    ANIMACAO_SOCANDO
#define LUNA_CHUTANDO   ANIMACAO_CHUTANDO
#define LUNA_DEFENDENDO ANIMACAO_DEFENDENDO
#define LUNA_PULANDO    ANIMACAO_PULANDO
#define LUNA_AGACHANDO  ANIMACAO_AGACHANDO
#define LUNA_DANO       ANIMACAO_DANO
#define LUNA_DERROTADA  ANIMACAO_DERROTADA
#define LUNA_VITORIA    ANIMACAO_VITORIA
#define LUNA_ESPECIAL   ANIMACAO_ESPECIAL

// Estado da animacao de cada jogador
typedef struct {
    int animacao;
    int frameAtual;
    float tempoAnimacao;
    bool terminou;
} EstadoAnimacao;

// Mantem os nomes antigos usados pela Luna
typedef EstadoAnimacao EstadoAnimacaoLuna;
typedef int AnimacaoLuna;

// Carregamento e descarregamento dos personagens
void carregarPersonagens(void);
void descarregarPersonagens(void);

// Controle das animacoes
void iniciarAnimacaoPersonagem(EstadoAnimacao *estado);

void definirAnimacaoPersonagem(
    EstadoAnimacao *estado,
    int animacao
);

void atualizarAnimacaoPersonagem(
    EstadoAnimacao *estado,
    float dt
);

// Desenho do personagem selecionado
void desenharPersonagem(
    int personagem,
    Vector2 posicao,
    bool viradaEsquerda,
    const EstadoAnimacao *estado
);

// Funcoes antigas da Luna, mantidas temporariamente
void carregarLuna(void);
void descarregarLuna(void);

void iniciarAnimacaoLuna(EstadoAnimacaoLuna *estado);

void definirAnimacaoLunaJogador(
    EstadoAnimacaoLuna *estado,
    AnimacaoLuna animacao
);

void atualizarAnimacaoLunaJogador(
    EstadoAnimacaoLuna *estado,
    float dt
);

void desenharLunaJogador(
    Vector2 posicao,
    bool viradaEsquerda,
    const EstadoAnimacaoLuna *estado
);

#endif