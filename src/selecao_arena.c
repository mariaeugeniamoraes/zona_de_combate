#include "selecao_arena.h"
#include "cenarios.h"
#include "janela.h"
#include <math.h>
#include <stddef.h>

// =====================================
// A TELA E DESENHADA EM COORDENADAS DE 320x180
// E AMPLIADA 3x COM UMA CAMERA (mesmo estilo das outras telas)
// =====================================
#define BASE_W 320
#define BASE_H 180
#define ESCALA 3.0f

#define TOTAL_OPCOES   (TOTAL_ARENAS + 1)   // as arenas + ALEATORIA
#define OPCAO_ALEATORIA TOTAL_ARENAS

#define CARTA_W     96
#define PREVIA_H    54
#define FAIXA_NOME  12
#define CARTA_ESPACO 8
#define CARTA_Y     34

#define COR_SOMBRA  (Color){ 22, 12, 30, 255 }
#define COR_AMARELO (Color){ 255, 214, 80, 255 }
#define COR_LARANJA (Color){ 255, 140, 40, 255 }

static const char *DESCRICOES[TOTAL_OPCOES] = {
    "PARQUE ESQUECIDO AO POR DO SOL",
    "ARENA ANTIGA COM O PUBLICO GRITANDO",
    "O JOGO SORTEIA UMA DAS ARENAS"
};

static const char *NOMES_CURTOS[TOTAL_OPCOES] = {
    "PARQUE",
    "COLISEU",
    "ALEATORIA"
};

static const char *NOMES_LUTADORES[5] = { "???", "KAI", "LUNA", "NERO", "AYRA" };

static const Color CORES_LUTADORES[5] = {
    { 150, 150, 160, 255 },
    { 255, 225, 80, 255 },
    { 255, 110, 50, 255 },
    { 90, 180, 255, 255 },
    { 120, 230, 170, 255 }
};

// =====================================
// ESTADO
// =====================================
static int opcaoSelecionada = 0;
static double ultimaAtualizacao = -10.0;
static float tempoTela = 0.0f;
static float pulo[TOTAL_OPCOES] = { 0 };   // animacao da carta selecionada subindo

// =====================================
// AUXILIARES
// =====================================
static float xDaCarta(int i) {

    float larguraTotal = TOTAL_OPCOES * CARTA_W + (TOTAL_OPCOES - 1) * CARTA_ESPACO;

    return (BASE_W - larguraTotal) / 2.0f + i * (CARTA_W + CARTA_ESPACO);
}

static void textoCentralizado(const char *texto, float cx, float y, int tamanho, Color cor) {

    int largura = MeasureText(texto, tamanho);
    int x = (int)floorf(cx - largura / 2.0f);

    DrawText(texto, x + 1, (int)y + 1, tamanho, COR_SOMBRA);
    DrawText(texto, x, (int)y, tamanho, cor);
}

static int arenaDaPrevia(int opcao) {

    // A CARTA ALEATORIA FICA ALTERNANDO ENTRE AS ARENAS
    if (opcao == OPCAO_ALEATORIA) {
        return (int)(GetTime() / 0.5) % TOTAL_ARENAS;
    }

    return opcao;
}

static Camera2D cameraTela(void) {

    Camera2D camera = { 0 };

    camera.zoom = ESCALA;

    return camera;
}

// =====================================
// ATUALIZAR (TECLADO)
// =====================================
int atualizarSelecaoArena(void) {

    double agora = GetTime();
    float dt = GetFrameTime();

    // ACABOU DE ENTRAR NA TELA: REINICIA A ANIMACAO DE ENTRADA
    if (agora - ultimaAtualizacao > 0.25) {

        tempoTela = 0.0f;

        for (int i = 0; i < TOTAL_OPCOES; i++) {
            pulo[i] = 0.0f;
        }
    }

    ultimaAtualizacao = agora;
    tempoTela += dt;

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        opcaoSelecionada = (opcaoSelecionada + TOTAL_OPCOES - 1) % TOTAL_OPCOES;
    }

    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        opcaoSelecionada = (opcaoSelecionada + 1) % TOTAL_OPCOES;
    }

    if (IsKeyPressed(KEY_ONE))   opcaoSelecionada = 0;
    if (IsKeyPressed(KEY_TWO))   opcaoSelecionada = 1;
    if (IsKeyPressed(KEY_THREE)) opcaoSelecionada = 2;

    // ANIMACAO SUAVE DA CARTA SELECIONADA
    for (int i = 0; i < TOTAL_OPCOES; i++) {

        float alvo = (i == opcaoSelecionada) ? 1.0f : 0.0f;

        pulo[i] += (alvo - pulo[i]) * fminf(1.0f, dt * 14.0f);
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        return SELECAO_ARENA_VOLTAR;
    }

    // ENTER COMECA A LUTA (ALT+ENTER E TELA CHEIA, NAO CONTA)
    if (IsKeyPressed(KEY_ENTER) && !IsKeyDown(KEY_LEFT_ALT) && !IsKeyDown(KEY_RIGHT_ALT)) {

        if (opcaoSelecionada == OPCAO_ALEATORIA) {
            return GetRandomValue(0, TOTAL_ARENAS - 1);
        }

        return opcaoSelecionada;
    }

    return SELECAO_ARENA_NADA;
}

// =====================================
// FUNDO
// =====================================
static void desenharFundo(float tempo) {

    Color topo = { 12, 8, 30, 255 };
    Color base = { 60, 22, 70, 255 };

    for (int y = 0; y < BASE_H; y += 3) {

        float t = floorf((y / (float)BASE_H) * 6.0f) / 5.0f;

        Color cor = {
            (unsigned char)(topo.r + (base.r - topo.r) * t),
            (unsigned char)(topo.g + (base.g - topo.g) * t),
            (unsigned char)(topo.b + (base.b - topo.b) * t),
            255
        };

        DrawRectangle(0, y, BASE_W, 3, cor);
    }

    // ESTRELAS
    for (int i = 0; i < 35; i++) {

        int x = (i * 97 + 31) % BASE_W;
        int y = (i * 53 + 11) % 100;
        float brilho = 0.5f + 0.5f * sinf(tempo * (1.0f + (i % 4) * 0.5f) + i);

        if (brilho > 0.3f) {
            DrawRectangle(x, y, 1, 1, Fade(WHITE, brilho));
        }
    }

    // FAIXAS DIAGONAIS PASSANDO
    for (int i = 0; i < 6; i++) {

        float x = fmodf(i * 70.0f + tempo * 25.0f, 420.0f) - 80.0f;

        DrawTriangle(
            (Vector2){ x + 40, 0 }, (Vector2){ x, (float)BASE_H }, (Vector2){ x + 12, (float)BASE_H },
            Fade(WHITE, 0.035f)
        );
        DrawTriangle(
            (Vector2){ x + 40, 0 }, (Vector2){ x + 12, (float)BASE_H }, (Vector2){ x + 52, 0 },
            Fade(WHITE, 0.035f)
        );
    }
}

// =====================================
// CARTAS: MOLDURA POR BAIXO DA PREVIA
// =====================================
static void posicaoCarta(int i, float *x, float *y, float *aparecer) {

    // ENTRADA: AS CARTAS SOBEM UMA DEPOIS DA OUTRA
    float a = (tempoTela - i * 0.08f) / 0.25f;

    if (a < 0.0f) a = 0.0f;
    if (a > 1.0f) a = 1.0f;

    *aparecer = a;
    *x = xDaCarta(i);
    *y = CARTA_Y + (1.0f - a) * 40.0f - floorf(pulo[i] * 3.0f);
}

static void desenharFundoCarta(int i) {

    float x, y, aparecer;

    posicaoCarta(i, &x, &y, &aparecer);

    if (aparecer <= 0.0f) {
        return;
    }

    DrawRectangle((int)x + 2, (int)y + 2, CARTA_W, PREVIA_H + FAIXA_NOME, Fade(BLACK, 0.5f * aparecer));
    DrawRectangle((int)x - 1, (int)y - 1, CARTA_W + 2, PREVIA_H + FAIXA_NOME + 2, COR_SOMBRA);
}

static void desenharMolduraCarta(int i, float tempo) {

    float x, y, aparecer;

    posicaoCarta(i, &x, &y, &aparecer);

    if (aparecer <= 0.0f) {
        return;
    }

    int selecionada = (i == opcaoSelecionada);
    int yNome = (int)y + PREVIA_H;

    // CARTA ALEATORIA: "?" GRANDE POR CIMA DA PREVIA
    if (i == OPCAO_ALEATORIA) {

        DrawRectangle((int)x, (int)y, CARTA_W, PREVIA_H, Fade(COR_SOMBRA, 0.45f));

        float balanco = floorf(sinf(tempo * 4.0f) * 1.5f);

        textoCentralizado("?", x + CARTA_W / 2.0f, y + 9 + balanco, 40, selecionada ? COR_AMARELO : Fade(WHITE, 0.6f));
    }

    // FAIXA DO NOME
    const char *nome = NOMES_CURTOS[i];
    Color corNome = selecionada ? COR_AMARELO : Fade(WHITE, 0.6f);

    DrawRectangle((int)x, yNome, CARTA_W, FAIXA_NOME, COR_SOMBRA);
    DrawRectangle((int)x, yNome, CARTA_W, 1, selecionada ? COR_LARANJA : Fade(WHITE, 0.25f));
    textoCentralizado(nome, x + CARTA_W / 2.0f, (float)yNome + 2, 10, corNome);

    // BORDA
    Rectangle borda = { x - 1, y - 1, CARTA_W + 2.0f, PREVIA_H + FAIXA_NOME + 2.0f };

    if (selecionada) {

        float pulso = 0.6f + 0.4f * sinf(tempo * 8.0f);

        DrawRectangleLinesEx((Rectangle){ borda.x - 1, borda.y - 1, borda.width + 2, borda.height + 2 }, 2, Fade(COR_AMARELO, pulso));
    }
    else {
        DrawRectangleLinesEx(borda, 1, Fade((Color){ 200, 180, 210, 255 }, 0.5f));
    }

    // NUMERO DO ATALHO NO CANTO
    const char *atalho = TextFormat("%d", i + 1);

    DrawRectangle((int)x + 1, (int)y + 1, 8, 10, Fade(COR_SOMBRA, 0.8f));
    DrawText(atalho, (int)x + 3, (int)y + 1, 10, selecionada ? COR_AMARELO : Fade(WHITE, 0.6f));
}

// =====================================
// PREVIAS AO VIVO (desenhadas fora da camera, em 960x540)
// =====================================
static void desenharPrevias(void) {

    for (int i = 0; i < TOTAL_OPCOES; i++) {

        float x, y, aparecer;

        posicaoCarta(i, &x, &y, &aparecer);

        if (aparecer <= 0.0f) {
            continue;
        }

        int selecionada = (i == opcaoSelecionada);
        unsigned char luz = selecionada ? 255 : 120;

        desenharPreviaArena(
            (ArenaId)arenaDaPrevia(i),
            (Rectangle){ x * ESCALA, y * ESCALA, CARTA_W * ESCALA, PREVIA_H * ESCALA },
            (Color){ luz, luz, luz, 255 }
        );
    }
}

// =====================================
// PAINEL DE BAIXO
// =====================================
static void desenharPainel(int personagemJogador1, int personagemJogador2, float tempo) {

    float aparecer = fminf(1.0f, fmaxf(0.0f, (tempoTela - 0.3f) / 0.25f));

    if (aparecer <= 0.0f) {
        return;
    }

    Rectangle painel = { 16, 110, BASE_W - 32, 50 };

    DrawRectangle((int)painel.x + 2, (int)painel.y + 2, (int)painel.width, (int)painel.height, Fade(BLACK, 0.5f));
    DrawRectangleRec(painel, Fade(COR_SOMBRA, 0.9f));
    DrawRectangleLinesEx(painel, 1, Fade(COR_LARANJA, 0.8f));

    // NOME E DESCRICAO DA OPCAO SELECIONADA
    const char *nome = (opcaoSelecionada == OPCAO_ALEATORIA) ? "ARENA ALEATORIA" : nomeArena((ArenaId)opcaoSelecionada);

    textoCentralizado(nome, BASE_W / 2.0f, painel.y + 5, 20, COR_AMARELO);
    textoCentralizado(DESCRICOES[opcaoSelecionada], BASE_W / 2.0f, painel.y + 25, 10, Fade(WHITE, 0.85f));

    // QUEM VAI LUTAR: "KAI  VS  NERO"
    int p1 = (personagemJogador1 >= 1 && personagemJogador1 <= 4) ? personagemJogador1 : 0;
    int p2 = (personagemJogador2 >= 1 && personagemJogador2 <= 4) ? personagemJogador2 : 0;

    const char *nome1 = NOMES_LUTADORES[p1];
    const char *nome2 = NOMES_LUTADORES[p2];

    int l1 = MeasureText(nome1, 10);
    int lvs = MeasureText("VS", 10);
    int l2 = MeasureText(nome2, 10);
    int espaco = 8;
    int total = l1 + espaco + lvs + espaco + l2;
    int x = BASE_W / 2 - total / 2;
    int y = (int)painel.y + 37;
    int pulo = (int)floorf(sinf(tempo * 5.0f) * 1.0f);

    DrawText(nome1, x, y, 10, CORES_LUTADORES[p1]);
    DrawText("VS", x + l1 + espaco, y + pulo, 10, (Color){ 255, 90, 60, 255 });
    DrawText(nome2, x + l1 + espaco + lvs + espaco, y, 10, CORES_LUTADORES[p2]);
}

static void desenharRodape(float tempo) {

    DrawRectangle(0, BASE_H - 11, BASE_W, 11, Fade(BLACK, 0.6f));

    const char *esquerda = "< > ESCOLHER   BACKSPACE VOLTAR";
    const char *direita = "ENTER - LUTAR!";

    float piscar = 0.7f + 0.3f * sinf(tempo * 5.0f);

    DrawText(esquerda, 4, BASE_H - 10, 10, Fade(WHITE, 0.5f));
    DrawText(direita, BASE_W - 4 - MeasureText(direita, 10), BASE_H - 10, 10, Fade(COR_AMARELO, piscar));
}

// =====================================
// TELA COMPLETA
// A ORDEM IMPORTA: AS PREVIAS USAM RenderTexture,
// ENTAO SAO DESENHADAS FORA DA CAMERA
// =====================================
void desenharSelecaoArena(int personagemJogador1, int personagemJogador2) {

    float tempo = (float)GetTime();
    Camera2D camera = cameraTela();

    // 1. FUNDO, TITULO E BASE DAS CARTAS
    BeginMode2D(camera);

    desenharFundo(tempo);

    float queda = floorf(fminf(1.0f, tempoTela / 0.3f) * 30.0f) - 30.0f;

    textoCentralizado("ESCOLHA A ARENA", BASE_W / 2.0f, 6 + queda, 20, COR_LARANJA);

    for (int i = 0; i < TOTAL_OPCOES; i++) {
        desenharFundoCarta(i);
    }

    EndMode2D();

    // 2. PREVIAS AO VIVO DAS ARENAS
    desenharPrevias();

    // 3. MOLDURAS, TEXTOS, PAINEL E RODAPE
    BeginMode2D(camera);

    for (int i = 0; i < TOTAL_OPCOES; i++) {
        desenharMolduraCarta(i, tempo);
    }

    desenharPainel(personagemJogador1, personagemJogador2, tempo);
    desenharRodape(tempo);

    EndMode2D();
}