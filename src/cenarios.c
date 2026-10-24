#include "cenarios.h"
#include <math.h>

// =====================================
// ESTADO INTERNO DO MODULO
// =====================================
static int mapa[LINHAS_MAPA][COLUNAS_MAPA];
static int mapaPronto = 0;
static ArenaId arenaAtual = ARENA_PARQUE;

static const char *NOMES_ARENAS[TOTAL_ARENAS] = {
    "PARQUE DE DIVERSOES",
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

static Color clarear(Color cor, int quantidade) {

    int r = cor.r + quantidade;
    int g = cor.g + quantidade;
    int b = cor.b + quantidade;

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;

    return (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, cor.a };
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
// =====================================
static void desenharChao(Color superficie, Color solo, Color detalhe, int comGrade) {

    for (int linha = 0; linha < LINHAS_MAPA; linha++) {

        for (int coluna = 0; coluna < COLUNAS_MAPA; coluna++) {

            int tile = mapa[linha][coluna];
            int x = coluna * TAMANHO_TILE;
            int y = linha * TAMANHO_TILE;

            if (tile == TILE_SUPERFICIE) {

                DrawRectangle(x, y, TAMANHO_TILE, TAMANHO_TILE, superficie);
                DrawRectangle(x, y, TAMANHO_TILE, 4, clarear(superficie, 35));
            }
            else if (tile == TILE_SOLO) {

                DrawRectangle(x, y, TAMANHO_TILE, TAMANHO_TILE, solo);
            }
            else {
                continue;
            }

            // DETALHES (pedrinhas, graos de areia)
            int n = ruido(linha, coluna);

            if (n % 3 == 0) {
                DrawCircle(x + n % TAMANHO_TILE, y + 6 + (n / 7) % (TAMANHO_TILE - 8), 2, detalhe);
            }

            if (comGrade) {
                DrawRectangleLines(x, y, TAMANHO_TILE, TAMANHO_TILE, Fade(BLACK, 0.18f));
            }
        }
    }
}

// =====================================
// ARENA 1 - PARQUE DE DIVERSOES
// =====================================
static float alturaTrilho(float x) {

    return 270.0f + 55.0f * sinf(x * 0.018f) + 25.0f * sinf(x * 0.047f + 1.0f);
}

static void desenharMontanhaRussa(float tempo) {

    Color estrutura = { 60, 30, 70, 255 };
    Color trilho = { 95, 45, 105, 255 };

    // PILARES
    for (int x = 560; x <= LARGURA_TELA; x += 40) {

        float y = alturaTrilho((float)x);

        DrawLineEx((Vector2){ (float)x, y }, (Vector2){ (float)x, ALTURA_CHAO }, 3, estrutura);
    }

    // TRILHO
    Vector2 anterior = { 560.0f, alturaTrilho(560.0f) };

    for (int x = 568; x <= LARGURA_TELA; x += 8) {

        Vector2 ponto = { (float)x, alturaTrilho((float)x) };

        DrawLineEx(anterior, ponto, 5, trilho);

        anterior = ponto;
    }

    // CARRINHO ANDANDO NO TRILHO
    float cx = 560.0f + fmodf(tempo * 110.0f, 400.0f);
    float cy = alturaTrilho(cx);

    DrawRectangle((int)cx - 14, (int)cy - 16, 28, 12, (Color){ 255, 220, 80, 255 });
    DrawCircle((int)cx - 8, (int)cy - 3, 3, BLACK);
    DrawCircle((int)cx + 8, (int)cy - 3, 3, BLACK);
}

static void desenharRodaGigante(float tempo) {

    Vector2 centro = { 230.0f, 250.0f };
    float raio = 140.0f;
    float giro = tempo * 0.25f;

    Color estrutura = { 240, 220, 240, 255 };
    Color estruturaEscura = { 120, 80, 130, 255 };

    Color coresGondola[4] = {
        { 230, 70, 90, 255 },
        { 70, 170, 230, 255 },
        { 250, 200, 60, 255 },
        { 120, 210, 120, 255 }
    };

    // PERNAS DE SUSTENTACAO
    DrawLineEx(centro, (Vector2){ centro.x - 95.0f, ALTURA_CHAO }, 7, estruturaEscura);
    DrawLineEx(centro, (Vector2){ centro.x + 95.0f, ALTURA_CHAO }, 7, estruturaEscura);

    // RAIOS DA RODA
    for (int i = 0; i < 12; i++) {

        float angulo = giro + i * (2.0f * PI / 12.0f);
        Vector2 ponta = { centro.x + cosf(angulo) * raio, centro.y + sinf(angulo) * raio };

        DrawLineEx(centro, ponta, 2, estrutura);
    }

    // ARCOS
    DrawRing(centro, raio - 4.0f, raio, 0.0f, 360.0f, 72, estrutura);
    DrawRing(centro, raio * 0.55f - 3.0f, raio * 0.55f, 0.0f, 360.0f, 48, estrutura);

    // LUZES PISCANDO NO ARCO
    for (int i = 0; i < 36; i++) {

        float angulo = giro + i * (2.0f * PI / 36.0f);
        int acesa = ((i + (int)(tempo * 4.0f)) % 3) == 0;

        DrawCircle(
            (int)(centro.x + cosf(angulo) * (raio - 2.0f)),
            (int)(centro.y + sinf(angulo) * (raio - 2.0f)),
            3,
            acesa ? (Color){ 255, 240, 140, 255 } : Fade(WHITE, 0.35f)
        );
    }

    // GONDOLAS (ficam sempre penduradas para baixo)
    for (int i = 0; i < 12; i++) {

        float angulo = giro + i * (2.0f * PI / 12.0f);
        Vector2 ponta = { centro.x + cosf(angulo) * raio, centro.y + sinf(angulo) * raio };

        DrawLineEx(ponta, (Vector2){ ponta.x, ponta.y + 8.0f }, 2, estrutura);

        DrawRectangleRounded(
            (Rectangle){ ponta.x - 11.0f, ponta.y + 8.0f, 22.0f, 16.0f },
            0.4f, 4, coresGondola[i % 4]
        );
    }

    // EIXO CENTRAL
    DrawCircleV(centro, 12.0f, estrutura);
    DrawCircleV(centro, 5.0f, estruturaEscura);
}

static void desenharBarraca(float cx, Color cor, float tempo) {

    float base = ALTURA_CHAO;
    float largura = 120.0f;
    float alturaParede = 70.0f;
    int listras = 6;
    float larguraListra = largura / listras;

    // PAREDE LISTRADA
    for (int i = 0; i < listras; i++) {

        DrawRectangle(
            (int)(cx - largura / 2.0f + i * larguraListra),
            (int)(base - alturaParede),
            (int)larguraListra + 1,
            (int)alturaParede,
            (i % 2 == 0) ? cor : RAYWHITE
        );
    }

    // ENTRADA
    DrawRectangle((int)cx - 15, (int)base - 45, 30, 45, (Color){ 40, 20, 30, 255 });

    // TELHADO
    float topo = base - alturaParede - 55.0f;

    DrawTriangle(
        (Vector2){ cx, topo },
        (Vector2){ cx - largura / 2.0f - 10.0f, base - alturaParede },
        (Vector2){ cx + largura / 2.0f + 10.0f, base - alturaParede },
        cor
    );

    // BANDEIRINHA TREMULANDO
    float onda = 4.0f * sinf(tempo * 6.0f + cx);

    DrawLineEx((Vector2){ cx, topo }, (Vector2){ cx, topo - 25.0f }, 2, DARKGRAY);

    DrawTriangle(
        (Vector2){ cx, topo - 25.0f },
        (Vector2){ cx, topo - 15.0f },
        (Vector2){ cx + 18.0f, topo - 20.0f + onda },
        YELLOW
    );
}

static void desenharVaralDeLuzes(float tempo) {

    Color cores[4] = {
        { 255, 90, 90, 255 },
        { 255, 220, 80, 255 },
        { 90, 200, 255, 255 },
        { 140, 255, 140, 255 }
    };

    Vector2 anterior = { 0.0f, 0.0f };

    for (int i = 0; i <= 30; i++) {

        float x = i * 32.0f;
        float y = 185.0f - 0.00015f * (x - 480.0f) * (x - 480.0f);
        Vector2 ponto = { x, y };

        if (i > 0) {
            DrawLineEx(anterior, ponto, 1.5f, (Color){ 40, 25, 45, 255 });
        }

        int acesa = ((i + (int)(tempo * 3.0f)) % 2) == 0;
        Color cor = cores[i % 4];

        if (acesa) {
            DrawCircle((int)x, (int)y + 5, 8, Fade(cor, 0.25f));
            DrawCircle((int)x, (int)y + 5, 4, cor);
        }
        else {
            DrawCircle((int)x, (int)y + 5, 4, Fade(cor, 0.35f));
        }

        anterior = ponto;
    }
}

static void desenharParque(void) {

    float tempo = (float)GetTime();

    // CEU DE POR DO SOL
    DrawRectangleGradientV(
        0, 0, LARGURA_TELA, ALTURA_CHAO,
        (Color){ 28, 16, 64, 255 },
        (Color){ 235, 120, 85, 255 }
    );

    // ESTRELAS
    for (int i = 0; i < 45; i++) {

        int x = ruido(i, 1) % LARGURA_TELA;
        int y = ruido(i, 2) % 200;
        float brilho = 0.3f + 0.7f * fabsf(sinf(tempo * 1.5f + i));

        DrawCircle(x, y, 1.5f, Fade(WHITE, brilho));
    }

    // SOL SE PONDO
    DrawCircle(780, 400, 95, Fade((Color){ 255, 180, 110, 255 }, 0.25f));
    DrawCircle(780, 400, 70, Fade((Color){ 255, 210, 130, 255 }, 0.85f));

    // ARVORES AO FUNDO
    for (int i = 0; i < 20; i++) {

        int cx = i * 55 - 10;
        int raio = 45 + ruido(i, 3) % 30;

        DrawCircle(cx, ALTURA_CHAO - 10, (float)raio, (Color){ 70, 35, 80, 255 });
    }

    desenharMontanhaRussa(tempo);
    desenharRodaGigante(tempo);

    desenharBarraca(470.0f, (Color){ 200, 40, 60, 255 }, tempo);
    desenharBarraca(880.0f, (Color){ 40, 110, 200, 255 }, tempo);

    desenharVaralDeLuzes(tempo);

    // CHAO DE PEDRA
    desenharChao(
        (Color){ 95, 82, 105, 255 },
        (Color){ 62, 52, 72, 255 },
        (Color){ 120, 108, 130, 255 },
        1
    );
}

// =====================================
// ARENA 2 - COLISEU
// =====================================
static void desenharNuvem(float x, float y) {

    Color branco = (Color){ 255, 255, 255, 255 };

    DrawCircle((int)x, (int)y, 20, branco);
    DrawCircle((int)x + 22, (int)y - 8, 26, branco);
    DrawCircle((int)x + 48, (int)y, 20, branco);
    DrawRectangle((int)x, (int)y, 48, 20, branco);
}

static void desenharTocha(float x, float y, float tempo) {

    float tremor = sinf(tempo * 18.0f + x) * 2.0f + sinf(tempo * 31.0f) * 1.5f;

    // LUZ AO REDOR
    DrawCircle((int)x, (int)y - 14, 32, Fade(ORANGE, 0.12f));

    // SUPORTE
    DrawRectangle((int)x - 4, (int)y, 8, 30, (Color){ 70, 45, 25, 255 });
    DrawRectangle((int)x - 8, (int)y - 4, 16, 8, (Color){ 90, 60, 30, 255 });

    // FOGO
    DrawCircle((int)x, (int)y - 12, 11.0f + tremor * 0.5f, Fade(ORANGE, 0.85f));
    DrawCircle((int)x, (int)y - 17, 7.0f + tremor * 0.3f, YELLOW);
    DrawCircle((int)x, (int)(y - 22.0f - tremor), 4, Fade(WHITE, 0.8f));
}

static void desenharEstandarte(float x, float tempo) {

    float onda = 5.0f * sinf(tempo * 3.0f + x * 0.05f);
    Color vermelho = { 150, 20, 25, 255 };
    Color dourado = { 230, 190, 70, 255 };

    DrawRectangle((int)x - 4, 362, 48, 5, (Color){ 90, 60, 30, 255 });
    DrawRectangle((int)x, 365, 40, 50, vermelho);

    DrawTriangle(
        (Vector2){ x, 415.0f },
        (Vector2){ x + 20.0f + onda, 432.0f },
        (Vector2){ x + 40.0f, 415.0f },
        vermelho
    );

    DrawRectangle((int)x + 4, 365, 3, 50, dourado);
    DrawRectangle((int)x + 33, 365, 3, 50, dourado);
    DrawCircle((int)x + 20, 388, 8, dourado);
}

static void desenharColiseu(void) {

    float tempo = (float)GetTime();

    Color pedra = { 205, 170, 125, 255 };
    Color pedraEscura = { 165, 130, 90, 255 };
    Color aberturaArco = { 230, 205, 165, 255 };

    // CEU DE DIA
    DrawRectangleGradientV(
        0, 0, LARGURA_TELA, ALTURA_CHAO,
        (Color){ 95, 155, 215, 255 },
        (Color){ 245, 215, 170, 255 }
    );

    // SOL
    DrawCircle(820, 120, 70, Fade((Color){ 255, 240, 180, 255 }, 0.3f));
    DrawCircle(820, 120, 40, (Color){ 255, 245, 200, 255 });

    // NUVENS
    for (int i = 0; i < 4; i++) {

        float x = fmodf((float)(ruido(i, 5) % LARGURA_TELA) + tempo * (8.0f + i * 3.0f), 1160.0f) - 100.0f;
        float y = 115.0f + (i % 2) * 25.0f;

        desenharNuvem(x, y);
    }

    // MURALHA DO LADO OPOSTO
    DrawRectangle(0, 150, LARGURA_TELA, 90, pedra);
    DrawRectangle(0, 145, LARGURA_TELA, 8, pedraEscura);

    for (int x = 10; x < LARGURA_TELA; x += 48) {

        DrawRectangle(x + 12, 180, 24, 40, aberturaArco);
        DrawCircle(x + 24, 180, 12, aberturaArco);
    }

    DrawRectangle(0, 232, LARGURA_TELA, 8, pedraEscura);

    // ARQUIBANCADA COM PUBLICO
    Color roupas[6] = {
        { 200, 50, 50, 255 }, { 50, 90, 180, 255 }, { 230, 200, 60, 255 },
        { 60, 150, 80, 255 }, { 240, 240, 240, 255 }, { 130, 60, 150, 255 }
    };

    Color peles[3] = {
        { 240, 200, 160, 255 }, { 190, 130, 90, 255 }, { 120, 80, 55, 255 }
    };

    for (int fileira = 0; fileira < 6; fileira++) {

        int y = 240 + fileira * 20;

        DrawRectangle(
            0, y, LARGURA_TELA, 20,
            (fileira % 2) ? (Color){ 180, 145, 105, 255 } : (Color){ 195, 160, 118, 255 }
        );

        for (int x = 6 + (fileira % 2) * 7; x < LARGURA_TELA; x += 14) {

            int n = ruido(fileira, x);

            // LUGAR VAZIO
            if (n % 7 == 0) {
                continue;
            }

            // PARTE DO PUBLICO PULA
            float pulo = 0.0f;

            if (n % 3 == 0) {
                pulo = fabsf(sinf(tempo * 6.0f + (float)(n % 10))) * 3.0f;
            }

            DrawRectangle(x - 4, (int)(y + 9 - pulo), 8, 10, roupas[n % 6]);
            DrawCircle(x, (int)(y + 6 - pulo), 3.5f, peles[n % 3]);
        }
    }

    // PARAPEITO
    DrawRectangle(0, 356, LARGURA_TELA, 8, pedraEscura);

    // MURO DO PODIO COM BLOCOS DE PEDRA
    DrawRectangle(0, 362, LARGURA_TELA, ALTURA_CHAO - 362, (Color){ 175, 140, 100, 255 });

    for (int linha = 0; linha < 5; linha++) {

        int y = 362 + linha * 18;
        int deslocamento = (linha % 2) * 15;

        for (int x = -deslocamento; x < LARGURA_TELA; x += 30) {
            DrawRectangleLines(x, y, 30, 18, Fade(BLACK, 0.2f));
        }
    }

    // PORTAO CENTRAL
    Color portao = { 40, 25, 15, 255 };

    DrawRectangle(440, 395, 80, ALTURA_CHAO - 395, portao);
    DrawCircle(480, 395, 40, portao);

    for (int x = 450; x <= 510; x += 12) {
        DrawLineEx((Vector2){ (float)x, 365.0f }, (Vector2){ (float)x, ALTURA_CHAO }, 2, (Color){ 110, 110, 115, 255 });
    }

    DrawLineEx((Vector2){ 442.0f, 405.0f }, (Vector2){ 518.0f, 405.0f }, 2, (Color){ 110, 110, 115, 255 });
    DrawLineEx((Vector2){ 440.0f, 425.0f }, (Vector2){ 520.0f, 425.0f }, 2, (Color){ 110, 110, 115, 255 });

    // ESTANDARTES
    desenharEstandarte(110.0f, tempo);
    desenharEstandarte(310.0f, tempo);
    desenharEstandarte(610.0f, tempo);
    desenharEstandarte(810.0f, tempo);

    // TOCHAS
    desenharTocha(220.0f, 385.0f, tempo);
    desenharTocha(740.0f, 385.0f, tempo);

    // CHAO DE AREIA
    desenharChao(
        (Color){ 218, 182, 122, 255 },
        (Color){ 192, 152, 96, 255 },
        (Color){ 170, 130, 80, 255 },
        0
    );
}

// =====================================
// DESENHAR A ARENA ATUAL
// =====================================
void desenharArena(void) {

    if (!mapaPronto) {
        montarMapa();
    }

    if (arenaAtual == ARENA_COLISEU) {
        desenharColiseu();
    }
    else {
        desenharParque();
    }
}