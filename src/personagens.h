
#ifndef PERSONAGENS_H
#define PERSONAGENS_H

#include "raylib.h"

typedef enum {
    LUNA_PARADA,
    LUNA_CAMINHANDO,
    LUNA_SOCANDO,
    LUNA_CHUTANDO,
    LUNA_DEFENDENDO,
    LUNA_PULANDO,
    LUNA_AGACHANDO,
    LUNA_DANO,
    LUNA_DERROTADA,
    LUNA_VITORIA,
    LUNA_ESPECIAL
} AnimacaoLuna;

// Cada jogador possui sua propria animacao
typedef struct {
    AnimacaoLuna animacao;
    int frameAtual;
    float tempoAnimacao;
} EstadoAnimacaoLuna;

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

// Animacoes compartilhadas pelos quatro personagens
void carregarOutrosPersonagens(void);
void descarregarOutrosPersonagens(void);

void desenharOutroPersonagem(
    int personagem,
    Vector2 posicao,
    bool viradoEsquerda,
    const EstadoAnimacaoLuna *estado
);

#endif
