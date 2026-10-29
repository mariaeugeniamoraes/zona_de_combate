#include "personagens.h"
#include <stddef.h>
#include <stdio.h>

#define TAMANHO_FRAME 64
#define INTERVALO_FRAME 0.125f

static Texture2D texturas[TOTAL_PERSONAGENS][TOTAL_ANIMACOES];

static bool personagensCarregados = false;

static const int quantidadeFrames[TOTAL_ANIMACOES] = {
4, 6, 4, 5, 3, 6, 3, 3, 6, 6, 8
};

static const char *arquivos[TOTAL_PERSONAGENS][TOTAL_ANIMACOES] = {
{
"assets/personagens/luna/luna_parado.png",
"assets/personagens/luna/luna_caminhando.png",
"assets/personagens/luna/luna_socando.png",
"assets/personagens/luna/luna_chutando.png",
"assets/personagens/luna/luna_defendendo.png",
"assets/personagens/luna/luna_pulando.png",
"assets/personagens/luna/luna_agachando.png",
"assets/personagens/luna/luna_dano.png",
"assets/personagens/luna/luna_derrotado.png",
"assets/personagens/luna/luna_vitoria.png",
"assets/personagens/luna/luna_especial.png"
},
{
"assets/personagens/ayra/ayra_parado.png",
"assets/personagens/ayra/ayra_caminhando.png",
"assets/personagens/ayra/ayra_socando.png",
"assets/personagens/ayra/ayra_chutando.png",
"assets/personagens/ayra/ayra_defendendo.png",
"assets/personagens/ayra/ayra_pulando.png",
"assets/personagens/ayra/ayra_agachando.png",
"assets/personagens/ayra/ayra_dano.png",
"assets/personagens/ayra/ayra_derrotado.png",
"assets/personagens/ayra/ayra_vitoria.png",
"assets/personagens/ayra/ayra_especial.png"
},
{
"assets/personagens/kai/kai_parado.png",
"assets/personagens/kai/kai_caminhando.png",
"assets/personagens/kai/kai_socando.png",
"assets/personagens/kai/kai_chutando.png",
"assets/personagens/kai/kai_defendendo.png",
"assets/personagens/kai/kai_pulando.png",
"assets/personagens/kai/kai_agachando.png",
"assets/personagens/kai/kai_dano.png",
"assets/personagens/kai/kai_derrotado.png",
"assets/personagens/kai/kai_vitoria.png",
"assets/personagens/kai/kai_especial.png"
},
{
"assets/personagens/nero/nero_parado.png",
"assets/personagens/nero/nero_caminhando.png",
"assets/personagens/nero/nero_socando.png",
"assets/personagens/nero/nero_chutando.png",
"assets/personagens/nero/nero_defendendo.png",
"assets/personagens/nero/nero_pulando.png",
"assets/personagens/nero/nero_agachando.png",
"assets/personagens/nero/nero_dano.png",
"assets/personagens/nero/nero_derrotado.png",
"assets/personagens/nero/nero_vitoria.png",
"assets/personagens/nero/nero_especial.png"
}
};

void carregarPersonagens(void) {
    if (personagensCarregados) {
        return;
    }

    for (int p = 0; p < TOTAL_PERSONAGENS; p++) {
        TraceLog(LOG_INFO, "Carregando personagem: %d", p);

        for (int a = 0; a < TOTAL_ANIMACOES; a++) {
            texturas[p][a] = LoadTexture(arquivos[p][a]);

            if (texturas[p][a].id != 0) {
                SetTextureFilter(
                    texturas[p][a],
                    TEXTURE_FILTER_POINT
                );
            } else {
                TraceLog(
                    LOG_WARNING,
                    "Nao foi possivel carregar: %s",
                    arquivos[p][a]
                );
            }
        }
    }

    personagensCarregados = true;
}

void iniciarAnimacaoPersonagem(EstadoAnimacao *estado) {
    if (estado == NULL) {
        return;
    }

    estado->animacao = ANIMACAO_PARADA;
    estado->frameAtual = 0;
    estado->tempoAnimacao = 0.0f;
    estado->terminou = false;
}

void definirAnimacaoPersonagem(EstadoAnimacao *estado, int animacao) {
    if (estado == NULL ||
        animacao < 0 ||
        animacao >= TOTAL_ANIMACOES) {
        return;
    }

    if (estado->animacao != animacao) {
        estado->animacao = animacao;
        estado->frameAtual = 0;
        estado->tempoAnimacao = 0.0f;
        estado->terminou = false;
    }
}

void atualizarAnimacaoPersonagem(EstadoAnimacao *estado, float dt) {
    if (estado == NULL || !personagensCarregados) {
        return;
    }

    if (estado->animacao < 0 ||
        estado->animacao >= TOTAL_ANIMACOES) {
        return;
    }

    if (estado->terminou) {
        return;
    }

    estado->tempoAnimacao += dt;

    while (estado->tempoAnimacao >= INTERVALO_FRAME) {
        estado->tempoAnimacao -= INTERVALO_FRAME;
        estado->frameAtual++;

        if (estado->frameAtual >= quantidadeFrames[estado->animacao]) {

            if (estado->animacao == ANIMACAO_ESPECIAL) {
                estado->frameAtual =
                    quantidadeFrames[estado->animacao] - 1;

                estado->tempoAnimacao = 0.0f;
                estado->terminou = true;
                return;
            }

            estado->frameAtual = 0;
        }
    }
}

void desenharPersonagem(
int personagem,
Vector2 posicao,
bool viradaEsquerda,
const EstadoAnimacao *estado
) {
if (estado == NULL || !personagensCarregados) {
return;
}

if (personagem < 0 || personagem >= TOTAL_PERSONAGENS) {
    return;
}

if (estado->animacao < 0 || estado->animacao >= TOTAL_ANIMACOES) {
    return;
}

Texture2D textura = texturas[personagem][estado->animacao];

if (textura.id == 0) {
    return;
}

Rectangle origem = {
    (float)(estado->frameAtual * TAMANHO_FRAME),
    0.0f,
    (float)TAMANHO_FRAME,
    (float)TAMANHO_FRAME
};

if (viradaEsquerda) {
    origem.width = -(float)TAMANHO_FRAME;
}

Rectangle destino = {
    posicao.x,
    posicao.y - 128.0f,
    128.0f,
    128.0f
};

Vector2 centro = {64.0f, 0.0f};

DrawTexturePro(textura, origem, destino, centro, 0.0f, WHITE);

}

void descarregarPersonagens(void) {
if (!personagensCarregados) {
return;
}

for (int p = 0; p < TOTAL_PERSONAGENS; p++) {
    for (int a = 0; a < TOTAL_ANIMACOES; a++) {
        if (texturas[p][a].id != 0) {
            UnloadTexture(texturas[p][a]);
            texturas[p][a] = (Texture2D){0};
        }
    }
}

personagensCarregados = false;

}

/* Compatibilidade temporaria com a Luna */

void carregarLuna(void) {
carregarPersonagens();
}

void descarregarLuna(void) {
descarregarPersonagens();
}


void iniciarAnimacaoLuna(EstadoAnimacaoLuna *estado) {
iniciarAnimacaoPersonagem(estado);
}

void definirAnimacaoLunaJogador(EstadoAnimacaoLuna *estado, AnimacaoLuna animacao) {
definirAnimacaoPersonagem(estado, animacao);
}

void atualizarAnimacaoLunaJogador(EstadoAnimacaoLuna *estado, float dt) {
atualizarAnimacaoPersonagem(estado, dt);
}

void desenharLunaJogador(
Vector2 posicao,
bool viradaEsquerda,
const EstadoAnimacaoLuna *estado
) {
desenharPersonagem(PERSONAGEM_LUNA, posicao, viradaEsquerda, estado);
}
