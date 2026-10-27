#include "cenarios.h"
#include <math.h>

// =====================================
// TELA BASE EM PIXEL ART
// As arenas sao desenhadas numa tela pequena (320x180)
// e ampliadas 3x para a janela de 960x540.
// =====================================
#define BASE_W      320
#define BASE_H      180
#define ESCALA_BASE (LARGURA_TELA / BASE_W)          // 3
#define CHAO_BASE   (ALTURA_CHAO / ESCALA_BASE)      // 150
#define TILE_BASE   (TAMANHO_TILE / ESCALA_BASE)     // 10

// =====================================
// ESTADO INTERNO DO MODULO
// =====================================
static int mapa[LINHAS_MAPA][COLUNAS_MAPA];
static int mapaPronto = 0;
static ArenaId arenaAtual = ARENA_PARQUE;

static RenderTexture2D telaArena;
static int telaCarregada = 0;

static const char *NOMES_ARENAS[TOTAL_ARENAS] = {
    "PARQUE ABANDONADO",
    "COLISEU"
};

// =====================================
// FUNCOES AUXILIARES
// =====================================

// GERA UM NUMERO "ALEATORIO" FIXO A PARTIR DE DOIS VALORES
// (o mesmo par sempre gera o mesmo numero, assim o cenario
// nao fica mudando de um frame para o outro)
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

static Color clarear(Color cor, int quantidade) {

    int r = cor.r + quantidade;
    int g = cor.g + quantidade;
    int b = cor.b + quantidade;

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;

    return (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, cor.a };
}

// ARREDONDA UM DEGRADE EM "DEGRAUS" (cara de pixel art)
static float degrau(float t, int degraus) {

    if (t < 0.0f) t = 0.0f;
    if (t > 0.999f) t = 0.999f;

    return floorf(t * degraus) / (float)(degraus - 1);
}

// A RAYLIB SO DESENHA TRIANGULOS EM SENTIDO ANTI-HORARIO:
// ESTA FUNCAO CORRIGE A ORDEM DOS VERTICES SE PRECISAR
static void triangulo(Vector2 a, Vector2 b, Vector2 c, Color cor) {

    float giro = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

    if (giro < 0.0f) {
        DrawTriangle(a, b, c, cor);
    }
    else {
        DrawTriangle(a, c, b, cor);
    }
}

// QUADRILATERO: a = cima-esq, b = cima-dir, c = baixo-dir, d = baixo-esq
static void quadrilatero(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color cor) {

    triangulo(a, d, c, cor);
    triangulo(a, c, b, cor);
}

// CEU EM FAIXAS HORIZONTAIS PASSANDO POR VARIAS CORES
static void ceuEmFaixas(const Color *cores, const int *alturas, int quantidade, int espessura) {

    for (int y = 0; y < alturas[quantidade - 1]; y += espessura) {

        int i = 0;

        while (i < quantidade - 2 && y >= alturas[i + 1]) {
            i++;
        }

        float t = (float)(y - alturas[i]) / (float)(alturas[i + 1] - alturas[i]);

        DrawRectangle(0, y, BASE_W, espessura, misturarCores(cores[i], cores[i + 1], degrau(t, 6)));
    }
}

// =====================================
// MONTAR A MATRIZ DA ARENA
// =====================================
static void montarMapa(void) {

    for (int linha = 0; linha < LINHAS_MAPA; linha++) {

        for (int coluna = 0; coluna < COLUNAS_MAPA; coluna++) {

            if (linha == LINHA_CHAO) {
                mapa[linha][coluna] = TILE_SUPERFICIE;
            }
            else if (linha > LINHA_CHAO) {
                mapa[linha][coluna] = TILE_SOLO;
            }
            else if (coluna == 0 || coluna == COLUNAS_MAPA - 1) {
                mapa[linha][coluna] = TILE_LIMITE;
            }
            else {
                mapa[linha][coluna] = TILE_VAZIO;
            }
        }
    }

    mapaPronto = 1;
}

// =====================================
// CARREGAR UMA ARENA
// =====================================
void carregarArena(ArenaId arena) {

    if (arena < 0 || arena >= TOTAL_ARENAS) {
        arena = ARENA_PARQUE;
    }

    arenaAtual = arena;

    montarMapa();
}

ArenaId obterArenaAtual(void) {

    return arenaAtual;
}

const char *nomeArena(ArenaId arena) {

    if (arena < 0 || arena >= TOTAL_ARENAS) {
        return "ARENA DESCONHECIDA";
    }

    return NOMES_ARENAS[arena];
}

// =====================================
// CONSULTAR A MATRIZ
// =====================================
int tileNaPosicao(float x, float y) {

    if (!mapaPronto) {
        montarMapa();
    }

    // ACIMA DA TELA E AR LIVRE (permite pulos altos)
    if (y < 0.0f && x >= 0.0f && x < LARGURA_TELA) {
        return TILE_VAZIO;
    }

    // FORA DA TELA E LIMITE
    if (x < 0.0f || y < 0.0f || x >= LARGURA_TELA || y >= ALTURA_TELA) {
        return TILE_LIMITE;
    }

    int coluna = (int)(x / TAMANHO_TILE);
    int linha = (int)(y / TAMANHO_TILE);

    return mapa[linha][coluna];
}

int posicaoSolida(float x, float y) {

    return tileNaPosicao(x, y) != TILE_VAZIO;
}

// =====================================
// DESENHAR O CHAO A PARTIR DA MATRIZ
// (cada tile de 30x30 da tela vira 10x10 na tela base)
// =====================================
static void desenharChao(Color superficie, Color solo, Color detalhe) {

    for (int linha = 0; linha < LINHAS_MAPA; linha++) {

        for (int coluna = 0; coluna < COLUNAS_MAPA; coluna++) {

            int tile = mapa[linha][coluna];
            int x = coluna * TILE_BASE;
            int y = linha * TILE_BASE;

            if (tile == TILE_SUPERFICIE) {

                DrawRectangle(x, y, TILE_BASE, TILE_BASE, superficie);
                DrawRectangle(x, y, TILE_BASE, 1, clarear(superficie, 40));
                DrawRectangle(x, y + 1, TILE_BASE, 1, clarear(superficie, 18));
            }
            else if (tile == TILE_SOLO) {

                DrawRectangle(x, y, TILE_BASE, TILE_BASE, solo);
            }
            else {
                continue;
            }

            // DETALHES (pedrinhas, rachaduras)
            int n = ruido(linha, coluna);

            if (n % 3 == 0) {
                DrawPixel(x + n % TILE_BASE, y + 3 + (n / 7) % (TILE_BASE - 4), detalhe);
            }

            if (n % 5 == 0) {
                DrawRectangle(x + (n / 3) % (TILE_BASE - 2), y + 4 + (n / 11) % 5, 2, 1, detalhe);
            }
        }
    }
}

// ==========================================================
// ARENA 1 - COLISEU
// Interior de um estadio de pedra iluminado por luz dourada,
// com cupula de vidro, fileiras de arcos, publico,
// bandeiras roxas e o piso de areia com um mosaico circular.
// ==========================================================

// AS FILEIRAS DO COLISEU SAO CURVAS (mais altas nas pontas),
// PARA DAR A SENSACAO DE ESTAR DENTRO DE UM CIRCULO
static float curvaColiseu(float x, float yBase, float curvatura) {

    float u = (x - BASE_W / 2.0f) / (BASE_W / 2.0f);

    return yBase - curvatura * u * u;
}

static void faixaCurva(float yCima, float yBaixo, float curvatura, Color cor) {

    for (int x = 0; x < BASE_W; x++) {

        int topo = (int)curvaColiseu((float)x, yCima, curvatura);
        int base = (int)curvaColiseu((float)x, yBaixo, curvatura);

        DrawRectangle(x, topo, 1, base - topo, cor);
    }
}

static void desenharCupula(float tempo) {

    Color cores[4] = {
        { 52, 26, 86, 255 },
        { 140, 90, 170, 255 },
        { 236, 186, 210, 255 },
        { 255, 226, 170, 255 }
    };
    int alturas[4] = { 0, 14, 28, 44 };

    ceuEmFaixas(cores, alturas, 4, 2);

    // ESTRUTURA DA CUPULA: RAIOS SAINDO DO CENTRO + ANEIS
    Color ferro = { 110, 70, 80, 255 };
    Vector2 centro = { BASE_W / 2.0f, -70.0f };

    for (int i = -10; i <= 10; i++) {
        DrawLineV(centro, (Vector2){ BASE_W / 2.0f + i * 22.0f, 44.0f }, ferro);
    }

    for (int r = 85; r < 130; r += 12) {
        DrawEllipseLines((int)centro.x, (int)centro.y, r * 2.2f, (float)r, ferro);
    }

    // VIDROS BRILHANDO DE LEVE
    for (int i = 0; i < 12; i++) {

        int x = 20 + ruido(i, 3) % 280;
        int y = 4 + ruido(i, 4) % 26;
        float brilho = 0.5f + 0.5f * sinf(tempo * 1.2f + i);

        DrawRectangle(x, y, 2, 1, Fade(WHITE, 0.35f * brilho));
    }
}

static void desenharArcos(float yCima, float yBaixo, float curvatura, int largura, int espaco,
                          Color pedra, Color dentroCima, Color dentroBaixo, float tempo, int comLuz) {

    faixaCurva(yCima, yBaixo, curvatura, pedra);

    // FRISO CLARO EM CIMA E SOMBRA EMBAIXO
    faixaCurva(yCima, yCima + 1, curvatura, clarear(pedra, 30));
    faixaCurva(yBaixo - 2, yBaixo, curvatura, clarear(pedra, -45));

    int alturaArco = (int)(yBaixo - yCima) - 6;

    for (int x = 2; x < BASE_W; x += espaco) {

        float meio = x + largura / 2.0f;
        int topo = (int)curvaColiseu(meio, yCima + 3 + largura / 2.0f, curvatura);

        // INTERIOR DO ARCO EM DEGRADE (luz quente la dentro)
        for (int k = 0; k < alturaArco - largura / 2; k++) {

            float t = degrau(k / (float)alturaArco, 4);
            DrawRectangle(x, topo + k, largura, 1, misturarCores(dentroCima, dentroBaixo, t));
        }

        DrawCircle((int)meio, topo, largura / 2.0f, dentroCima);

        // COLUNA CLARA ENTRE OS ARCOS
        DrawRectangle(x + largura, topo - largura / 2, 1, alturaArco, clarear(pedra, 22));

        // ALGUMAS LAMPADAS PENDURADAS PISCANDO DE LEVE
        if (comLuz && ruido(x, 9) % 4 == 0) {

            float brilho = 0.75f + 0.25f * sinf(tempo * 3.0f + x);

            DrawCircle((int)meio, topo + 3, 2.5f, Fade((Color){ 255, 230, 150, 255 }, 0.3f * brilho));
            DrawPixel((int)meio, topo + 3, Fade((Color){ 255, 250, 220, 255 }, brilho));
        }
    }
}

static void desenharPublicoColiseu(float tempo) {

    Color roupas[6] = {
        { 190, 50, 60, 255 }, { 60, 90, 170, 255 }, { 230, 190, 70, 255 },
        { 70, 140, 80, 255 }, { 235, 230, 220, 255 }, { 130, 60, 140, 255 }
    };

    Color peles[3] = { { 240, 200, 160, 255 }, { 190, 130, 90, 255 }, { 120, 80, 55, 255 } };

    faixaCurva(113, 131, 4, (Color){ 168, 122, 80, 255 });

    for (int fileira = 0; fileira < 3; fileira++) {

        float yFileira = 114.0f + fileira * 6.0f;

        // DEGRAU DA ARQUIBANCADA
        faixaCurva(yFileira + 5, yFileira + 6, 4, (Color){ 140, 98, 62, 255 });

        for (int x = 1 + (fileira % 2) * 2; x < BASE_W; x += 4) {

            int n = ruido(fileira, x);

            if (n % 6 == 0) {
                continue;
            }

            int y = (int)curvaColiseu((float)x, yFileira, 4);

            // PARTE DO PUBLICO PULA
            int pulo = 0;

            if (n % 3 == 0 && sinf(tempo * 7.0f + (float)(n % 10)) > 0.3f) {
                pulo = 1;
            }

            DrawRectangle(x, y + 2 - pulo, 2, 3, roupas[n % 6]);
            DrawRectangle(x, y - pulo, 2, 2, peles[n % 3]);

            // ALGUNS LEVANTAM OS BRACOS
            if (n % 9 == 0 && pulo) {
                DrawPixel(x - 1, y - 1, peles[n % 3]);
                DrawPixel(x + 2, y - 1, peles[n % 3]);
            }
        }
    }
}

static void desenharBandeiraColiseu(float x, float yTopo, float comprimento, float tempo) {

    Color roxo = { 120, 36, 120, 255 };
    Color roxoClaro = { 170, 70, 160, 255 };
    Color roxoEscuro = { 80, 20, 82, 255 };
    Color dourado = { 240, 196, 80, 255 };

    float onda = sinf(tempo * 2.5f + x * 0.1f);

    // HASTE
    DrawRectangle((int)x - 2, (int)yTopo, 14, 2, (Color){ 110, 70, 40, 255 });

    // PANO (cada linha balanca um pouco mais que a de cima)
    for (int k = 0; k < (int)comprimento; k++) {

        float desloc = onda * (k / comprimento) * 2.0f;
        int xl = (int)(x + desloc);

        DrawRectangle(xl, (int)yTopo + 2 + k, 10, 1, roxo);
        DrawPixel(xl + 1, (int)yTopo + 2 + k, roxoClaro);
        DrawPixel(xl + 9, (int)yTopo + 2 + k, roxoEscuro);
        DrawPixel(xl + 2, (int)yTopo + 2 + k, dourado);
        DrawPixel(xl + 7, (int)yTopo + 2 + k, dourado);
    }

    // PONTA EM V
    float pontaX = x + onda * 2.0f;
    float pontaY = yTopo + 2 + comprimento;

    triangulo((Vector2){ pontaX, pontaY }, (Vector2){ pontaX + 10, pontaY }, (Vector2){ pontaX + 5, pontaY + 5 }, roxo);

    // EMBLEMA DOURADO
    int ex = (int)(x + onda * 0.8f) + 5;
    int ey = (int)(yTopo + 2 + comprimento * 0.4f);

    DrawCircle(ex, ey, 3, dourado);
    DrawCircle(ex, ey, 1.5f, roxoEscuro);
}

static void desenharPanoCaido(float x0, float x1, float y, float queda, Color cor, Color claro, Color franja) {

    // PANO PENDURADO ENTRE DOIS GANCHOS (formato de "sorriso")
    for (int x = (int)x0; x <= (int)x1; x++) {

        float t = (x - x0) / (x1 - x0);
        float caida = 4.0f * queda * t * (1.0f - t);

        DrawRectangle(x, (int)y, 1, (int)(caida + 4), cor);
        DrawPixel(x, (int)(y + caida + 1), claro);
        DrawPixel(x, (int)(y + caida + 4), franja);
    }

    DrawCircle((int)x0, (int)y, 2, franja);
    DrawCircle((int)x1, (int)y, 2, franja);
}

static void desenharTochaColiseu(int x, int y, float tempo) {

    float tremor = sinf(tempo * 17.0f + x) + sinf(tempo * 29.0f) * 0.6f;

    DrawCircle(x, y - 4, 9, Fade((Color){ 255, 170, 60, 255 }, 0.12f));
    DrawRectangle(x - 1, y, 3, 8, (Color){ 80, 50, 30, 255 });
    DrawRectangle(x - 2, y - 1, 5, 2, (Color){ 110, 70, 35, 255 });

    DrawCircle(x, y - 3, 2.5f + tremor * 0.4f, (Color){ 240, 90, 30, 255 });
    DrawCircle(x, y - 4, 1.6f, (Color){ 255, 190, 60, 255 });
    DrawPixel(x, y - 6 - (int)(tremor > 0.5f), (Color){ 255, 245, 200, 255 });
}

static void desenharRaiosDeLuz(float tempo) {

    // FEIXES DE LUZ DESCENDO DA CUPULA
    for (int i = 0; i < 4; i++) {

        float x = 70.0f + i * 62.0f;
        float alfa = 0.06f + 0.03f * sinf(tempo * 0.8f + i * 1.7f);

        quadrilatero(
            (Vector2){ x, 30 }, (Vector2){ x + 14, 30 },
            (Vector2){ x + 44, (float)CHAO_BASE }, (Vector2){ x + 18, (float)CHAO_BASE },
            Fade((Color){ 255, 236, 180, 255 }, alfa)
        );
    }
}

static void desenharPoeiraColiseu(float tempo) {

    for (int i = 0; i < 28; i++) {

        float x = fmodf((float)(ruido(i, 51) % BASE_W) + tempo * (2.0f + i % 4), (float)BASE_W);
        float y = 40.0f + (float)(ruido(i, 52) % 110) + sinf(tempo * 0.7f + i) * 4.0f;
        float brilho = 0.4f + 0.4f * sinf(tempo * 2.0f + i);

        DrawPixel((int)x, (int)y, Fade((Color){ 255, 240, 200, 255 }, brilho));
    }
}

static void desenharMosaicoColiseu(void) {

    Color linha = { 176, 128, 74, 255 };
    Color centro = { 236, 200, 136, 255 };
    int cx = BASE_W / 2;
    int cy = 166;

    // POCA DE LUZ NO CENTRO
    DrawEllipse(cx, cy, 120, 11, Fade((Color){ 255, 230, 160, 255 }, 0.25f));

    // ANEIS DO MOSAICO
    DrawEllipseLines(cx, cy, 140, 13, linha);
    DrawEllipseLines(cx, cy, 104, 10, linha);
    DrawEllipseLines(cx, cy, 64, 6, linha);
    DrawEllipse(cx, cy, 26, 3, centro);
    DrawEllipseLines(cx, cy, 26, 3, linha);

    // RAIOS DO MOSAICO
    for (int i = 0; i < 16; i++) {

        float a = i * (2.0f * PI / 16.0f);

        DrawLineV(
            (Vector2){ cx + cosf(a) * 26, cy + sinf(a) * 3 },
            (Vector2){ cx + cosf(a) * 104, cy + sinf(a) * 10 },
            Fade(linha, 0.7f)
        );
    }
}

static void desenharColiseu(float tempo) {

    Color pedra = { 226, 182, 118, 255 };
    Color pedraMedia = { 206, 160, 100, 255 };
    Color pedraEscura = { 184, 138, 86, 255 };

    desenharCupula(tempo);

    // BORDA DE PEDRA ONDE A CUPULA SE APOIA
    faixaCurva(38, 46, 14, (Color){ 150, 104, 66, 255 });
    faixaCurva(38, 39, 14, (Color){ 200, 150, 96, 255 });

    // TRES ANDARES DE ARCOS (do mais alto e distante para o mais baixo)
    desenharArcos(46, 64, 12, 4, 8, pedraEscura, (Color){ 150, 96, 64, 255 }, (Color){ 110, 66, 46, 255 }, tempo, 0);
    desenharArcos(64, 88, 9, 6, 11, pedraMedia, (Color){ 255, 196, 116, 255 }, (Color){ 196, 118, 66, 255 }, tempo, 1);
    desenharArcos(88, 113, 6, 8, 14, pedra, (Color){ 255, 224, 150, 255 }, (Color){ 230, 150, 80, 255 }, tempo, 1);

    // BANDEIRAS PENDURADAS NOS ARCOS
    for (int i = 0; i < 5; i++) {

        float x = 26.0f + i * 64.0f;
        float y = curvaColiseu(x + 5, 62, 12);

        desenharBandeiraColiseu(x, y, 26.0f, tempo);
    }

    // ARQUIBANCADA COM PUBLICO
    desenharPublicoColiseu(tempo);

    // PARAPEITO
    faixaCurva(130, 134, 3, (Color){ 150, 104, 66, 255 });
    faixaCurva(130, 131, 3, (Color){ 214, 168, 108, 255 });

    // MURO DO PODIO COM BLOCOS DE PEDRA
    DrawRectangle(0, 134, BASE_W, CHAO_BASE - 134, (Color){ 196, 150, 96, 255 });

    for (int linha = 0; linha < 3; linha++) {

        int y = 134 + linha * 6;
        int deslocamento = (linha % 2) * 6;

        DrawRectangle(0, y, BASE_W, 1, (Color){ 160, 116, 72, 255 });

        for (int x = -deslocamento; x < BASE_W; x += 12) {
            DrawRectangle(x, y, 1, 6, (Color){ 160, 116, 72, 255 });
        }
    }

    // PANOS ROXOS ENTRE OS GANCHOS DO MURO
    for (int i = 0; i < 5; i++) {

        float x0 = 4.0f + i * 64.0f;

        desenharPanoCaido(x0, x0 + 56, 135, 5,
                          (Color){ 120, 36, 120, 255 }, (Color){ 170, 70, 160, 255 }, (Color){ 240, 196, 80, 255 });
    }

    // TOCHAS NO MURO
    for (int i = 0; i < 4; i++) {
        desenharTochaColiseu(64 + i * 64, 138, tempo);
    }

    // PISO DE AREIA (MATRIZ) + MOSAICO CIRCULAR
    desenharChao(pedra, (Color){ 204, 158, 98, 255 }, (Color){ 176, 128, 74, 255 });
    desenharMosaicoColiseu();

    // PANOS ROXOS CAIDOS NOS CANTOS (como na referencia)
    triangulo((Vector2){ 0, 128 }, (Vector2){ 0, 176 }, (Vector2){ 26, 176 }, (Color){ 110, 30, 110, 255 });
    triangulo((Vector2){ 0, 134 }, (Vector2){ 0, 176 }, (Vector2){ 14, 176 }, (Color){ 160, 60, 150, 255 });
    triangulo((Vector2){ 320, 128 }, (Vector2){ 320, 176 }, (Vector2){ 294, 176 }, (Color){ 110, 30, 110, 255 });
    triangulo((Vector2){ 320, 134 }, (Vector2){ 320, 176 }, (Vector2){ 306, 176 }, (Color){ 160, 60, 150, 255 });

    // LUZ E POEIRA NO AR
    desenharRaiosDeLuz(tempo);
    desenharPoeiraColiseu(tempo);
}

// ==========================================================
// ARENA 2 - PARQUE DE DIVERSOES ABANDONADO
// Ceu vermelho de fim de tarde, roda-gigante parada,
// montanha-russa com looping, carrossel com luzes falhando,
// onibus abandonado, arvores secas, pocas e neblina.
// ==========================================================
static void desenharCeuParque(float tempo) {

    Color cores[5] = {
        { 22, 8, 22, 255 },
        { 80, 18, 42, 255 },
        { 170, 44, 66, 255 },
        { 236, 108, 104, 255 },
        { 250, 160, 130, 255 }
    };
    int alturas[5] = { 0, 34, 70, 104, 124 };

    ceuEmFaixas(cores, alturas, 5, 2);

    // BRILHO ATRAS DA RODA-GIGANTE
    DrawCircle(176, 92, 70, Fade((Color){ 255, 150, 140, 255 }, 0.12f));
    DrawCircle(176, 92, 42, Fade((Color){ 255, 180, 160, 255 }, 0.15f));

    // NUVENS ESCURAS EM FAIXAS SE MOVENDO DEVAGAR
    for (int i = 0; i < 9; i++) {

        float velocidade = 1.5f + (i % 3);
        float largura = 40.0f + (float)(ruido(i, 61) % 50);
        float x = fmodf((float)(ruido(i, 62) % 420) + tempo * velocidade, 420.0f) - 60.0f;
        float y = 8.0f + (float)(ruido(i, 63) % 70);
        int altura = 3 + ruido(i, 64) % 4;

        Color escura = (y < 40) ? (Color){ 40, 12, 30, 255 } : (Color){ 110, 30, 52, 255 };

        DrawEllipse((int)x, (int)y, largura / 2.0f, (float)altura, Fade(escura, 0.85f));
        DrawEllipse((int)x + 4, (int)y + altura - 1, largura / 2.0f - 6.0f, 1.5f, Fade((Color){ 240, 120, 120, 255 }, 0.5f));
    }

    // CLARAO DE RAIO DISTANTE DE VEZ EM QUANDO (clima misterioso)
    float ciclo = fmodf(tempo, 9.0f);

    if ((ciclo > 6.0f && ciclo < 6.08f) || (ciclo > 6.16f && ciclo < 6.22f)) {
        DrawRectangle(0, 0, BASE_W, 124, Fade((Color){ 255, 210, 220, 255 }, 0.18f));
    }
}

static void desenharFundoParque(float tempo) {

    Color longe = { 96, 30, 52, 255 };

    // MORROS E ARVORES DISTANTES
    for (int i = 0; i < 26; i++) {

        int cx = i * 13 - 4;
        int raio = 5 + ruido(i, 71) % 8;

        DrawCircle(cx, 124, (float)raio, longe);
    }

    DrawRectangle(0, 122, BASE_W, CHAO_BASE - 122, longe);

    // TORRE DE QUEDA LIVRE AO FUNDO
    Color torre = { 70, 22, 40, 255 };

    DrawRectangle(124, 26, 1, 100, torre);
    DrawRectangle(130, 26, 1, 100, torre);

    for (int y = 30; y < 124; y += 8) {
        DrawLine(124, y, 130, y + 8, torre);
        DrawLine(130, y, 124, y + 8, torre);
    }

    DrawRectangle(121, 22, 13, 5, torre);
    DrawRectangle(123, 18, 9, 4, torre);

    // LUZ VERMELHA PISCANDO NO TOPO
    if (fmodf(tempo, 1.6f) < 0.8f) {
        DrawCircle(127, 16, 3, Fade((Color){ 255, 60, 60, 255 }, 0.3f));
        DrawRectangle(126, 15, 2, 2, (Color){ 255, 80, 80, 255 });
    }
}

static void desenharMontanhaRussaParque(void) {

    Color trilho = { 34, 12, 26, 255 };
    Color borda = { 140, 50, 70, 255 };

    Vector2 centro = { 284.0f, 70.0f };
    float raio = 26.0f;

    // PILARES
    for (int x = 214; x < BASE_W; x += 9) {

        float y;

        if (x < 270) {
            float t = (x - 214) / 56.0f;
            y = 120.0f - t * 24.0f - sinf(t * PI) * 14.0f;
        }
        else {
            y = 98.0f;
        }

        DrawLine(x, (int)y, x, CHAO_BASE, trilho);

        if (x % 2 == 0) {
            DrawLine(x, (int)y + 6, x + 9, CHAO_BASE - 4, Fade(trilho, 0.7f));
        }
    }

    DrawLine((int)centro.x - 14, 98, (int)centro.x - 20, CHAO_BASE, trilho);
    DrawLine((int)centro.x + 14, 98, (int)centro.x + 20, CHAO_BASE, trilho);

    // SUBIDA ATE O LOOPING
    Vector2 anterior = { 214.0f, 120.0f };

    for (int x = 216; x <= 270; x += 2) {

        float t = (x - 214) / 56.0f;
        Vector2 ponto = { (float)x, 120.0f - t * 24.0f - sinf(t * PI) * 14.0f };

        DrawLineEx(anterior, ponto, 2, trilho);
        DrawPixel((int)ponto.x, (int)ponto.y - 1, borda);

        anterior = ponto;
    }

    DrawLineEx(anterior, (Vector2){ centro.x, centro.y + raio }, 2, trilho);
    DrawLineEx((Vector2){ centro.x, centro.y + raio }, (Vector2){ (float)BASE_W, 104.0f }, 2, trilho);

    // LOOPING
    DrawRing(centro, raio - 2.0f, raio, 0.0f, 360.0f, 48, trilho);
    DrawRing(centro, raio - 6.0f, raio - 5.0f, 0.0f, 360.0f, 48, trilho);

    for (int i = 0; i < 16; i++) {

        float a = i * (2.0f * PI / 16.0f);

        DrawLineV(
            (Vector2){ centro.x + cosf(a) * (raio - 5), centro.y + sinf(a) * (raio - 5) },
            (Vector2){ centro.x + cosf(a) * (raio - 1), centro.y + sinf(a) * (raio - 1) },
            trilho
        );
    }

    // BORDA ILUMINADA PELO CEU
    DrawRing(centro, raio, raio + 1.0f, 200.0f, 340.0f, 24, borda);
}

static void desenharRodaGiganteParque(float tempo) {

    Vector2 centro = { 176.0f, 66.0f };
    float raio = 46.0f;
    float giro = 0.15f;                       // PARADA: ESTA ABANDONADA
    Color metal = { 46, 16, 32, 255 };
    Color metalClaro = { 120, 44, 64, 255 };

    // PERNAS EM "A"
    DrawLineEx(centro, (Vector2){ centro.x - 32, (float)CHAO_BASE }, 3, metal);
    DrawLineEx(centro, (Vector2){ centro.x + 32, (float)CHAO_BASE }, 3, metal);
    DrawLine((int)centro.x - 22, 116, (int)centro.x + 22, 116, metal);

    // RAIOS E ARCOS
    for (int i = 0; i < 16; i++) {

        float a = giro + i * (2.0f * PI / 16.0f);

        DrawLineV(centro, (Vector2){ centro.x + cosf(a) * raio, centro.y + sinf(a) * raio }, metal);
    }

    DrawRing(centro, raio - 2.0f, raio, 0.0f, 360.0f, 64, metal);
    DrawRing(centro, raio * 0.62f - 1.0f, raio * 0.62f, 0.0f, 360.0f, 48, metal);
    DrawRing(centro, raio, raio + 1.0f, 190.0f, 350.0f, 32, metalClaro);

    // LAMPADAS DO ARCO: A MAIORIA QUEIMADA, ALGUMAS FALHANDO
    int instante = (int)(tempo * 9.0f);

    for (int i = 0; i < 32; i++) {

        float a = giro + i * (2.0f * PI / 32.0f);
        int x = (int)(centro.x + cosf(a) * (raio - 1));
        int y = (int)(centro.y + sinf(a) * (raio - 1));
        int funciona = (ruido(i, 81) % 3 == 0);
        int acesa = funciona && (ruido(i, instante) % 10 < 8);

        if (acesa) {
            DrawCircle(x, y, 2.5f, Fade((Color){ 255, 200, 200, 255 }, 0.3f));
            DrawPixel(x, y, (Color){ 255, 236, 220, 255 });
        }
        else {
            DrawPixel(x, y, (Color){ 90, 40, 50, 255 });
        }
    }

    // GONDOLAS BALANCANDO DE LEVE COM O VENTO
    for (int i = 0; i < 12; i++) {

        float a = giro + i * (2.0f * PI / 12.0f);
        Vector2 ponta = { centro.x + cosf(a) * raio, centro.y + sinf(a) * raio };
        float balanco = sinf(tempo * 1.3f + i) * 1.2f;

        DrawLineV(ponta, (Vector2){ ponta.x + balanco, ponta.y + 4 }, metal);

        int gx = (int)(ponta.x + balanco) - 4;
        int gy = (int)ponta.y + 4;

        DrawRectangle(gx, gy, 9, 6, (Color){ 30, 10, 22, 255 });
        DrawRectangle(gx + 1, gy - 1, 7, 1, (Color){ 30, 10, 22, 255 });
        DrawRectangle(gx + 1, gy + 1, 7, 2, (Color){ 80, 30, 46, 255 });
    }

    DrawCircleV(centro, 4.0f, metal);
}

static void desenharCavalinho(int x, int y, Color cor) {

    DrawRectangle(x - 4, y, 8, 3, cor);              // corpo
    DrawRectangle(x + 3, y - 3, 2, 4, cor);          // pescoco
    DrawRectangle(x + 4, y - 4, 3, 2, cor);          // cabeca
    DrawRectangle(x - 4, y + 3, 1, 3, cor);          // pernas
    DrawRectangle(x - 2, y + 3, 1, 2, cor);
    DrawRectangle(x + 2, y + 3, 1, 3, cor);
    DrawPixel(x + 4, y + 3, cor);
    DrawPixel(x - 5, y, cor);                        // rabo
    DrawPixel(x - 6, y + 1, cor);
}

static void desenharCarrossel(float tempo) {

    int cx = 46;
    Color telhado = { 70, 20, 30, 255 };
    Color listraA = { 130, 34, 40, 255 };
    Color listraB = { 196, 160, 130, 255 };
    Color poste = { 170, 130, 70, 255 };
    Color sombra = { 34, 12, 22, 255 };

    // LUZ QUENTE DEBAIXO DO TELHADO
    DrawEllipse(cx, 100, 54, 40, Fade((Color){ 255, 160, 90, 255 }, 0.10f));

    // BASE
    DrawEllipse(cx, 140, 50, 6, sombra);
    DrawRectangle(cx - 50, 134, 100, 6, (Color){ 60, 22, 30, 255 });
    DrawRectangle(cx - 50, 134, 100, 1, (Color){ 150, 80, 70, 255 });

    // MASTRO CENTRAL
    DrawRectangle(cx - 2, 60, 5, 74, (Color){ 120, 70, 50, 255 });
    DrawRectangle(cx - 1, 60, 1, 74, (Color){ 190, 140, 90, 255 });

    // POSTES COM CAVALINHOS SUBINDO E DESCENDO DEVAGAR
    for (int i = 0; i < 6; i++) {

        int x = cx - 40 + i * 16;
        int altura = (int)(sinf(tempo * 1.2f + i * 1.3f) * 3.0f);

        DrawRectangle(x, 68, 1, 66, poste);
        desenharCavalinho(x, 112 + altura, (i % 2) ? (Color){ 210, 190, 170, 255 } : (Color){ 150, 90, 70, 255 });
    }

    // TELHADO EM CONE
    triangulo((Vector2){ (float)cx, 36 }, (Vector2){ (float)cx - 54, 62 }, (Vector2){ (float)cx + 54, 62 }, telhado);

    for (int i = -3; i <= 3; i++) {
        DrawLine(cx, 36, cx + i * 16, 62, listraA);
    }

    DrawRectangle(cx - 1, 30, 3, 7, poste);
    DrawCircle(cx, 29, 2, poste);

    // BABADO LISTRADO EM SEMICIRCULOS
    for (int i = 0; i < 12; i++) {

        int x = cx - 54 + i * 9;
        Color cor = (i % 2) ? listraA : listraB;

        DrawRectangle(x, 62, 9, 4, cor);
        DrawCircle(x + 4, 66, 4.5f, cor);
    }

    DrawRectangle(cx - 54, 62, 108, 1, (Color){ 230, 190, 120, 255 });

    // LAMPADAS DO CARROSSEL (algumas piscando, outras queimadas)
    int instante = (int)(tempo * 7.0f);

    for (int i = 0; i < 18; i++) {

        int x = cx - 52 + i * 6;
        int queimada = (ruido(i, 91) % 5 == 0);
        int falhando = (ruido(i, 92) % 4 == 0);
        int acesa = !queimada && (!falhando || ruido(i, instante) % 3 != 0);

        if (acesa) {
            DrawCircle(x, 70, 3, Fade((Color){ 255, 170, 80, 255 }, 0.25f));
            DrawRectangle(x, 69, 2, 2, (Color){ 255, 214, 130, 255 });
        }
        else {
            DrawRectangle(x, 69, 2, 2, (Color){ 90, 50, 40, 255 });
        }
    }
}

static void desenharArvoreSeca(int x, int base, int altura, Color cor) {

    DrawLine(x, base, x, base - altura, cor);
    DrawLine(x + 1, base, x + 1, base - altura + 6, cor);

    DrawLine(x, base - altura / 2, x - 8, base - altura / 2 - 9, cor);
    DrawLine(x - 8, base - altura / 2 - 9, x - 11, base - altura / 2 - 15, cor);
    DrawLine(x, base - altura * 2 / 3, x + 9, base - altura * 2 / 3 - 7, cor);
    DrawLine(x + 9, base - altura * 2 / 3 - 7, x + 13, base - altura * 2 / 3 - 6, cor);
    DrawLine(x, base - altura, x - 4, base - altura - 6, cor);
    DrawLine(x, base - altura, x + 5, base - altura - 5, cor);
}

static void desenharOnibus(void) {

    int x = 254;
    int y = 128;
    Color lataria = { 150, 40, 40, 255 };
    Color ferrugem = { 104, 34, 30, 255 };
    Color creme = { 200, 170, 150, 255 };
    Color vidro = { 40, 16, 30, 255 };

    // SOMBRA E CORPO
    DrawEllipse(x + 26, CHAO_BASE + 1, 30, 2, Fade(BLACK, 0.5f));
    DrawRectangleRounded((Rectangle){ (float)x, (float)y, 54, 20 }, 0.35f, 4, lataria);
    DrawRectangle(x + 2, y + 1, 50, 7, creme);

    // JANELAS (uma quebrada)
    for (int i = 0; i < 6; i++) {

        int jx = x + 4 + i * 8;

        DrawRectangle(jx, y + 2, 6, 5, vidro);
        DrawPixel(jx + 1, y + 2, (Color){ 220, 110, 120, 255 });

        if (i == 2) {
            DrawLine(jx, y + 2, jx + 5, y + 6, creme);
        }
    }

    // MANCHAS DE FERRUGEM
    DrawRectangle(x + 8, y + 12, 6, 3, ferrugem);
    DrawRectangle(x + 34, y + 14, 9, 2, ferrugem);
    DrawRectangle(x + 2, y + 10, 50, 1, (Color){ 230, 200, 170, 255 });

    // FAROL APAGADO E RODAS (uma murcha)
    DrawRectangle(x + 51, y + 11, 2, 3, (Color){ 200, 190, 160, 255 });
    DrawCircle(x + 12, y + 20, 4, (Color){ 22, 12, 18, 255 });
    DrawEllipse(x + 42, y + 21, 5, 3, (Color){ 22, 12, 18, 255 });
    DrawCircle(x + 12, y + 20, 1.5f, (Color){ 100, 90, 90, 255 });
}

static void desenharCarrinho(void) {

    int x = 104;
    int y = 138;
    Color corpo = { 60, 90, 110, 255 };

    DrawEllipse(x + 11, CHAO_BASE + 1, 13, 2, Fade(BLACK, 0.5f));
    DrawRectangleRounded((Rectangle){ (float)x, (float)y, 22, 10 }, 0.5f, 4, corpo);
    DrawRectangle(x + 3, y + 2, 7, 4, (Color){ 30, 20, 30, 255 });
    DrawRectangle(x + 12, y + 2, 7, 4, (Color){ 30, 20, 30, 255 });
    DrawRectangle(x + 2, y + 6, 18, 1, (Color){ 200, 180, 120, 255 });
    DrawCircle(x + 5, y + 10, 2, (Color){ 22, 12, 18, 255 });
    DrawCircle(x + 17, y + 10, 2, (Color){ 22, 12, 18, 255 });
}

static void desenharMatoSeco(void) {

    Color mato = { 70, 26, 34, 255 };

    for (int i = 0; i < 40; i++) {

        int x = ruido(i, 101) % BASE_W;
        int altura = 2 + ruido(i, 102) % 4;

        DrawLine(x, CHAO_BASE, x - 1, CHAO_BASE - altura, mato);
        DrawLine(x + 1, CHAO_BASE, x + 2, CHAO_BASE - altura + 1, mato);
    }
}

static void desenharPocas(float tempo) {

    // POCAS REFLETINDO O CEU VERMELHO (desenhadas por cima do chao da matriz)
    int pocas[4][3] = { { 60, 160, 22 }, { 150, 168, 30 }, { 236, 158, 18 }, { 296, 172, 14 } };

    for (int i = 0; i < 4; i++) {

        int x = pocas[i][0];
        int y = pocas[i][1];
        int largura = pocas[i][2];

        DrawEllipse(x, y, (float)largura, 2.5f, (Color){ 150, 50, 66, 255 });
        DrawEllipse(x - 2, y, largura * 0.6f, 1.5f, (Color){ 220, 110, 110, 255 });

        // REFLEXO TREMENDO DE UMA LUZ
        int brilho = (int)(sinf(tempo * 3.0f + i) * 2.0f);
        DrawRectangle(x + brilho - 2, y - 1, 3, 1, (Color){ 255, 200, 180, 255 });
    }
}

static void desenharNeblina(float tempo) {

    Color nevoa = { 220, 150, 170, 255 };

    for (int camada = 0; camada < 3; camada++) {

        float velocidade = 3.0f + camada * 2.0f;
        int y = 124 + camada * 9;

        for (int i = 0; i < 5; i++) {

            float x = fmodf(i * 90.0f + tempo * velocidade + camada * 40.0f, 400.0f) - 60.0f;

            DrawEllipse((int)x, y, 50, 4 + camada, Fade(nevoa, 0.07f + camada * 0.02f));
        }
    }
}

static void desenharCorvos(float tempo) {

    Color corvo = { 20, 6, 16, 255 };

    for (int i = 0; i < 3; i++) {

        float x = fmodf(tempo * (10.0f + i * 3.0f) + i * 140.0f, 380.0f) - 30.0f;
        float y = 22.0f + i * 11.0f + sinf(tempo * 1.5f + i) * 3.0f;
        int asaBaixa = ((int)(tempo * 6.0f + i) % 2) == 0;

        DrawPixel((int)x, (int)y, corvo);

        if (asaBaixa) {
            DrawPixel((int)x - 1, (int)y + 1, corvo);
            DrawPixel((int)x + 1, (int)y + 1, corvo);
            DrawPixel((int)x - 2, (int)y + 1, corvo);
            DrawPixel((int)x + 2, (int)y + 1, corvo);
        }
        else {
            DrawPixel((int)x - 1, (int)y - 1, corvo);
            DrawPixel((int)x + 1, (int)y - 1, corvo);
            DrawPixel((int)x - 2, (int)y - 2, corvo);
            DrawPixel((int)x + 2, (int)y - 2, corvo);
        }
    }
}

static void desenharParque(float tempo) {

    desenharCeuParque(tempo);
    desenharCorvos(tempo);
    desenharFundoParque(tempo);
    desenharMontanhaRussaParque();
    desenharRodaGiganteParque(tempo);

    desenharArvoreSeca(100, CHAO_BASE, 34, (Color){ 40, 12, 26, 255 });
    desenharArvoreSeca(232, CHAO_BASE, 28, (Color){ 40, 12, 26, 255 });

    desenharCarrossel(tempo);
    desenharCarrinho();
    desenharOnibus();
    desenharMatoSeco();

    // CHAO DE ASFALTO MOLHADO (MATRIZ) + POCAS
    desenharChao((Color){ 64, 38, 54, 255 }, (Color){ 40, 24, 38, 255 }, (Color){ 28, 16, 26, 255 });
    desenharPocas(tempo);

    desenharNeblina(tempo);
}

// =====================================
// LIBERAR A TEXTURA
// =====================================
void descarregarCenarios(void) {

    if (telaCarregada) {

        UnloadRenderTexture(telaArena);
        telaCarregada = 0;
    }
}

// =====================================
// DESENHAR A ARENA ATUAL
// =====================================
void desenharArena(void) {

    if (!mapaPronto) {
        montarMapa();
    }

    if (!telaCarregada) {

        telaArena = LoadRenderTexture(BASE_W, BASE_H);
        SetTextureFilter(telaArena.texture, TEXTURE_FILTER_POINT);
        telaCarregada = 1;
    }

    float tempo = (float)GetTime();

    // DESENHA A ARENA NA TELA PEQUENA...
    BeginTextureMode(telaArena);

    ClearBackground(BLACK);

    if (arenaAtual == ARENA_COLISEU) {
        desenharColiseu(tempo);
    }
    else {
        desenharParque(tempo);
    }

    EndTextureMode();

    // ...E AMPLIA PARA A JANELA (3x). O BLEND PRE-MULTIPLICADO EVITA QUE AS PARTES
    // SEMITRANSPARENTES (neblina, luz, brilhos) FIQUEM MAIS ESCURAS NA AMPLIACAO
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

    DrawTexturePro(
        telaArena.texture,
        (Rectangle){ 0, 0, (float)BASE_W, -(float)BASE_H },
        (Rectangle){ 0, 0, (float)LARGURA_TELA, (float)ALTURA_TELA },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    EndBlendMode();
}
