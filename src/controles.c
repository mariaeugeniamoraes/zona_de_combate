#include "controles.h"
#include "janela.h"
#include <math.h>
#include <stddef.h>

// =====================================
// TELA BASE EM PIXEL ART
// =====================================
#define BASE_W 320
#define BASE_H 180
#define CHAO_TELA 150

#define COR_SOMBRA  (Color){ 22, 12, 30, 255 }
#define COR_LARANJA (Color){ 255, 140, 40, 255 }
#define COR_AMARELO (Color){ 255, 214, 80, 255 }
#define COR_J1      (Color){ 80, 170, 255, 255 }
#define COR_J2      (Color){ 255, 80, 90, 255 }

// =====================================
// COLUNAS DA TABELA
// =====================================
#define COLUNA_J1_X     6
#define COLUNA_J1_W     62
#define COLUNA_ACAO_X   72
#define COLUNA_ACAO_W   176
#define COLUNA_J2_X     252
#define COLUNA_J2_W     62

#define TABELA_Y        26
#define CABECALHO_H     11
#define LINHA_H         13
#define TOTAL_ACOES     7

// =====================================
// TECLAS ESPECIAIS (desenhadas como setas ou com nome)
// =====================================
#define TECLA_ESQUERDA "<"
#define TECLA_DIREITA  ">"
#define TECLA_CIMA     "^"
#define TECLA_BAIXO    "v"

// =====================================
// ACOES DO JOGO
// (as teclas sao as mesmas usadas no main.c)
// =====================================
typedef struct {
    const char *nome;
    const char *paraQueServe;
    const char *teclasJogador1[2];
    const char *teclasJogador2[2];
    Color cor;
} AcaoControle;

static const AcaoControle ACOES[TOTAL_ACOES] = {
    { "MOVER",    "ANDA PARA OS LADOS",   { "A", "D" }, { TECLA_ESQUERDA, TECLA_DIREITA }, { 235, 235, 245, 255 } },
    { "PULAR",    "SALTA NO AR",          { "W", NULL }, { TECLA_CIMA, NULL },             { 235, 235, 245, 255 } },
    { "AGACHAR",  "FICA ABAIXADO",        { "C", NULL }, { "SHIFT", NULL },                { 235, 235, 245, 255 } },
    { "DEFENDER", "SO 1/5 DO DANO",  { "S", NULL }, { TECLA_BAIXO, NULL },            { 120, 200, 255, 255 } },
    { "SOCO",     "RAPIDO, 10 DE DANO",   { "F", NULL }, { "K", NULL },                    { 255, 190, 90, 255 } },
    { "CHUTE",    "15 DE DANO, + LONGE",  { "G", NULL }, { "L", NULL },                    { 255, 150, 70, 255 } },
    { "ESPECIAL", "PODER DO ELEMENTO",    { "E", NULL }, { "O", NULL },                    { 255, 214, 80, 255 } }
};

// =====================================
// ESTADO INTERNO
// =====================================
static RenderTexture2D telaPixel;
static int telaCarregada = 0;

static double ultimoDesenho = -10.0;
static float tempoTela = 0.0f;

// =====================================
// FUNCOES AUXILIARES
// =====================================
static int ruido(int a, int b) {

    unsigned int h = (unsigned int)a * 73856093u ^ (unsigned int)b * 19349663u;

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

    float giro = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

    if (giro < 0.0f) {
        DrawTriangle(a, b, c, cor);
    }
    else {
        DrawTriangle(a, c, b, cor);
    }
}

static void quadrilatero(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color cor) {

    triangulo(a, d, c, cor);
    triangulo(a, c, b, cor);
}

static float larguraTexto(const char *texto, float tamanho, float espaco) {

    return MeasureTextEx(GetFontDefault(), texto, tamanho, espaco).x;
}

// TEXTO COM CONTORNO (fonte padrao em tamanho 10 ou 20 fica pixel perfeito)
static void textoContorno(const char *texto, float x, float y, float tamanho, float espaco,
                          int contorno, Color corContorno, Color cor) {

    Font fonte = GetFontDefault();
    Vector2 pos = { floorf(x), floorf(y) };

    if (contorno > 0) {

        DrawTextEx(fonte, texto, (Vector2){ pos.x + 1, pos.y + 1 }, tamanho, espaco, Fade(BLACK, 0.6f));

        for (int dx = -contorno; dx <= contorno; dx++) {
            for (int dy = -contorno; dy <= contorno; dy++) {
                if (dx != 0 || dy != 0) {
                    DrawTextEx(fonte, texto, (Vector2){ pos.x + dx, pos.y + dy }, tamanho, espaco, corContorno);
                }
            }
        }
    }

    DrawTextEx(fonte, texto, pos, tamanho, espaco, cor);
}

static void textoCentralizado(const char *texto, float centroX, float y, float tamanho, float espaco,
                              int contorno, Color corContorno, Color cor) {

    textoContorno(texto, centroX - larguraTexto(texto, tamanho, espaco) / 2.0f, y, tamanho, espaco, contorno, corContorno, cor);
}

// =====================================
// FUNDO
// =====================================
static void desenharFundo(float tempo) {

    Color topo = { 12, 8, 30, 255 };
    Color meio = { 44, 18, 66, 255 };
    Color horizonte = { 110, 50, 110, 255 };

    for (int y = 0; y < CHAO_TELA; y += 3) {

        Color cor = (y < 80)
            ? misturarCores(topo, meio, degrau(y / 80.0f, 6))
            : misturarCores(meio, horizonte, degrau((y - 80) / (float)(CHAO_TELA - 80), 6));

        DrawRectangle(0, y, BASE_W, 3, cor);
    }

    for (int i = 0; i < 40; i++) {

        int x = ruido(i, 11) % BASE_W;
        int y = ruido(i, 12) % 120;
        float brilho = 0.5f + 0.5f * sinf(tempo * (1.0f + (i % 5) * 0.4f) + i);

        if (brilho > 0.25f) {
            DrawPixel(x, y, Fade(WHITE, brilho));
        }
    }

    // FAIXAS DIAGONAIS PASSANDO
    for (int i = 0; i < 6; i++) {

        float x = fmodf(i * 70.0f + tempo * 25.0f, 420.0f) - 80.0f;

        quadrilatero(
            (Vector2){ x + 40, 0 }, (Vector2){ x + 52, 0 },
            (Vector2){ x + 12, (float)CHAO_TELA }, (Vector2){ x, (float)CHAO_TELA },
            Fade(WHITE, 0.035f)
        );
    }

    // CHAO COM GRADE
    int alturaChao = BASE_H - CHAO_TELA;

    for (int y = CHAO_TELA; y < BASE_H; y += 3) {

        float t = degrau((y - CHAO_TELA) / (float)alturaChao, 4);
        DrawRectangle(0, y, BASE_W, 3, misturarCores((Color){ 46, 22, 60, 255 }, (Color){ 18, 10, 26, 255 }, t));
    }

    for (int i = -12; i <= 12; i++) {

        DrawLineV(
            (Vector2){ BASE_W / 2.0f + i * 8.0f, (float)CHAO_TELA },
            (Vector2){ BASE_W / 2.0f + i * 50.0f, (float)BASE_H },
            Fade(COR_LARANJA, 0.3f)
        );
    }

    DrawRectangle(0, CHAO_TELA - 1, BASE_W, 1, (Color){ 255, 170, 90, 255 });
}

// =====================================
// TECLA DESENHADA (com relevo, como um teclado)
// =====================================
static int larguraTecla(const char *tecla) {

    int largura = MeasureText(tecla, 10) + 6;

    return (largura < 11) ? 11 : largura;
}

static void desenharTecla(const char *tecla, int x, int y, Color corJogador, int apertada) {

    int largura = larguraTecla(tecla);
    int afundar = apertada ? 1 : 0;

    Color face = apertada ? misturarCores((Color){ 230, 228, 240, 255 }, corJogador, 0.5f) : (Color){ 230, 228, 240, 255 };
    Color lado = (Color){ 120, 112, 140, 255 };

    // SOMBRA E LATERAL DE BAIXO (relevo)
    DrawRectangle(x + 1, y + 2, largura, 11, Fade(BLACK, 0.5f));
    DrawRectangle(x, y + 1, largura, 11, lado);

    // FACE DA TECLA
    DrawRectangle(x, y + afundar, largura, 10, face);
    DrawRectangle(x, y + afundar, largura, 1, WHITE);
    DrawRectangleLines(x, y + afundar, largura, 10 + (1 - afundar), COR_SOMBRA);

    int cx = x + largura / 2;
    int cy = y + 5 + afundar;
    Color simbolo = COR_SOMBRA;

    // SETAS SAO DESENHADAS COMO TRIANGULOS
    if (tecla[0] == '<' && tecla[1] == '\0') {
        triangulo((Vector2){ (float)cx - 3, (float)cy }, (Vector2){ (float)cx + 2, (float)cy - 3 }, (Vector2){ (float)cx + 2, (float)cy + 3 }, simbolo);
    }
    else if (tecla[0] == '>' && tecla[1] == '\0') {
        triangulo((Vector2){ (float)cx + 3, (float)cy }, (Vector2){ (float)cx - 2, (float)cy - 3 }, (Vector2){ (float)cx - 2, (float)cy + 3 }, simbolo);
    }
    else if (tecla[0] == '^' && tecla[1] == '\0') {
        triangulo((Vector2){ (float)cx, (float)cy - 3 }, (Vector2){ (float)cx - 3, (float)cy + 2 }, (Vector2){ (float)cx + 3, (float)cy + 2 }, simbolo);
    }
    else if (tecla[0] == 'v' && tecla[1] == '\0') {
        triangulo((Vector2){ (float)cx, (float)cy + 3 }, (Vector2){ (float)cx - 3, (float)cy - 2 }, (Vector2){ (float)cx + 3, (float)cy - 2 }, simbolo);
    }
    else {
        DrawText(tecla, x + 3, y + 1 + afundar, 10, simbolo);
    }
}

// DESENHA AS TECLAS DE UMA ACAO CENTRALIZADAS NA COLUNA
static void desenharTeclasDaAcao(const char *const teclas[2], float centroX, int y, Color corJogador, int apertada) {

    int largura = larguraTecla(teclas[0]);

    if (teclas[1] != NULL) {
        largura += 3 + larguraTecla(teclas[1]);
    }

    int x = (int)(centroX - largura / 2.0f);

    desenharTecla(teclas[0], x, y, corJogador, apertada);

    if (teclas[1] != NULL) {
        desenharTecla(teclas[1], x + larguraTecla(teclas[0]) + 3, y, corJogador, apertada);
    }
}

// =====================================
// TABELA DE CONTROLES
// =====================================
static void desenharCabecalho(void) {

    int y = TABELA_Y;

    DrawRectangle(COLUNA_J1_X, y, COLUNA_J1_W, CABECALHO_H, COR_J1);
    DrawRectangle(COLUNA_ACAO_X, y, COLUNA_ACAO_W, CABECALHO_H, COR_LARANJA);
    DrawRectangle(COLUNA_J2_X, y, COLUNA_J2_W, CABECALHO_H, COR_J2);

    textoCentralizado("JOGADOR 1", COLUNA_J1_X + COLUNA_J1_W / 2.0f, (float)y + 1, 10, 1, 0, BLANK, WHITE);
    textoCentralizado("ACAO E PRA QUE SERVE", COLUNA_ACAO_X + COLUNA_ACAO_W / 2.0f, (float)y + 1, 10, 1, 0, BLANK, WHITE);
    textoCentralizado("JOGADOR 2", COLUNA_J2_X + COLUNA_J2_W / 2.0f, (float)y + 1, 10, 1, 0, BLANK, WHITE);
}

static void desenharLinha(int i, int destacada, float tempo) {

    const AcaoControle *acao = &ACOES[i];

    // ENTRADA: AS LINHAS APARECEM UMA DEPOIS DA OUTRA, VINDO DO LADO
    float aparecer = (tempoTela - 0.15f - i * 0.05f) / 0.2f;

    if (aparecer <= 0.0f) {
        return;
    }

    if (aparecer > 1.0f) {
        aparecer = 1.0f;
    }

    int deslize = (int)((1.0f - aparecer) * 20.0f);
    int y = TABELA_Y + CABECALHO_H + 2 + i * LINHA_H;

    // FUNDO DA LINHA (listrado, e mais claro na linha em destaque)
    Color fundo = (i % 2 == 0) ? Fade(COR_SOMBRA, 0.85f) : Fade((Color){ 34, 20, 46, 255 }, 0.85f);

    DrawRectangle(COLUNA_J1_X, y, COLUNA_J1_W, LINHA_H - 1, fundo);
    DrawRectangle(COLUNA_ACAO_X, y, COLUNA_ACAO_W, LINHA_H - 1, fundo);
    DrawRectangle(COLUNA_J2_X, y, COLUNA_J2_W, LINHA_H - 1, fundo);

    if (destacada) {

        float pulso = 0.5f + 0.5f * sinf(tempo * 6.0f);

        DrawRectangle(COLUNA_ACAO_X, y, COLUNA_ACAO_W, LINHA_H - 1, Fade(acao->cor, 0.10f + 0.08f * pulso));
        DrawRectangle(COLUNA_ACAO_X, y, 2, LINHA_H - 1, acao->cor);
    }

    // AS TECLAS "AFUNDAM" NA LINHA EM DESTAQUE, COMO SE ESTIVESSEM SENDO APERTADAS
    int apertada = destacada && (fmodf(tempo, 0.6f) < 0.3f);

    desenharTeclasDaAcao(acao->teclasJogador1, COLUNA_J1_X + COLUNA_J1_W / 2.0f - deslize, y + 1, COR_J1, apertada);
    desenharTeclasDaAcao(acao->teclasJogador2, COLUNA_J2_X + COLUNA_J2_W / 2.0f + deslize, y + 1, COR_J2, apertada);

    // NOME DA ACAO + PRA QUE SERVE
    float x = COLUNA_ACAO_X + 6.0f + deslize;
    Color corNome = Fade(acao->cor, aparecer);
    Color corTexto = Fade(WHITE, (destacada ? 0.95f : 0.7f) * aparecer);

    textoContorno(acao->nome, x, (float)y + 2, 10, 1, 0, BLANK, corNome);
    textoContorno(acao->paraQueServe, x + 62.0f, (float)y + 2, 10, 1, 0, BLANK, corTexto);
}

// =====================================
// TECLAS GERAIS E DICA DO ESPECIAL
// =====================================
static void desenharTeclasGerais(float tempo) {

    if (tempoTela < 0.6f) {
        return;
    }

    int y = 133;

    DrawRectangle(COLUNA_J1_X, y, BASE_W - 2 * COLUNA_J1_X, 34, Fade(COR_SOMBRA, 0.88f));
    DrawRectangleLines(COLUNA_J1_X, y, BASE_W - 2 * COLUNA_J1_X, 34, Fade(COR_LARANJA, 0.8f));

    // TRES TECLAS GERAIS LADO A LADO
    const char *teclas[3] = { "P", "ESC", "F11" };
    const char *textos[3] = { "PAUSAR", "MENU", "TELA CHEIA" };
    float centros[3] = { 56.0f, 152.0f, 250.0f };

    for (int i = 0; i < 3; i++) {

        int largura = larguraTecla(teclas[i]) + 4 + MeasureText(textos[i], 10);
        int x = (int)(centros[i] - largura / 2.0f);

        desenharTecla(teclas[i], x, y + 3, WHITE, 0);
        DrawText(textos[i], x + larguraTecla(teclas[i]) + 4, y + 4, 10, Fade(WHITE, 0.85f));
    }

    // DICA DO ESPECIAL COM A BARRINHA DE ENERGIA ENCHENDO
    float enche = fmodf(tempo * 0.4f, 1.0f);
    int barraX = 14;
    int barraY = y + 18;
    int barraW = 40;

    DrawRectangle(barraX, barraY, barraW, 6, (Color){ 40, 30, 60, 255 });
    DrawRectangle(barraX, barraY, (int)(barraW * enche), 6, enche >= 0.5f ? COR_AMARELO : (Color){ 80, 170, 255, 255 });
    DrawRectangle(barraX + barraW / 2, barraY - 1, 1, 8, WHITE);
    DrawRectangleLines(barraX, barraY, barraW, 6, Fade(WHITE, 0.6f));

    DrawText("ACERTE GOLPES PRA ENCHER A ENERGIA.", barraX + barraW + 5, barraY - 2, 10, Fade(WHITE, 0.85f));
    DrawText("O ESPECIAL GASTA METADE.", barraX + barraW + 5, barraY + 7, 10, Fade(COR_AMARELO, 0.9f));
}

// =====================================
// LIBERAR A TEXTURA
// =====================================
void descarregarControles(void) {

    if (telaCarregada) {

        UnloadRenderTexture(telaPixel);
        telaCarregada = 0;
    }
}

// =====================================
// DESENHAR A TELA
// =====================================
void desenharControlesPixel(void) {

    if (!telaCarregada) {

        telaPixel = LoadRenderTexture(BASE_W, BASE_H);
        SetTextureFilter(telaPixel.texture, TEXTURE_FILTER_POINT);
        telaCarregada = 1;
    }

    double agora = GetTime();

    // ACABOU DE ENTRAR NA TELA: REINICIA A ANIMACAO DE ENTRADA
    if (agora - ultimoDesenho > 0.25) {
        tempoTela = 0.0f;
    }

    ultimoDesenho = agora;
    tempoTela += GetFrameTime();

    float tempo = (float)agora;

    // A LINHA EM DESTAQUE MUDA A CADA 1,2 SEGUNDO
    int linhaDestaque = (int)(tempo / 1.2f) % TOTAL_ACOES;

    BeginTextureMode(telaPixel);

    ClearBackground(BLACK);

    desenharFundo(tempo);

    // TITULO
    float queda = floorf(fminf(1.0f, tempoTela / 0.3f) * 30.0f) - 30.0f;

    textoCentralizado("CONTROLES", BASE_W / 2.0f, 5 + queda, 20, 2, 1, COR_SOMBRA, COR_LARANJA);

    BeginScissorMode(0, (int)(5 + queda), BASE_W, 9);
    textoCentralizado("CONTROLES", BASE_W / 2.0f, 5 + queda, 20, 2, 0, BLANK, COR_AMARELO);
    EndScissorMode();

    // TABELA
    if (tempoTela > 0.1f) {
        desenharCabecalho();
    }

    for (int i = 0; i < TOTAL_ACOES; i++) {
        desenharLinha(i, i == linhaDestaque, tempo);
    }

    desenharTeclasGerais(tempo);

    // RODAPE
    float piscar = 0.7f + 0.3f * sinf(tempo * 5.0f);
    const char *rodape = "ESC - VOLTAR AO MENU";

    DrawRectangle(0, BASE_H - 11, BASE_W, 11, Fade(BLACK, 0.6f));
    DrawText(rodape, BASE_W / 2 - MeasureText(rodape, 10) / 2, BASE_H - 10, 10, Fade(WHITE, piscar));

    EndTextureMode();
    retomarQuadro();

    // AMPLIA PARA A TELA VIRTUAL (3x em 960x540)
    float escala = fminf(JANELA_LARGURA / (float)BASE_W, JANELA_ALTURA / (float)BASE_H);

    if (escala >= 1.0f) {
        escala = floorf(escala);
    }

    float x = (JANELA_LARGURA - BASE_W * escala) / 2.0f;
    float y = (JANELA_ALTURA - BASE_H * escala) / 2.0f;

    // BLEND PRE-MULTIPLICADO: evita que partes semitransparentes fiquem escuras na ampliacao
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
