
#include "personagens.h"
#include <stddef.h>
#include <stdio.h>

#define TOTAL_ANIMACOES_LUNA 11
#define TAMANHO_FRAME 64

static Texture2D texturasLuna[TOTAL_ANIMACOES_LUNA];
static bool lunaCarregada = false;

static const int quantidadeFrames[TOTAL_ANIMACOES_LUNA] = {
    4, 6, 4, 5, 3, 6, 3, 3, 6, 6, 8
};

void carregarLuna(void) {

    if (lunaCarregada) {
        return;
    }

    const char *arquivos[TOTAL_ANIMACOES_LUNA] = {
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
    };

    for (int i = 0; i < TOTAL_ANIMACOES_LUNA; i++) {

        texturasLuna[i] = LoadTexture(arquivos[i]);

        if (texturasLuna[i].id != 0) {
            SetTextureFilter(
                texturasLuna[i],
                TEXTURE_FILTER_POINT
            );
        }
    }

    lunaCarregada = true;
}

void iniciarAnimacaoLuna(EstadoAnimacaoLuna *estado) {

    if (estado == NULL) {
        return;
    }

    estado->animacao = LUNA_PARADA;
    estado->frameAtual = 0;
    estado->tempoAnimacao = 0.0f;
}

void definirAnimacaoLunaJogador(
    EstadoAnimacaoLuna *estado,
    AnimacaoLuna animacao
) {

    if (estado == NULL) {
        return;
    }

    if (animacao < 0 || animacao >= TOTAL_ANIMACOES_LUNA) {
        return;
    }

    if (estado->animacao != animacao) {

        estado->animacao = animacao;
        estado->frameAtual = 0;
        estado->tempoAnimacao = 0.0f;
    }
}

void atualizarAnimacaoLunaJogador(
    EstadoAnimacaoLuna *estado,
    float dt
) {

    if (estado == NULL || !lunaCarregada) {
        return;
    }

    estado->tempoAnimacao += dt;

    const float intervaloFrame = 0.125f;

    while (estado->tempoAnimacao >= intervaloFrame) {

        estado->tempoAnimacao -= intervaloFrame;
        estado->frameAtual++;

        if (estado->frameAtual >=
            quantidadeFrames[estado->animacao]) {

            estado->frameAtual = 0;
        }
    }
}

void desenharLunaJogador(
    Vector2 posicao,
    bool viradaEsquerda,
    const EstadoAnimacaoLuna *estado
) {

    if (estado == NULL || !lunaCarregada) {
        return;
    }

    Texture2D textura = texturasLuna[estado->animacao];

    if (textura.id == 0) {
        return;
    }

    Rectangle origem = {
        (float)(estado->frameAtual * TAMANHO_FRAME),
        0.0f,
        64.0f,
        64.0f
    };

    if (viradaEsquerda) {
        origem.width = -64.0f;
    }

    Rectangle destino = {
        posicao.x,
        posicao.y - 128.0f,
        128.0f,
        128.0f
    };

    Vector2 centro = {
        64.0f,
        0.0f
    };

    DrawTexturePro(
        textura,
        origem,
        destino,
        centro,
        0.0f,
        WHITE
    );
}

void descarregarLuna(void) {

    if (!lunaCarregada) {
        return;
    }

    for (int i = 0; i < TOTAL_ANIMACOES_LUNA; i++) {

        if (texturasLuna[i].id != 0) {
            UnloadTexture(texturasLuna[i]);
        }
    }

    lunaCarregada = false;
}

// =====================================
// KAI, NERO E AYRA
// =====================================

#define TOTAL_OUTROS_PERSONAGENS 3
#define TOTAL_ANIMACOES 11

static Texture2D texturasOutros[3][11];
static bool outrosCarregados = false;

// Ordem: Kai, Nero e Ayra
static const char *nomesPersonagens[3] = {
    "kai",
    "nero",
    "ayra"
};

// Mesma ordem do enum AnimacaoLuna
static const char *nomesAnimacoes[11] = {
    "parado",
    "caminhando",
    "socando",
    "chutando",
    "defendendo",
    "pulando",
    "agachando",
    "dano",
    "derrotado",
    "vitoria",
    "especial"
};

static int indiceOutroPersonagem(int personagem) {
    if (personagem == 1) return 0; // Kai
    if (personagem == 3) return 1; // Nero
    if (personagem == 4) return 2; // Ayra
    return -1;
}

void carregarOutrosPersonagens(void) {

    if (outrosCarregados) {
        return;
    }

    for (int p = 0; p < TOTAL_OUTROS_PERSONAGENS; p++) {

        for (int a = 0; a < TOTAL_ANIMACOES; a++) {

            char caminho[256];

            snprintf(
                caminho,
                sizeof(caminho),
                "assets/personagens/%s/%s_%s.png",
                nomesPersonagens[p],
                nomesPersonagens[p],
                nomesAnimacoes[a]
            );

            texturasOutros[p][a] = LoadTexture(caminho);

            if (texturasOutros[p][a].id != 0) {
                SetTextureFilter(
                    texturasOutros[p][a],
                    TEXTURE_FILTER_POINT
                );
            }
        }
    }

    outrosCarregados = true;
}

void desenharOutroPersonagem(
    int personagem,
    Vector2 posicao,
    bool viradoEsquerda,
    const EstadoAnimacaoLuna *estado
) {

    int indice = indiceOutroPersonagem(personagem);

    if (
        indice < 0 ||
        estado == NULL ||
        !outrosCarregados
    ) {
        return;
    }

    int animacao = estado->animacao;

    if (animacao < 0 || animacao >= TOTAL_ANIMACOES) {
        return;
    }

    Texture2D textura = texturasOutros[indice][animacao];

    if (textura.id == 0) {
        return;
    }

    // Calcula a largura de cada quadro.
    // Pressupoe sprites com quadros de 64x64.
    int totalFrames = quantidadeFrames[animacao];

    int larguraFrame = textura.width / totalFrames;

    int frame = estado->frameAtual % totalFrames;

    Rectangle origem = {
        (float)(frame * larguraFrame),
        0.0f,
        (float)larguraFrame,
        (float)textura.height
    };

    if (viradoEsquerda) {
        origem.width = -origem.width;
    }

    Rectangle destino = {
        posicao.x,
        posicao.y - 128.0f,
        128.0f,
        128.0f
    };

    Vector2 centro = {
        64.0f,
        0.0f
    };

    DrawTexturePro(
        textura,
        origem,
        destino,
        centro,
        0.0f,
        WHITE
    );
}

void descarregarOutrosPersonagens(void) {

    if (!outrosCarregados) {
        return;
    }

    for (int p = 0; p < TOTAL_OUTROS_PERSONAGENS; p++) {

        for (int a = 0; a < TOTAL_ANIMACOES; a++) {

            if (texturasOutros[p][a].id != 0) {
                UnloadTexture(texturasOutros[p][a]);
            }
        }
    }

    outrosCarregados = false;
}
