#include "telas.h"
#include <math.h>
#include <stdio.h>

// =====================================
// TELA BASE EM PIXEL ART
// =====================================
#define BASE_W 320
#define BASE_H 180
#define CHAO_TELAS 150

#define TAMANHO_FRAME   64
#define PE_NO_FRAME     61
#define CENTRO_NO_FRAME 30

#define TOTAL_LUTADORES 4

#define COR_SOMBRA  (Color){ 22, 12, 30, 255 }
#define COR_LARANJA (Color){ 255, 140, 40, 255 }
#define COR_AMARELO (Color){ 255, 214, 80, 255 }
#define COR_J1      (Color){ 80, 170, 255, 255 }
#define COR_J2      (Color){ 255, 80, 90, 255 }

// =====================================
// LUTADORES
// =====================================
typedef struct {
    const char *nome;
    const char *elemento;
    const char *teclaJogador1;
    const char *teclaJogador2;
    const char *arquivoParado;
    const char *arquivoVitoria;
    const char *arquivoAura;
    const char *arquivoDerrotado;
    Color cor;
    Color corEscura;
    Texture2D parado;
    Texture2D vitoria;
    Texture2D aura;
    Texture2D derrotado;
} LutadorTela;

static LutadorTela lutadores[TOTAL_LUTADORES] = {
    { "KAI", "RAIO", "1", "Q",
      "assets/personagens/kai/kai_parado.png",
      "assets/personagens/kai/kai_vitoria.png",
      "assets/personagens/kai/efeitos/aura_eletrica.png",
      "assets/personagens/kai/kai_derrotado.png",
      { 255, 225, 80, 255 },
      { 90, 70, 20, 255 },
      { 0 }, { 0 }, { 0 }, { 0 } },

    { "LUNA", "FOGO", "2", "W",
      "assets/personagens/luna/luna_parado.png",
      "assets/personagens/luna/luna_vitoria.png",
      "assets/personagens/luna/efeitos/aura_fogo.png",
      "assets/personagens/luna/luna_derrotado.png",
      { 255, 110, 50, 255 },
      { 100, 26, 22, 255 },
      { 0 }, { 0 }, { 0 }, { 0 } },

    { "NERO", "AGUA", "3", "E",
      "assets/personagens/nero/nero_parado.png",
      "assets/personagens/nero/nero_vitoria.png",
      "assets/personagens/nero/efeitos/aura_agua.png",
      "assets/personagens/nero/nero_derrotado.png",
      { 90, 180, 255, 255 },
      { 18, 46, 110, 255 },
      { 0 }, { 0 }, { 0 }, { 0 } },

    { "AYRA", "VENTO", "4", "R",
      "assets/personagens/ayra/ayra_parado.png",
      "assets/personagens/ayra/ayra_vitoria.png",
      "assets/personagens/ayra/efeitos/aura_vento.png",
      "assets/personagens/ayra/ayra_derrotado.png",
      { 120, 230, 170, 255 },
      { 22, 80, 64, 255 },
      { 0 }, { 0 }, { 0 }, { 0 } }
};

#define FRAMES_PARADO  4
#define FRAMES_VITORIA 6
#define FRAMES_AURA    6
#define FRAMES_DERROTA 6

// =====================================
// ESTADO INTERNO
// =====================================
static RenderTexture2D telaPixel;
static int recursosCarregados = 0;

// SELECAO
static double ultimoDesenhoSelecao = -10.0;
static float tempoSelecao = 0.0f;
static int escolhaAnterior[2] = { 0, 0 };
static float flashEscolha[2] = { 0.0f, 0.0f };

// FIM DE PARTIDA
static double ultimoDesenhoVitoria = -10.0;
static float tempoVitoria = 0.0f;

// RANKING
static double ultimoDesenhoRanking = -10.0;
static float tempoRanking = 0.0f;
static int ordemRanking[TOTAL_LUTADORES] = { 0, 1, 2, 3 };
static int vitoriasRanking[TOTAL_LUTADORES] = { 0, 0, 0, 0 };
static int totalVitorias = 0;

// =====================================
// FUNCOES AUXILIARES
// =====================================
static int ruido(int a, int b) {

    unsigned int h =
        (unsigned int)a * 73856093u ^
        (unsigned int)b * 19349663u;

    h ^= h >> 13;
    h *= 0x5bd1e995u;
    h ^= h >> 15;

    return (int)(h & 0x7fffffff);
}

static Color misturarCores(Color a, Color b, float t) {

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

static float degrau(float t, int degraus) {

    if (t < 0.0f) t = 0.0f;
    if (t > 0.999f) t = 0.999f;

    return floorf(t * degraus) / (float)(degraus - 1);
}

static void triangulo(Vector2 a, Vector2 b, Vector2 c, Color cor) {

    float giro =
        (b.x - a.x) * (c.y - a.y) -
        (b.y - a.y) * (c.x - a.x);

    if (giro < 0.0f) {
        DrawTriangle(a, b, c, cor);
    }
    else {
        DrawTriangle(a, c, b, cor);
    }
}

static void quadrilatero(
    Vector2 a,
    Vector2 b,
    Vector2 c,
    Vector2 d,
    Color cor
) {

    triangulo(a, d, c, cor);
    triangulo(a, c, b, cor);
}

static Vector2 deslocar(Vector2 p, float dx, float dy) {

    return (Vector2){ p.x + dx, p.y + dy };
}

// =====================================
// TEXTO COM CONTORNO E SOMBRA
// =====================================
static void textoContorno(
    const char *texto,
    float x,
    float y,
    float tamanho,
    float espaco,
    int contorno,
    int sombra,
    Color corContorno,
    Color cor
) {

    Font fonte = GetFontDefault();
    Vector2 pos = { floorf(x), floorf(y) };

    if (sombra > 0) {
        DrawTextEx(
            fonte,
            texto,
            deslocar(pos, (float)sombra, (float)sombra),
            tamanho,
            espaco,
            Fade(BLACK, 0.6f)
        );
    }

    for (int dx = -contorno; dx <= contorno; dx++) {
        for (int dy = -contorno; dy <= contorno; dy++) {

            if (dx != 0 || dy != 0) {
                DrawTextEx(
                    fonte,
                    texto,
                    deslocar(pos, (float)dx, (float)dy),
                    tamanho,
                    espaco,
                    corContorno
                );
            }
        }
    }

    DrawTextEx(fonte, texto, pos, tamanho, espaco, cor);
}

static float larguraTexto(
    const char *texto,
    float tamanho,
    float espaco
) {

    return MeasureTextEx(
        GetFontDefault(),
        texto,
        tamanho,
        espaco
    ).x;
}

static void textoCentralizado(
    const char *texto,
    float centroX,
    float y,
    float tamanho,
    float espaco,
    int contorno,
    Color corContorno,
    Color cor
) {

    textoContorno(
        texto,
        centroX - larguraTexto(texto, tamanho, espaco) / 2.0f,
        y,
        tamanho,
        espaco,
        contorno,
        contorno > 0 ? 1 : 0,
        corContorno,
        cor
    );
}

// =====================================
// CARREGAR E LIBERAR TEXTURAS
// =====================================
static Texture2D carregarSeExistir(const char *arquivo) {

    Texture2D textura = { 0 };

    if (FileExists(arquivo)) {

        textura = LoadTexture(arquivo);

        if (textura.id != 0) {
            SetTextureFilter(textura, TEXTURE_FILTER_POINT);
        }
    }

    return textura;
}

static void carregarRecursos(void) {

    if (recursosCarregados) {
        return;
    }

    telaPixel = LoadRenderTexture(BASE_W, BASE_H);
    SetTextureFilter(
        telaPixel.texture,
        TEXTURE_FILTER_POINT
    );

    for (int i = 0; i < TOTAL_LUTADORES; i++) {

        lutadores[i].parado =
            carregarSeExistir(lutadores[i].arquivoParado);

        lutadores[i].vitoria =
            carregarSeExistir(lutadores[i].arquivoVitoria);

        lutadores[i].aura =
            carregarSeExistir(lutadores[i].arquivoAura);

        lutadores[i].derrotado =
            carregarSeExistir(lutadores[i].arquivoDerrotado);
    }

    recursosCarregados = 1;
}

void descarregarTelas(void) {

    if (!recursosCarregados) {
        return;
    }

    UnloadRenderTexture(telaPixel);

    for (int i = 0; i < TOTAL_LUTADORES; i++) {

        if (lutadores[i].parado.id != 0) {
            UnloadTexture(lutadores[i].parado);
        }

        if (lutadores[i].vitoria.id != 0) {
            UnloadTexture(lutadores[i].vitoria);
        }

        if (lutadores[i].aura.id != 0) {
            UnloadTexture(lutadores[i].aura);
        }

        if (lutadores[i].derrotado.id != 0) {
            UnloadTexture(lutadores[i].derrotado);
        }

        lutadores[i].parado = (Texture2D){ 0 };
        lutadores[i].vitoria = (Texture2D){ 0 };
        lutadores[i].aura = (Texture2D){ 0 };
        lutadores[i].derrotado = (Texture2D){ 0 };
    }

    recursosCarregados = 0;
}

// =====================================
// COMECAR E TERMINAR O DESENHO
// =====================================
static void comecarTelaPixel(void) {

    carregarRecursos();
    BeginTextureMode(telaPixel);
    ClearBackground(BLACK);
}

static void terminarTelaPixel(void) {

    EndTextureMode();

    float escala = fminf(
        GetScreenWidth() / (float)BASE_W,
        GetScreenHeight() / (float)BASE_H
    );

    if (escala >= 1.0f) {
        escala = floorf(escala);
    }

    float x =
        (GetScreenWidth() - BASE_W * escala) / 2.0f;

    float y =
        (GetScreenHeight() - BASE_H * escala) / 2.0f;

    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

    DrawTexturePro(
        telaPixel.texture,
        (Rectangle){ 0, 0, (float)BASE_W, -(float)BASE_H },
        (Rectangle){ x, y, BASE_W * escala, BASE_H * escala },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    EndBlendMode();
}

// =====================================
// DESENHAR UM FRAME DE UM SPRITE
// =====================================
static void desenharFrame(
    Texture2D textura,
    int frame,
    float centroX,
    float peY,
    int olhaDireita,
    Color tinta
) {

    if (textura.id == 0) {
        return;
    }

    float larguraOrigem =
        olhaDireita ? TAMANHO_FRAME : -TAMANHO_FRAME;

    float centro =
        olhaDireita ? CENTRO_NO_FRAME :
        (TAMANHO_FRAME - CENTRO_NO_FRAME);

    Rectangle origem = {
        (float)(frame * TAMANHO_FRAME),
        0,
        larguraOrigem,
        TAMANHO_FRAME
    };

    Vector2 destino = {
        floorf(centroX - centro),
        floorf(peY - PE_NO_FRAME)
    };

    DrawTextureRec(textura, origem, destino, tinta);
}

// =====================================
// FUNDO COMUM DAS TELAS
// =====================================
static void desenharFundo(float tempo, Color tomHorizonte) {

    Color topo = { 12, 8, 30, 255 };
    Color meio = { 44, 18, 66, 255 };

    // CEU EM FAIXAS
    for (int y = 0; y < CHAO_TELAS; y += 3) {

        Color cor;

        if (y < 80) {
            cor = misturarCores(
                topo,
                meio,
                degrau(y / 80.0f, 6)
            );
        }
        else {
            cor = misturarCores(
                meio,
                tomHorizonte,
                degrau(
                    (y - 80) / (float)(CHAO_TELAS - 80),
                    6
                )
            );
        }

        DrawRectangle(0, y, BASE_W, 3, cor);
    }

    // ESTRELAS
    for (int i = 0; i < 40; i++) {

        int x = ruido(i, 11) % BASE_W;
        int y = ruido(i, 12) % 90;

        float brilho =
            0.5f +
            0.5f * sinf(
                tempo * (1.0f + (i % 5) * 0.4f) + i
            );

        if (brilho > 0.25f) {
            DrawPixel(x, y, Fade(WHITE, brilho));
        }
    }

    // FAIXAS DIAGONAIS DE ENERGIA
    for (int i = 0; i < 6; i++) {

        float x =
            fmodf(i * 70.0f + tempo * 25.0f, 420.0f)
            - 80.0f;

        quadrilatero(
            (Vector2){ x + 40, 0 },
            (Vector2){ x + 52, 0 },
            (Vector2){ x + 12, (float)CHAO_TELAS },
            (Vector2){ x, (float)CHAO_TELAS },
            Fade(WHITE, 0.035f)
        );
    }

    // CHAO COM GRADE EM PERSPECTIVA
    int alturaChao = BASE_H - CHAO_TELAS;

    for (int y = CHAO_TELAS; y < BASE_H; y += 3) {

        float t = degrau(
            (y - CHAO_TELAS) / (float)alturaChao,
            4
        );

        DrawRectangle(
            0,
            y,
            BASE_W,
            3,
            misturarCores(
                (Color){ 46, 22, 60, 255 },
                (Color){ 18, 10, 26, 255 },
                t
            )
        );
    }

    for (int i = -12; i <= 12; i++) {

        DrawLineV(
            (Vector2){
                BASE_W / 2.0f + i * 8.0f,
                (float)CHAO_TELAS
            },
            (Vector2){
                BASE_W / 2.0f + i * 50.0f,
                (float)BASE_H
            },
            Fade(COR_LARANJA, 0.3f)
        );
    }

    float avanco = fmodf(tempo * 0.6f, 1.0f);

    for (int k = 0; k < 4; k++) {

        float t = (k + avanco) / 4.0f;

        DrawRectangle(
            0,
            CHAO_TELAS +
                (int)(t * t * alturaChao),
            BASE_W,
            1,
            Fade(
                COR_LARANJA,
                0.15f + 0.3f * t
            )
        );
    }

    DrawRectangle(
        0,
        CHAO_TELAS - 1,
        BASE_W,
        1,
        (Color){ 255, 170, 90, 255 }
    );
}

// =====================================
// RODAPE DAS TELAS
// =====================================
static void desenharRodape(
    const char *esquerda,
    const char *centro,
    Color corCentro,
    float tempo
) {

    DrawRectangle(
        0,
        BASE_H - 11,
        BASE_W,
        11,
        Fade(BLACK, 0.6f)
    );

    if (esquerda != NULL) {

        DrawText(
            esquerda,
            4,
            BASE_H - 10,
            10,
            Fade(WHITE, 0.45f)
        );
    }

    if (centro != NULL) {

        float piscar =
            0.7f + 0.3f * sinf(tempo * 5.0f);

        int largura = MeasureText(centro, 10);

        DrawText(
            centro,
            BASE_W / 2 - largura / 2,
            BASE_H - 10,
            10,
            Fade(corCentro, piscar)
        );
    }
}

// =====================================
// TELA DE SELECAO DE PERSONAGENS
// =====================================
#define CARTA_LARGURA 52
#define CARTA_ALTURA  62
#define CARTA_Y       28
#define CARTA_ESPACO  60

static float xDaCarta(int i) {

    return 44.0f + i * CARTA_ESPACO;
}

// =====================================
// ETIQUETAS DAS CARTAS
// =====================================
static void etiqueta(
    const char *texto,
    float x,
    float y,
    Color fundo,
    Color corTexto
) {

    int largura = MeasureText(texto, 10);

    DrawRectangle(
        (int)x,
        (int)y,
        largura + 4,
        10,
        fundo
    );

    DrawRectangle(
        (int)x,
        (int)y + 9,
        largura + 4,
        1,
        Fade(BLACK, 0.5f)
    );

    DrawText(
        texto,
        (int)x + 2,
        (int)y + 1,
        10,
        corTexto
    );
}

// =====================================
// CARTA DE CADA PERSONAGEM
// =====================================
static void desenharCarta(
    int i,
    int escolhaJ1,
    int escolhaJ2,
    float tempo
) {

    LutadorTela *l = &lutadores[i];

    // ANIMACAO DE ENTRADA DAS CARTAS
    float atraso = i * 0.07f;

    float aparecer =
        (tempoSelecao - atraso) / 0.25f;

    if (aparecer <= 0.0f) {
        return;
    }

    if (aparecer > 1.0f) {
        aparecer = 1.0f;
    }

    int x = (int)xDaCarta(i);

    int y =
        CARTA_Y -
        (int)((1.0f - aparecer) * 30.0f);

    int escolhidaJ1 = (escolhaJ1 == i + 1);
    int escolhidaJ2 = (escolhaJ2 == i + 1);

    int alguemEscolheu =
        escolhidaJ1 || escolhidaJ2;

    int ambosProntos =
        (escolhaJ1 != 0 && escolhaJ2 != 0);

    // SOMBRA
    DrawRectangle(
        x + 2,
        y + 2,
        CARTA_LARGURA,
        CARTA_ALTURA,
        Fade(BLACK, 0.5f)
    );

    // FUNDO NA COR DO ELEMENTO
    for (int k = 0; k < CARTA_ALTURA - 12; k += 2) {

        float t = degrau(
            k / (float)(CARTA_ALTURA - 12),
            5
        );

        DrawRectangle(
            x,
            y + k,
            CARTA_LARGURA,
            2,
            misturarCores(
                l->corEscura,
                misturarCores(
                    l->corEscura,
                    l->cor,
                    0.55f
                ),
                t
            )
        );
    }

    // LISTRAS DIAGONAIS
    for (int k = -60; k < CARTA_LARGURA; k += 10) {

        DrawLine(
            x + k,
            y + CARTA_ALTURA - 12,
            x + k + 50,
            y,
            Fade(WHITE, 0.06f)
        );
    }

    // RETRATO DO PERSONAGEM
    if (l->parado.id != 0) {

        int frame =
            (int)(tempo * 6.0f + i)
            % FRAMES_PARADO;

        Rectangle origem = {
            (float)(frame * TAMANHO_FRAME + 4),
            11.0f,
            (float)CARTA_LARGURA,
            (float)(CARTA_ALTURA - 12)
        };

        Color tinta =
            (ambosProntos && !alguemEscolheu)
            ? (Color){ 120, 120, 130, 255 }
            : WHITE;

        DrawTextureRec(
            l->parado,
            origem,
            (Vector2){ (float)x, (float)y },
            tinta
        );
    }

    // FAIXA DO NOME
    DrawRectangle(
        x,
        y + CARTA_ALTURA - 12,
        CARTA_LARGURA,
        12,
        COR_SOMBRA
    );

    DrawRectangle(
        x,
        y + CARTA_ALTURA - 12,
        CARTA_LARGURA,
        1,
        l->cor
    );

    textoCentralizado(
        l->nome,
        x + CARTA_LARGURA / 2.0f,
        (float)(y + CARTA_ALTURA - 10),
        10,
        2,
        0,
        BLANK,
        l->cor
    );

    // BORDA DAS CARTAS
    float pulso =
        0.6f + 0.4f * sinf(tempo * 8.0f);

    Color borda =
        Fade(
            (Color){ 200, 180, 210, 255 },
            0.8f
        );

    int espessura = 1;

    if (escolhidaJ1 && escolhidaJ2) {

        borda = misturarCores(
            COR_J1,
            COR_J2,
            0.5f + 0.5f * sinf(tempo * 6.0f)
        );

        espessura = 2;
    }
    else if (escolhidaJ1) {

        borda = Fade(COR_J1, pulso);
        espessura = 2;
    }
    else if (escolhidaJ2) {

        borda = Fade(COR_J2, pulso);
        espessura = 2;
    }

    DrawRectangleLinesEx(
        (Rectangle){
            (float)x,
            (float)y,
            CARTA_LARGURA,
            CARTA_ALTURA
        },
        (float)espessura,
        borda
    );

    // TECLAS DOS JOGADORES
    etiqueta(
        l->teclaJogador1,
        (float)x + 1,
        (float)y + 1,
        Fade(
            COR_J1,
            escolhaJ1 == 0 ? 1.0f : 0.35f
        ),
        WHITE
    );

    etiqueta(
        l->teclaJogador2,
        (float)(x + CARTA_LARGURA - 11),
        (float)y + 1,
        Fade(
            COR_J2,
            escolhaJ2 == 0 ? 1.0f : 0.35f
        ),
        WHITE
    );

    // MARCADORES J1 E J2
    int seta =
        (int)(sinf(tempo * 6.0f) * 1.5f);

    if (escolhidaJ1) {

        etiqueta(
            "J1",
            (float)x + 2,
            (float)(
                y + CARTA_ALTURA + 1 + seta
            ),
            COR_J1,
            WHITE
        );
    }

    if (escolhidaJ2) {

        etiqueta(
            "J2",
            (float)(x + CARTA_LARGURA - 17),
            (float)(
                y + CARTA_ALTURA + 1 + seta
            ),
            COR_J2,
            WHITE
        );
    }
}

// =====================================
// PAINEL DOS JOGADORES
// =====================================
static void desenharPainelJogador(
    int jogador,
    int escolha,
    float tempo
) {

    // 0 = JOGADOR 1
    // 1 = JOGADOR 2
    Rectangle painel = {
        jogador == 0 ? 6.0f : 186.0f,
        100.0f,
        128.0f,
        66.0f
    };

    Color corJogador =
        jogador == 0 ? COR_J1 : COR_J2;

    int olhaDireita = (jogador == 0);

    // FUNDO DO PAINEL
    DrawRectangle(
        (int)painel.x + 2,
        (int)painel.y + 2,
        (int)painel.width,
        (int)painel.height,
        Fade(BLACK, 0.5f)
    );

    DrawRectangleRec(
        painel,
        Fade(COR_SOMBRA, 0.88f)
    );

    // CABECALHO
    DrawRectangle(
        (int)painel.x,
        (int)painel.y,
        (int)painel.width,
        11,
        corJogador
    );

    textoCentralizado(
        jogador == 0 ? "JOGADOR 1" : "JOGADOR 2",
        painel.x + painel.width / 2.0f,
        painel.y + 1,
        10,
        2,
        0,
        BLANK,
        WHITE
    );

    float espacoSpriteX =
        olhaDireita
        ? painel.x + 32.0f
        : painel.x + painel.width - 32.0f;

    float peY =
        painel.y + painel.height - 4.0f;

    float textoX =
        olhaDireita
        ? painel.x + 66.0f
        : painel.x + 6.0f;

    if (escolha >= 1 &&
        escolha <= TOTAL_LUTADORES) {

        LutadorTela *l = &lutadores[escolha - 1];

        // BRILHO DO CHAO
        DrawEllipse(
            (int)espacoSpriteX,
            (int)peY,
            22,
            3,
            Fade(l->cor, 0.35f)
        );

        BeginScissorMode(
            (int)painel.x + 1,
            (int)painel.y + 11,
            (int)painel.width - 2,
            (int)painel.height - 12
        );

        // AURA
        desenharFrame(
            l->aura,
            (int)(tempo * 12.0f) % FRAMES_AURA,
            espacoSpriteX,
            peY + 1,
            olhaDireita,
            WHITE
        );

        // ANIMACAO DO PERSONAGEM
        int frame =
            (int)(tempo * 8.0f) % FRAMES_VITORIA;

        if (l->vitoria.id != 0) {

            desenharFrame(
                l->vitoria,
                frame,
                espacoSpriteX,
                peY,
                olhaDireita,
                WHITE
            );
        }
        else {

            desenharFrame(
                l->parado,
                frame % FRAMES_PARADO,
                espacoSpriteX,
                peY,
                olhaDireita,
                WHITE
            );
        }

        EndScissorMode();

        // NOME DO PERSONAGEM
        textoContorno(
            l->nome,
            textoX,
            painel.y + 16,
            20,
            2,
            1,
            1,
            COR_SOMBRA,
            l->cor
        );

        // ELEMENTO
        DrawText(
            l->elemento,
            (int)textoX,
            (int)painel.y + 38,
            10,
            Fade(WHITE, 0.8f)
        );

        // TEXTO PRONTO
        float piscar =
            0.8f + 0.2f * sinf(tempo * 6.0f);

        textoContorno(
            "PRONTO!",
            textoX,
            painel.y + 52,
            10,
            2,
            1,
            0,
            COR_SOMBRA,
            Fade(COR_AMARELO, piscar)
        );

        // FLASH DA ESCOLHA
        if (flashEscolha[jogador] > 0.0f) {

            DrawRectangleRec(
                painel,
                Fade(
                    WHITE,
                    flashEscolha[jogador] * 2.0f
                )
            );
        }
    }
    else {

        // JOGADOR AINDA NAO ESCOLHEU
        float pulso =
            0.5f + 0.5f * sinf(tempo * 4.0f);

        textoCentralizado(
            "?",
            espacoSpriteX,
            painel.y + 20,
            40,
            0,
            1,
            COR_SOMBRA,
            Fade(
                corJogador,
                0.4f + 0.5f * pulso
            )
        );

        DrawText(
            "ESCOLHA",
            (int)textoX,
            (int)painel.y + 18,
            10,
            Fade(WHITE, 0.85f)
        );

        DrawText(
            "COM AS",
            (int)textoX,
            (int)painel.y + 29,
            10,
            Fade(WHITE, 0.6f)
        );

        DrawText(
            "TECLAS",
            (int)textoX,
            (int)painel.y + 40,
            10,
            Fade(WHITE, 0.6f)
        );

        textoContorno(
            jogador == 0 ? "1 2 3 4" : "Q W E R",
            textoX,
            painel.y + 52,
            10,
            1,
            1,
            0,
            COR_SOMBRA,
            corJogador
        );
    }

    DrawRectangleLinesEx(
        painel,
        1,
        Fade(corJogador, 0.9f)
    );
}

// =====================================
// TEXTO VS ENTRE OS JOGADORES
// =====================================
static void desenharVS(float tempo) {

    float cx = BASE_W / 2.0f;
    float cy = 128.0f;

    int quadro = (int)(tempo * 12.0f);

    // RAIOS AO REDOR DO VS
    for (int r = 0; r < 3; r++) {

        Vector2 anterior = { cx, cy };

        for (int s = 1; s <= 3; s++) {

            float angulo =
                (r * 120.0f + quadro * 37.0f)
                * DEG2RAD;

            Vector2 ponto = {
                cx + cosf(angulo) * s * 6.0f +
                    (float)(
                        ruido(quadro, r * 10 + s) % 5 - 2
                    ),

                cy + sinf(angulo) * s * 5.0f +
                    (float)(
                        ruido(quadro + 3, r * 10 + s) % 5 - 2
                    )
            };

            DrawLineV(
                anterior,
                ponto,
                Fade(COR_AMARELO, 0.8f)
            );

            anterior = ponto;
        }
    }

    float pulo =
        floorf(sinf(tempo * 5.0f) * 1.5f);

    textoCentralizado(
        "VS",
        cx,
        cy - 10 + pulo,
        20,
        2,
        2,
        COR_SOMBRA,
        (Color){ 255, 90, 60, 255 }
    );

    BeginScissorMode(
        (int)cx - 15,
        (int)(cy - 10 + pulo),
        30,
        8
    );

    textoCentralizado(
        "VS",
        cx,
        cy - 10 + pulo,
        20,
        2,
        0,
        BLANK,
        COR_AMARELO
    );

    EndScissorMode();
}

// =====================================
// TELA COMPLETA DE SELECAO
// =====================================
void desenharSelecaoPixel(
    int personagemJogador1,
    int personagemJogador2
) {

    double agora = GetTime();

    // REINICIA ANIMACAO AO ENTRAR NA TELA
    if (agora - ultimoDesenhoSelecao > 0.25) {

        tempoSelecao = 0.0f;

        escolhaAnterior[0] = personagemJogador1;
        escolhaAnterior[1] = personagemJogador2;

        flashEscolha[0] = 0.0f;
        flashEscolha[1] = 0.0f;
    }

    float dt = GetFrameTime();

    ultimoDesenhoSelecao = agora;
    tempoSelecao += dt;

    // DETECTA NOVA ESCOLHA
    int escolhas[2] = {
        personagemJogador1,
        personagemJogador2
    };

    for (int j = 0; j < 2; j++) {

        if (escolhas[j] != escolhaAnterior[j] &&
            escolhas[j] != 0) {

            flashEscolha[j] = 0.25f;
        }

        escolhaAnterior[j] = escolhas[j];

        if (flashEscolha[j] > 0.0f) {
            flashEscolha[j] -= dt;
        }
    }

    float tempo = (float)agora;

    comecarTelaPixel();

    // FUNDO
    desenharFundo(
        tempo,
        (Color){ 140, 50, 100, 255 }
    );

    // TITULO ANIMADO
    float queda =
        floorf(
            fminf(1.0f, tempoSelecao / 0.3f)
            * 30.0f
        ) - 30.0f;

    textoCentralizado(
        "ESCOLHA SEU PERSONAGEM",
        BASE_W / 2.0f,
        5 + queda,
        20,
        2,
        1,
        COR_SOMBRA,
        WHITE
    );

    BeginScissorMode(
        0,
        (int)(5 + queda),
        BASE_W,
        8
    );

    textoCentralizado(
        "ESCOLHA SEU PERSONAGEM",
        BASE_W / 2.0f,
        5 + queda,
        20,
        2,
        0,
        BLANK,
        COR_AMARELO
    );

    EndScissorMode();

    // PAINEIS DOS JOGADORES
    desenharPainelJogador(
        0,
        personagemJogador1,
        tempo
    );

    desenharPainelJogador(
        1,
        personagemJogador2,
        tempo
    );

    // VS
    desenharVS(tempo);

    // CARTAS DOS PERSONAGENS
    for (int i = 0; i < TOTAL_LUTADORES; i++) {

        desenharCarta(
            i,
            personagemJogador1,
            personagemJogador2,
            tempo
        );
    }

    // RODAPE
    if (personagemJogador1 != 0 &&
        personagemJogador2 != 0) {

        desenharRodape(
            "ESC VOLTAR",
            "ENTER - LUTAR!",
            COR_AMARELO,
            tempo
        );
    }
    else {

        desenharRodape(
            "ESC VOLTAR",
            "J1: 1 2 3 4    J2: Q W E R",
            WHITE,
            tempo
        );
    }

    terminarTelaPixel();
}

// =====================================
// TELA DE RANKING (PODIO)
// =====================================

// LE O ARQUIVO DE VITORIAS E ORDENA
// OS PERSONAGENS NO RANKING
static void carregarRanking(void) {

    int contagem[TOTAL_LUTADORES] = { 0, 0, 0, 0 };
    int personagem;

    FILE *arquivo = fopen("dados/ranking.txt", "r");

    if (arquivo != NULL) {

        while (fscanf(arquivo, "%d", &personagem) == 1) {

            if (personagem >= 1 &&
                personagem <= TOTAL_LUTADORES) {

                contagem[personagem - 1]++;
            }
        }

        fclose(arquivo);
    }

    totalVitorias = 0;

    for (int i = 0; i < TOTAL_LUTADORES; i++) {

        ordemRanking[i] = i;

        totalVitorias += contagem[i];
    }

    // ORDENA POR VITORIAS
    // UTILIZANDO BUBBLE SORT
    for (int i = 0; i < TOTAL_LUTADORES - 1; i++) {

        for (int j = 0; j < TOTAL_LUTADORES - 1 - i; j++) {

            if (contagem[ordemRanking[j]] <
                contagem[ordemRanking[j + 1]]) {

                int auxiliar = ordemRanking[j];

                ordemRanking[j] = ordemRanking[j + 1];

                ordemRanking[j + 1] = auxiliar;
            }
        }
    }

    for (int i = 0; i < TOTAL_LUTADORES; i++) {

        vitoriasRanking[i] = contagem[ordemRanking[i]];
    }
}

// =====================================
// DESENHAR TROFEU EM PIXEL ART
// =====================================
static void desenharTrofeu(int x, int y) {

    Color ouro = { 255, 206, 64, 255 };
    Color ouroEscuro = { 190, 130, 30, 255 };

    // PARTE SUPERIOR
    DrawRectangle(x, y, 9, 6, ouro);

    // BASE DO TROFEU
    DrawRectangle(x + 1, y + 6, 7, 1, ouro);
    DrawRectangle(x + 3, y + 7, 3, 2, ouroEscuro);
    DrawRectangle(x + 2, y + 9, 5, 2, ouro);

    // ALÇAS
    DrawPixel(x - 1, y + 1, ouro);
    DrawPixel(x - 1, y + 2, ouro);

    DrawPixel(x + 9, y + 1, ouro);
    DrawPixel(x + 9, y + 2, ouro);

    // BRILHO
    DrawPixel(x + 2, y + 1, WHITE);

    // SOMBRA
    DrawRectangle(x + 6, y + 1, 1, 4, ouroEscuro);
}

// =====================================
// COROA DO PRIMEIRO COLOCADO
// =====================================
static void desenharCoroa(int x, int y) {

    Color ouro = { 255, 214, 70, 255 };

    // BASE
    DrawRectangle(x - 5, y, 11, 3, ouro);

    // PONTAS DA COROA
    DrawPixel(x - 5, y - 1, ouro);
    DrawPixel(x - 5, y - 2, ouro);

    DrawPixel(x, y - 1, ouro);
    DrawPixel(x, y - 2, ouro);
    DrawPixel(x, y - 3, ouro);

    DrawPixel(x + 5, y - 1, ouro);
    DrawPixel(x + 5, y - 2, ouro);

    // PEDRAS PRECIOSAS
    DrawPixel(
        x,
        y + 1,
        (Color){ 230, 60, 70, 255 }
    );

    DrawPixel(
        x - 3,
        y + 1,
        (Color){ 80, 170, 255, 255 }
    );

    DrawPixel(
        x + 3,
        y + 1,
        (Color){ 80, 170, 255, 255 }
    );
}

// =====================================
// HOLOFOTES DO PODIO
// =====================================
static void desenharHolofotes(float tempo) {

    // HOLOFOTES QUE SE MOVIMENTAM
    for (int i = 0; i < 2; i++) {

        float origemX =
            (i == 0) ? 20.0f : 300.0f;

        float alvo =
            160.0f +
            sinf(tempo * 0.7f + i * 2.0f) * 50.0f;

        quadrilatero(
            (Vector2){ origemX - 3, 0 },
            (Vector2){ origemX + 3, 0 },
            (Vector2){ alvo + 22, (float)CHAO_TELAS },
            (Vector2){ alvo - 22, (float)CHAO_TELAS },
            Fade(
                (Color){ 255, 240, 200, 255 },
                0.07f
            )
        );
    }

    // HOLOFOTE FIXO NO PRIMEIRO LUGAR
    quadrilatero(
        (Vector2){ 154, 0 },
        (Vector2){ 166, 0 },
        (Vector2){ 196, (float)CHAO_TELAS },
        (Vector2){ 124, (float)CHAO_TELAS },
        Fade(
            (Color){ 255, 230, 150, 255 },
            0.09f
        )
    );
}

// =====================================
// CONFETES ANIMADOS
// =====================================
static void desenharConfete(float tempo) {

    for (int i = 0; i < 46; i++) {

        float velocidade =
            14.0f +
            (float)(ruido(i, 81) % 18);

        float y =
            fmodf(
                tempo * velocidade +
                (float)(ruido(i, 82) % BASE_H),
                (float)(BASE_H + 10)
            ) - 10.0f;

        float x =
            (float)(ruido(i, 83) % BASE_W) +
            sinf(tempo * 2.0f + i) * 6.0f;

        Color cor =
            (i % 5 == 4)
            ? WHITE
            : lutadores[i % TOTAL_LUTADORES].cor;

        int virado =
            ((int)(tempo * 8.0f + i) % 2) == 0;

        DrawRectangle(
            (int)x,
            (int)y,
            virado ? 2 : 1,
            virado ? 1 : 2,
            cor
        );
    }
}

// =====================================
// DEGRAUS DO PODIO
// =====================================
static void desenharDegrauPodio(
    float centroX,
    int largura,
    int altura,
    int posicao,
    float subida
) {

    Color frente = { 70, 40, 92, 255 };
    Color topo = { 120, 80, 150, 255 };
    Color lateral = { 50, 28, 70, 255 };

    Color medalhas[3] = {
        { 255, 206, 64, 255 },
        { 210, 214, 226, 255 },
        { 214, 140, 80, 255 }
    };

    int x = (int)(centroX - largura / 2.0f);

    int y =
        CHAO_TELAS - (int)(altura * subida);

    int h = (int)(altura * subida);

    if (h <= 0) {
        return;
    }

    // SOMBRA DO DEGRAU
    DrawRectangle(
        x + 2,
        y + 2,
        largura,
        h,
        Fade(BLACK, 0.4f)
    );

    // PARTE FRONTAL
    DrawRectangle(
        x,
        y,
        largura,
        h,
        frente
    );

    // PARTE SUPERIOR
    DrawRectangle(
        x,
        y,
        largura,
        3,
        topo
    );

    // LATERAL
    DrawRectangle(
        x + largura - 3,
        y + 3,
        3,
        h - 3,
        lateral
    );

    // LINHA DE DESTAQUE
    DrawRectangle(
        x,
        y + 3,
        largura,
        1,
        misturarCores(topo, frente, 0.5f)
    );

    // NUMERO DA POSICAO
    if (subida >= 1.0f) {

        const char *numero =
            TextFormat("%d", posicao);

        textoCentralizado(
            numero,
            centroX - 1,
            (float)y + 4,
            20,
            0,
            1,
            COR_SOMBRA,
            medalhas[posicao - 1]
        );
    }
}

// =====================================
// TELA COMPLETA DE RANKING
// =====================================
void desenharRankingPixel(void) {

    double agora = GetTime();

    // AO ENTRAR NA TELA:
    // RECARREGA O RANKING
    // E REINICIA A ANIMACAO
    if (agora - ultimoDesenhoRanking > 0.25) {

        carregarRanking();

        tempoRanking = 0.0f;
    }

    ultimoDesenhoRanking = agora;

    tempoRanking += GetFrameTime();

    float tempo = (float)agora;

    // POSICOES DOS PERSONAGENS
    // 1º NO CENTRO
    // 2º A ESQUERDA
    // 3º A DIREITA
    // 4º NO CHAO
    float centros[TOTAL_LUTADORES] = {
        160.0f,
        90.0f,
        230.0f,
        292.0f
    };

    int larguras[3] = {
        64,
        60,
        60
    };

    int alturas[3] = {
        40,
        28,
        20
    };

    int olhaDireita[TOTAL_LUTADORES] = {
        1,
        1,
        0,
        0
    };

    // INICIA A TELA
    comecarTelaPixel();

    // FUNDO
    desenharFundo(
        tempo,
        (Color){ 120, 60, 110, 255 }
    );

    // HOLOFOTES
    desenharHolofotes(tempo);

    // =====================================
    // TITULO DO RANKING
    // =====================================
    const char *titulo = "RANKING DE VITORIAS";

    float larguraTitulo =
        larguraTexto(titulo, 20, 2);

    textoCentralizado(
        titulo,
        BASE_W / 2.0f,
        6,
        20,
        2,
        1,
        COR_SOMBRA,
        (Color){ 255, 140, 40, 255 }
    );

    BeginScissorMode(
        0,
        6,
        BASE_W,
        9
    );

    textoCentralizado(
        titulo,
        BASE_W / 2.0f,
        6,
        20,
        2,
        0,
        BLANK,
        COR_AMARELO
    );

    EndScissorMode();

    // TROFEUS AO LADO DO TITULO
    desenharTrofeu(
        (int)(
            BASE_W / 2.0f -
            larguraTitulo / 2.0f
        ) - 16,
        7
    );

    desenharTrofeu(
        (int)(
            BASE_W / 2.0f +
            larguraTitulo / 2.0f
        ) + 7,
        7
    );

    // =====================================
    // MENSAGEM CASO NAO EXISTAM VITORIAS
    // =====================================
    if (totalVitorias == 0) {

        textoCentralizado(
            "NENHUMA LUTA AINDA - JOGUE PARA ENTRAR NO RANKING!",
            BASE_W / 2.0f,
            30,
            10,
            1,
            0,
            BLANK,
            Fade(WHITE, 0.7f)
        );
    }

    // =====================================
    // ANIMACAO DE SUBIDA DO PODIO
    // =====================================
    float subidas[3];

    // PRIMEIRO SOBE O TERCEIRO LUGAR
    subidas[2] = fminf(
        1.0f,
        fmaxf(
            0.0f,
            (tempoRanking - 0.00f) / 0.25f
        )
    );

    // DEPOIS O SEGUNDO
    subidas[1] = fminf(
        1.0f,
        fmaxf(
            0.0f,
            (tempoRanking - 0.15f) / 0.25f
        )
    );

    // POR ULTIMO O PRIMEIRO
    subidas[0] = fminf(
        1.0f,
        fmaxf(
            0.0f,
            (tempoRanking - 0.30f) / 0.30f
        )
    );

    // DESENHA OS DEGRAUS
    for (int p = 2; p >= 0; p--) {

        desenharDegrauPodio(
            centros[p],
            larguras[p],
            alturas[p],
            p + 1,
            subidas[p]
        );
    }

    // =====================================
    // DESENHAR OS LUTADORES DO RANKING
    // =====================================
    for (int p = TOTAL_LUTADORES - 1; p >= 0; p--) {

        LutadorTela *l =
            &lutadores[ordemRanking[p]];

        float peY =
            (p < 3)
            ? (float)(CHAO_TELAS - alturas[p])
            : (float)CHAO_TELAS;

        float aparecer =
            (p < 3)
            ? subidas[p]
            : fminf(
                1.0f,
                tempoRanking / 0.4f
            );

        // ESPERA O DEGRAU SUBIR
        if (aparecer < 1.0f) {
            continue;
        }

        // SOMBRA DO PERSONAGEM
        DrawEllipse(
            (int)centros[p],
            (int)peY + 1,
            11,
            2,
            Fade(BLACK, 0.4f)
        );

        // =====================================
        // PRIMEIRO COLOCADO
        // =====================================
        if (p == 0) {

            // AURA DO CAMPEAO
            desenharFrame(
                l->aura,
                (int)(tempo * 12.0f) % FRAMES_AURA,
                centros[p],
                peY + 1,
                1,
                WHITE
            );

            // ANIMACAO DE VITORIA
            int frame =
                (int)(tempo * 8.0f)
                % FRAMES_VITORIA;

            if (l->vitoria.id != 0) {

                desenharFrame(
                    l->vitoria,
                    frame,
                    centros[p],
                    peY,
                    olhaDireita[p],
                    WHITE
                );
            }
            else {

                desenharFrame(
                    l->parado,
                    frame % FRAMES_PARADO,
                    centros[p],
                    peY,
                    olhaDireita[p],
                    WHITE
                );
            }

            // COROA DO CAMPEAO
            int bob =
                (int)floorf(
                    sinf(tempo * 3.0f) * 1.5f
                );

            desenharCoroa(
                (int)centros[p],
                (int)peY - 56 + bob
            );
        }
        else {

            // OUTROS COLOCADOS
            Color tinta =
                (p == 3)
                ? (Color){ 150, 140, 160, 255 }
                : WHITE;

            int frame =
                (int)(tempo * 6.0f + p)
                % FRAMES_PARADO;

            desenharFrame(
                l->parado,
                frame,
                centros[p],
                peY,
                olhaDireita[p],
                tinta
            );
        }

        // =====================================
        // SILHUETA CASO O SPRITE NAO EXISTA
        // =====================================
        if (l->parado.id == 0) {

            DrawRectangle(
                (int)centros[p] - 4,
                (int)peY - 34,
                8,
                34,
                Fade(l->cor, 0.6f)
            );

            DrawCircle(
                (int)centros[p],
                (int)peY - 38,
                5,
                Fade(l->cor, 0.6f)
            );
        }
    }

    // =====================================
    // NOMES E QUANTIDADE DE VITORIAS
    // =====================================
    for (int p = 0; p < TOTAL_LUTADORES; p++) {

        if (tempoRanking < 0.55f) {
            break;
        }

        LutadorTela *l =
            &lutadores[ordemRanking[p]];

        const char *vitorias =
            (vitoriasRanking[p] == 1)
            ? "1 VITORIA"
            : TextFormat(
                "%d VITORIAS",
                vitoriasRanking[p]
            );

        Color corNome =
            (p == 3)
            ? Fade(l->cor, 0.6f)
            : l->cor;

        // NOME DO PERSONAGEM
        textoCentralizado(
            l->nome,
            centros[p],
            CHAO_TELAS + 1,
            10,
            2,
            1,
            COR_SOMBRA,
            corNome
        );

        // QUANTIDADE DE VITORIAS
        textoCentralizado(
            vitorias,
            centros[p],
            CHAO_TELAS + 10,
            10,
            0,
            1,
            COR_SOMBRA,
            Fade(
                WHITE,
                p == 3 ? 0.55f : 0.9f
            )
        );

        // MARCADOR DO QUARTO LUGAR
        if (p == 3) {

            DrawRectangle(
                (int)centros[p] - 5,
                CHAO_TELAS - 56,
                11,
                11,
                Fade(COR_SOMBRA, 0.85f)
            );

            DrawRectangleLines(
                (int)centros[p] - 5,
                CHAO_TELAS - 56,
                11,
                11,
                Fade(WHITE, 0.5f)
            );

            textoCentralizado(
                "4",
                centros[p] + 1,
                CHAO_TELAS - 55,
                10,
                0,
                0,
                BLANK,
                Fade(WHITE, 0.8f)
            );
        }
    }

    // =====================================
    // CONFETES
    // =====================================
    desenharConfete(tempo);

    // =====================================
    // RODAPE
    // =====================================
    desenharRodape(
        NULL,
        "ESC - VOLTAR",
        WHITE,
        tempo
    );

    // FINALIZA O DESENHO
    terminarTelaPixel();
}

// =====================================
// TELA DE FIM DE PARTIDA
// =====================================

// DESENHA O PERSONAGEM EM TAMANHO AMPLIADO
static void desenharFrameAmpliado(
    Texture2D textura,
    int frame,
    float centroX,
    float peY,
    int olhaDireita,
    float escala,
    Color tinta
) {

    if (textura.id == 0) {
        return;
    }

    float larguraOrigem =
        olhaDireita ? TAMANHO_FRAME : -TAMANHO_FRAME;

    float centro =
        (olhaDireita
            ? CENTRO_NO_FRAME
            : TAMANHO_FRAME - CENTRO_NO_FRAME)
        * escala;

    Rectangle origem = {
        (float)(frame * TAMANHO_FRAME),
        0,
        larguraOrigem,
        TAMANHO_FRAME
    };

    Rectangle destino = {
        floorf(centroX - centro),
        floorf(peY - PE_NO_FRAME * escala),
        TAMANHO_FRAME * escala,
        TAMANHO_FRAME * escala
    };

    DrawTexturePro(
        textura,
        origem,
        destino,
        (Vector2){ 0, 0 },
        0.0f,
        tinta
    );
}

// =====================================
// INDICADORES DOS ROUNDS VENCIDOS
// =====================================
static void desenharBolinhasRound(
    float centroX,
    float y,
    int vitorias,
    Color cor
) {

    // DUAS BOLINHAS:
    // O JOGADOR PRECISA VENCER 2 ROUNDS
    for (int i = 0; i < 2; i++) {

        int x = (int)centroX - 6 + i * 9;

        DrawCircle(
            x,
            (int)y,
            3,
            COR_SOMBRA
        );

        if (i < vitorias) {

            DrawCircle(
                x,
                (int)y,
                2,
                cor
            );

            DrawPixel(
                x - 1,
                (int)y - 1,
                WHITE
            );
        }
        else {

            DrawCircleLines(
                x,
                (int)y,
                2,
                Fade(cor, 0.5f)
            );
        }
    }
}

// =====================================
// TELA COMPLETA DE VITORIA
// =====================================
void desenharVitoriaPixel(
    int vencedorPartida,
    int vitoriasJogador1,
    int vitoriasJogador2,
    int personagemJogador1,
    int personagemJogador2
) {

    double agora = GetTime();

    // REINICIA A ANIMACAO AO ENTRAR NA TELA
    if (agora - ultimoDesenhoVitoria > 0.25) {

        tempoVitoria = 0.0f;
    }

    ultimoDesenhoVitoria = agora;
    tempoVitoria += GetFrameTime();

    float tempo = (float)agora;

    // =====================================
    // IDENTIFICAR VENCEDOR E PERDEDOR
    // =====================================
    int personagemVencedor =
        (vencedorPartida == 1)
        ? personagemJogador1
        : personagemJogador2;

    int personagemPerdedor =
        (vencedorPartida == 1)
        ? personagemJogador2
        : personagemJogador1;

    Color corJogador =
        (vencedorPartida == 1)
        ? COR_J1
        : COR_J2;

    LutadorTela *vencedor = NULL;
    LutadorTela *perdedor = NULL;

    // LOCALIZAR O PERSONAGEM VENCEDOR
    if (personagemVencedor >= 1 &&
        personagemVencedor <= TOTAL_LUTADORES) {

        vencedor = &lutadores[personagemVencedor - 1];
    }

    // LOCALIZAR O PERSONAGEM PERDEDOR
    if (personagemPerdedor >= 1 &&
        personagemPerdedor <= TOTAL_LUTADORES) {

        perdedor = &lutadores[personagemPerdedor - 1];
    }

    // =====================================
    // COR DO FUNDO BASEADA NO VENCEDOR
    // =====================================
    Color horizonte = {
        120, 60, 110, 255
    };

    if (vencedor != NULL) {

        horizonte = misturarCores(
            horizonte,
            vencedor->cor,
            0.35f
        );
    }

    // =====================================
    // INICIAR TELA
    // =====================================
    comecarTelaPixel();

    // FUNDO
    desenharFundo(tempo, horizonte);

    // HOLOFOTES
    desenharHolofotes(tempo);

    // =====================================
    // TITULO FIM DE PARTIDA
    // =====================================
    float queda =
        floorf(
            fminf(
                1.0f,
                tempoVitoria / 0.3f
            ) * 30.0f
        ) - 30.0f;

    textoCentralizado(
        "FIM DE PARTIDA!",
        BASE_W / 2.0f,
        5 + queda,
        20,
        2,
        1,
        COR_SOMBRA,
        (Color){ 255, 140, 40, 255 }
    );

    BeginScissorMode(
        0,
        (int)(5 + queda),
        BASE_W,
        9
    );

    textoCentralizado(
        "FIM DE PARTIDA!",
        BASE_W / 2.0f,
        5 + queda,
        20,
        2,
        0,
        BLANK,
        COR_AMARELO
    );

    EndScissorMode();

    // =====================================
    // MENSAGEM DO JOGADOR VENCEDOR
    // =====================================
    if (tempoVitoria > 0.3f) {

        const char *frase =
            (vencedorPartida == 1)
            ? "JOGADOR 1 VENCEU!"
            : "JOGADOR 2 VENCEU!";

        textoCentralizado(
            frase,
            BASE_W / 2.0f,
            27,
            10,
            2,
            1,
            COR_SOMBRA,
            corJogador
        );
    }

    // =====================================
    // PERSONAGEM DERROTADO
    // =====================================
    float xPerdedor = 266.0f;

    if (perdedor != NULL &&
        tempoVitoria > 0.3f) {

        // ULTIMO FRAME DA ANIMACAO DE DERROTA
        if (perdedor->derrotado.id != 0) {

            desenharFrame(
                perdedor->derrotado,
                FRAMES_DERROTA - 1,
                xPerdedor,
                CHAO_TELAS,
                0,
                (Color){ 150, 140, 160, 255 }
            );
        }
        else {

            // IMAGEM ALTERNATIVA CASO NAO EXISTA
            // O SPRITE DE DERROTA
            desenharFrame(
                perdedor->parado,
                0,
                xPerdedor,
                CHAO_TELAS,
                0,
                (Color){ 150, 140, 160, 255 }
            );
        }

        // =====================================
        // ESTRELINHAS GIRANDO NO PERDEDOR
        // =====================================
        for (int k = 0; k < 3; k++) {

            float a =
                tempo * 4.0f + k * 2.1f;

            int sx =
                (int)(
                    xPerdedor + 16 +
                    cosf(a) * 6.0f
                );

            int sy =
                (int)(
                    CHAO_TELAS - 15 +
                    sinf(a) * 2.0f
                );

            DrawPixel(
                sx,
                sy,
                COR_AMARELO
            );

            DrawPixel(
                sx - 1,
                sy,
                Fade(COR_AMARELO, 0.6f)
            );

            DrawPixel(
                sx + 1,
                sy,
                Fade(COR_AMARELO, 0.6f)
            );

            DrawPixel(
                sx,
                sy - 1,
                Fade(COR_AMARELO, 0.6f)
            );

            DrawPixel(
                sx,
                sy + 1,
                Fade(COR_AMARELO, 0.6f)
            );
        }

        // NOME DO PERSONAGEM DERROTADO
        textoCentralizado(
            perdedor->nome,
            xPerdedor,
            CHAO_TELAS + 2,
            10,
            2,
            1,
            COR_SOMBRA,
            Fade(perdedor->cor, 0.55f)
        );
    }

    // =====================================
    // PERSONAGEM VENCEDOR
    // =====================================
    float xVencedor = BASE_W / 2.0f;

    if (vencedor != NULL) {

        // ANIMACAO DE ENTRADA
        float entrada = fminf(
            1.0f,
            fmaxf(
                0.0f,
                (tempoVitoria - 0.2f) / 0.25f
            )
        );

        float subida =
            floorf(
                (1.0f - entrada) * 20.0f
            );

        // BRILHO NO CHAO
        DrawEllipse(
            (int)xVencedor,
            CHAO_TELAS + 1,
            26,
            4,
            Fade(vencedor->cor, 0.35f)
        );

        // =====================================
        // AURA AMPLIADA DO VENCEDOR
        // =====================================
        desenharFrameAmpliado(
            vencedor->aura,
            (int)(tempo * 12.0f) % FRAMES_AURA,
            xVencedor,
            CHAO_TELAS + 2 + subida,
            1,
            2.0f,
            WHITE
        );

        // =====================================
        // ANIMACAO DE VITORIA
        // =====================================
        int frame =
            (int)(tempo * 8.0f)
            % FRAMES_VITORIA;

        if (vencedor->vitoria.id != 0) {

            desenharFrameAmpliado(
                vencedor->vitoria,
                frame,
                xVencedor,
                CHAO_TELAS + subida,
                1,
                2.0f,
                WHITE
            );
        }
        else {

            desenharFrameAmpliado(
                vencedor->parado,
                frame % FRAMES_PARADO,
                xVencedor,
                CHAO_TELAS + subida,
                1,
                2.0f,
                WHITE
            );
        }

        // =====================================
        // NOME DO VENCEDOR
        // =====================================
        textoCentralizado(
            vencedor->nome,
            xVencedor,
            CHAO_TELAS + 2,
            10,
            2,
            1,
            COR_SOMBRA,
            vencedor->cor
        );

        // ELEMENTO DO VENCEDOR
        textoCentralizado(
            vencedor->elemento,
            xVencedor,
            CHAO_TELAS + 11,
            10,
            1,
            1,
            COR_SOMBRA,
            Fade(WHITE, 0.7f)
        );
    }

    // =====================================
    // PLACAR FINAL
    // =====================================
    if (tempoVitoria > 0.45f) {

        Rectangle painel = {
            10.0f,
            62.0f,
            92.0f,
            64.0f
        };

        // SOMBRA DO PAINEL
        DrawRectangle(
            (int)painel.x + 2,
            (int)painel.y + 2,
            (int)painel.width,
            (int)painel.height,
            Fade(BLACK, 0.5f)
        );

        // FUNDO DO PAINEL
        DrawRectangleRec(
            painel,
            Fade(COR_SOMBRA, 0.88f)
        );

        // CABECALHO
        DrawRectangle(
            (int)painel.x,
            (int)painel.y,
            (int)painel.width,
            11,
            COR_LARANJA
        );

        textoCentralizado(
            "PLACAR FINAL",
            painel.x + painel.width / 2.0f,
            painel.y + 1,
            10,
            1,
            0,
            BLANK,
            WHITE
        );

        float meio =
            painel.x + painel.width / 2.0f;

        // =====================================
        // PONTOS DO JOGADOR 1
        // =====================================
        textoCentralizado(
            TextFormat("%d", vitoriasJogador1),
            meio - 22,
            painel.y + 16,
            30,
            0,
            1,
            COR_SOMBRA,
            COR_J1
        );

        // X ENTRE OS PLACARES
        textoCentralizado(
            "X",
            meio,
            painel.y + 24,
            10,
            0,
            1,
            COR_SOMBRA,
            Fade(WHITE, 0.7f)
        );

        // =====================================
        // PONTOS DO JOGADOR 2
        // =====================================
        textoCentralizado(
            TextFormat("%d", vitoriasJogador2),
            meio + 22,
            painel.y + 16,
            30,
            0,
            1,
            COR_SOMBRA,
            COR_J2
        );

        // IDENTIFICACAO DOS JOGADORES
        DrawText(
            "J1",
            (int)(meio - 28),
            (int)painel.y + 46,
            10,
            COR_J1
        );

        DrawText(
            "J2",
            (int)(meio + 17),
            (int)painel.y + 46,
            10,
            COR_J2
        );

        // =====================================
        // INDICADORES DE ROUNDS
        // =====================================
        desenharBolinhasRound(
            meio - 22,
            painel.y + 59,
            vitoriasJogador1,
            COR_J1
        );

        desenharBolinhasRound(
            meio + 22,
            painel.y + 59,
            vitoriasJogador2,
            COR_J2
        );

        // BORDA DO PAINEL
        DrawRectangleLinesEx(
            painel,
            1,
            Fade(COR_LARANJA, 0.9f)
        );
    }

    // =====================================
    // CONFETES NA TELA
    // =====================================
    desenharConfete(tempo);

    // =====================================
    // RODAPE
    // =====================================
    desenharRodape(
        NULL,
        "ESC - VOLTAR AO MENU",
        WHITE,
        tempo
    );

    // =====================================
    // EFEITO DE CLARAO NA ENTRADA
    // =====================================
    if (tempoVitoria > 0.2f &&
        tempoVitoria < 0.5f) {

        float t =
            (tempoVitoria - 0.2f) / 0.3f;

        DrawRectangle(
            0,
            0,
            BASE_W,
            BASE_H,
            Fade(
                WHITE,
                0.5f * (1.0f - t)
            )
        );
    }

    // =====================================
    // FINALIZA A TELA
    // =====================================
    terminarTelaPixel();
}