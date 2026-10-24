
#include "personagens.h"
#include <stddef.h>

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
