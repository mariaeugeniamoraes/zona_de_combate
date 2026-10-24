#ifndef EFEITOS_H
#define EFEITOS_H

#include "raylib.h"
#include <stddef.h>

// =====================================
// EFEITOS VISUAIS
// Cada efeito e um no de uma lista encadeada,
// criado com malloc e liberado com free quando termina.
// =====================================

typedef enum {
    EFEITO_TEXTO,     // "ROUND 1", "LUTEM!", "K.O.", "TEMPO ESGOTADO"
    EFEITO_FAISCA,    // impacto de soco/chute
    EFEITO_FLASH,     // tela piscando
    EFEITO_RAIO,      // especial do KAI
    EFEITO_FOGO,      // especial da LUNA
    EFEITO_AGUA,      // especial do NERO
    EFEITO_VENTO      // especial da AYRA
} TipoEfeito;

typedef struct Efeito {
    TipoEfeito tipo;
    Vector2 posicao;
    float atraso;       // segundos ate o efeito aparecer
    float duracao;      // segundos que o efeito fica na tela
    float tempoVivo;    // quanto tempo ja passou desde que apareceu
    int direcao;        // 1 = direita, -1 = esquerda
    int semente;        // usada para variar o desenho de cada efeito
    Color cor;
    char texto[32];
    struct Efeito *proximo;
} Efeito;

// CRIACAO
void anunciarTexto(Efeito **lista, const char *texto, Color cor, float atraso, float duracao);
void efeitoImpacto(Efeito **lista, Vector2 posicao);
void efeitoFlash(Efeito **lista, Color cor, float duracao);
void efeitoEspecial(Efeito **lista, TipoEfeito elemento, Vector2 posicao, int direcao);

// CICLO DE VIDA
void atualizarEfeitos(Efeito **lista, float dt);
void desenharEfeitos(const Efeito *lista);
void liberarEfeitos(Efeito **lista);
int contarEfeitos(const Efeito *lista);

#endif