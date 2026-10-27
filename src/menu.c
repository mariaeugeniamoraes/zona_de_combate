#include "menu.h"
#include <math.h>
#include <stddef.h>

// =====================================
// TELA BASE EM PIXEL ART
// =====================================
#define BASE_W 320
#define BASE_H 180
#define CHAO   147

// =====================================
// MEDIDAS DO MENU (em pixels da tela base)
// =====================================
#define TOTAL_OPCOES     3
#define LARGURA_BOTAO    96
#define ALTURA_BOTAO     17
#define INCLINACAO       6
#define Y_PRIMEIRO_BOTAO 84
#define ESPACO_BOTOES    21
#define DURACAO_INTRO    0.6f

#define TOTAL_LUTADORES  4
#define TAMANHO_FRAME    64
#define PE_NO_FRAME      61   // linha do frame onde ficam os pes dos sprites
#define CENTRO_NO_FRAME  30   // coluna do frame onde fica o quadril

#define COR_SILHUETA (Color){ 58, 24, 68, 255 }
#define COR_SOMBRA   (Color){ 22, 12, 30, 255 }
#define COR_LARANJA  (Color){ 255, 140, 40, 255 }
#define COR_AMARELO  (Color){ 255, 214, 80, 255 }

// =====================================
// LUTADORES DO MENU
// =====================================
typedef struct {
    const char *nome;
    const char *arquivoSprite;
    const char *arquivoAura;
    int framesParado;
    int framesAura;
    float x;            // onde fica o centro do corpo na tela base
    int olhaDireita;
    Color cor;
    Texture2D sprite;
    Texture2D aura;
} LutadorMenu;

static LutadorMenu lutadores[TOTAL_LUTADORES] = {
    { "KAI",  "assets/personagens/kai/kai_parado.png",   "assets/personagens/kai/efeitos/aura_eletrica.png", 4, 6,  32.0f, 1, { 255, 225, 80, 255 }, { 0 }, { 0 } },
    { "NERO", "assets/personagens/nero/nero_parado.png", "assets/personagens/nero/efeitos/aura_agua.png",    4, 6,  76.0f, 1, { 90, 180, 255, 255 }, { 0 }, { 0 } },
    { "LUNA", "assets/personagens/luna/luna_parado.png", "assets/personagens/luna/efeitos/aura_fogo.png",    4, 6, 244.0f, 0, { 255, 110, 50, 255 }, { 0 }, { 0 } },
    { "AYRA", "assets/personagens/ayra/ayra_parado.png", "assets/personagens/ayra/efeitos/aura_vento.png",   4, 6, 288.0f, 0, { 120, 230, 170, 255 }, { 0 }, { 0 } }
};

// =====================================
// OPCOES DO MENU
// =====================================
static const char *TEXTOS_OPCOES[TOTAL_OPCOES] = { "JOGAR", "RANKING", "SAIR" };

static const Color CORES_OPCOES[TOTAL_OPCOES] = {
    { 255, 120, 40, 255 },
    { 240, 175, 40, 255 },
    { 215, 50, 70, 255 }
};

// =====================================
// ESTADO INTERNO DO MENU
// =====================================
static RenderTexture2D telaPixel;
static int recursosCarregados = 0;

static int opcaoSelecionada = 0;
static float destaque[TOTAL_OPCOES] = { 1.0f, 0.0f, 0.0f };   // 0 = normal, 1 = selecionado
static float tempoIntro = 0.0f;
static double ultimaAtualizacao = -10.0;

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

// ARREDONDA O DEGRADE EM "DEGRAUS" (fica com cara de pixel art)
static float degrau(float t, int degraus) {

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

static Vector2 deslocar(Vector2 p, float dx, float dy) {

    return (Vector2){ p.x + dx, p.y + dy };
}

// SUAVIZA A ENTRADA DO TITULO (passa um pouco do ponto e volta)
static float easeSaida(float t) {

    if (t >= 1.0f) return 1.0f;
    if (t <= 0.0f) return 0.0f;

    float c1 = 1.70158f;
    float c3 = c1 + 1.0f;
    float u = t - 1.0f;

    return 1.0f + c3 * u * u * u + c1 * u * u;
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
    SetTextureFilter(telaPixel.texture, TEXTURE_FILTER_POINT);

    for (int i = 0; i < TOTAL_LUTADORES; i++) {

        lutadores[i].sprite = carregarSeExistir(lutadores[i].arquivoSprite);
        lutadores[i].aura = carregarSeExistir(lutadores[i].arquivoAura);
    }

    recursosCarregados = 1;
}

void descarregarMenuInicial(void) {

    if (!recursosCarregados) {
        return;
    }

    UnloadRenderTexture(telaPixel);

    for (int i = 0; i < TOTAL_LUTADORES; i++) {

        if (lutadores[i].sprite.id != 0) UnloadTexture(lutadores[i].sprite);
        if (lutadores[i].aura.id != 0)   UnloadTexture(lutadores[i].aura);

        lutadores[i].sprite = (Texture2D){ 0 };
        lutadores[i].aura = (Texture2D){ 0 };
    }

    recursosCarregados = 0;
}

// =====================================
// ESCALA DA TELA BASE NA JANELA
// =====================================
static float escalaTela(void) {

    float escala = fminf(GetScreenWidth() / (float)BASE_W, GetScreenHeight() / (float)BASE_H);

    // ESCALA INTEIRA DEIXA TODOS OS PIXELS DO MESMO TAMANHO
    if (escala >= 1.0f) {
        escala = floorf(escala);
    }

    return escala;
}

static Vector2 origemTela(float escala) {

    return (Vector2){
        (GetScreenWidth() - BASE_W * escala) / 2.0f,
        (GetScreenHeight() - BASE_H * escala) / 2.0f
    };
}

static Vector2 mouseNaTelaBase(void) {

    float escala = escalaTela();
    Vector2 origem = origemTela(escala);
    Vector2 mouse = GetMousePosition();

    return (Vector2){ (mouse.x - origem.x) / escala, (mouse.y - origem.y) / escala };
}

// =====================================
// RETANGULO DE CADA BOTAO (para o mouse)
// =====================================
static Rectangle areaBotao(int i) {

    float largura = LARGURA_BOTAO + destaque[i] * 10.0f;
    float x = BASE_W / 2.0f - largura / 2.0f + destaque[i] * 3.0f;
    float y = (float)(Y_PRIMEIRO_BOTAO + i * ESPACO_BOTOES);

    return (Rectangle){ x, y, largura + INCLINACAO, ALTURA_BOTAO };
}

// =====================================
// ATUALIZAR MENU
// =====================================
OpcaoMenu atualizarMenuInicial(float dt) {

    double agora = GetTime();

    carregarRecursos();

    // SE O MENU FICOU UM TEMPO SEM SER ATUALIZADO, O JOGADOR ACABOU DE VOLTAR:
    // A ANIMACAO DE ENTRADA RECOMECA
    if (agora - ultimaAtualizacao > 0.25) {

        tempoIntro = 0.0f;
        opcaoSelecionada = 0;
    }

    ultimaAtualizacao = agora;
    tempoIntro += dt;

    // ANIMACAO SUAVE DO BOTAO SELECIONADO
    for (int i = 0; i < TOTAL_OPCOES; i++) {

        float alvo = (i == opcaoSelecionada) ? 1.0f : 0.0f;

        destaque[i] += (alvo - destaque[i]) * fminf(1.0f, dt * 12.0f);
    }

    // DURANTE A ENTRADA DO TITULO O MENU NAO RESPONDE
    // (evita que o ENTER/ESC da tela anterior escolha algo sem querer)
    if (tempoIntro < DURACAO_INTRO * 0.5f) {
        return OPCAO_NENHUMA;
    }

    // TECLADO: NAVEGAR
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        opcaoSelecionada = (opcaoSelecionada + TOTAL_OPCOES - 1) % TOTAL_OPCOES;
    }

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        opcaoSelecionada = (opcaoSelecionada + 1) % TOTAL_OPCOES;
    }

    // ATALHOS ANTIGOS (1, 2, 3) CONTINUAM FUNCIONANDO
    if (IsKeyPressed(KEY_ONE))   { opcaoSelecionada = 0; return OPCAO_JOGAR; }
    if (IsKeyPressed(KEY_TWO))   { opcaoSelecionada = 1; return OPCAO_RANKING; }
    if (IsKeyPressed(KEY_THREE)) { opcaoSelecionada = 2; return OPCAO_SAIR; }

    // MOUSE (convertido para as coordenadas da tela pequena)
    Vector2 mouse = mouseNaTelaBase();
    Vector2 movimento = GetMouseDelta();
    int mouseMexeu = (movimento.x != 0.0f || movimento.y != 0.0f);

    for (int i = 0; i < TOTAL_OPCOES; i++) {

        if (CheckCollisionPointRec(mouse, areaBotao(i))) {

            if (mouseMexeu) {
                opcaoSelecionada = i;
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                opcaoSelecionada = i;
                return (OpcaoMenu)(i + 1);
            }
        }
    }

    // CONFIRMAR
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return (OpcaoMenu)(opcaoSelecionada + 1);
    }

    return OPCAO_NENHUMA;
}

// =====================================
// FUNDO: CEU EM FAIXAS, ESTRELAS E SOL
// =====================================
static void desenharCeu(float tempo) {

    Color topo = { 16, 8, 42, 255 };
    Color meio = { 105, 34, 104, 255 };
    Color base = { 240, 115, 80, 255 };

    // CEU EM FAIXAS DE 3 PIXELS (degrade "em degraus")
    for (int y = 0; y < CHAO; y += 3) {

        Color cor;

        if (y < 95) {
            cor = misturarCores(topo, meio, degrau(y / 95.0f, 8));
        }
        else {
            cor = misturarCores(meio, base, degrau((y - 95) / (float)(CHAO - 95), 6));
        }

        DrawRectangle(0, y, BASE_W, 3, cor);
    }

    // ESTRELAS (algumas viram uma cruzinha quando brilham)
    for (int i = 0; i < 45; i++) {

        int x = ruido(i, 11) % BASE_W;
        int y = ruido(i, 12) % 85;
        float brilho = 0.5f + 0.5f * sinf(tempo * (1.0f + (i % 5) * 0.4f) + i);

        if (brilho < 0.25f) {
            continue;
        }

        Color cor = Fade(WHITE, brilho);

        DrawPixel(x, y, cor);

        if (i % 7 == 0 && brilho > 0.8f) {
            DrawPixel(x - 1, y, Fade(WHITE, 0.5f));
            DrawPixel(x + 1, y, Fade(WHITE, 0.5f));
            DrawPixel(x, y - 1, Fade(WHITE, 0.5f));
            DrawPixel(x, y + 1, Fade(WHITE, 0.5f));
        }
    }
}

static void desenharSol(float tempo) {

    int cx = BASE_W / 2;
    int cy = 136;
    int raio = 60;

    Color topo = { 255, 222, 120, 255 };
    Color base = { 255, 90, 110, 255 };

    // BRILHO AO REDOR
    DrawCircle(cx, cy, raio + 14, Fade((Color){ 255, 150, 110, 255 }, 0.12f));
    DrawCircle(cx, cy, raio + 6, Fade((Color){ 255, 170, 120, 255 }, 0.18f));

    // SOL LINHA POR LINHA, COM FAIXAS RETRO ANDANDO
    float deslocamento = fmodf(tempo * 4.0f, 6.0f);

    for (int y = cy - raio; y < CHAO; y++) {

        float dy = (float)(y - cy);
        float meiaLargura = sqrtf((float)(raio * raio) - dy * dy);

        if (meiaLargura <= 0.0f) {
            continue;
        }

        float inicioFaixas = cy - raio * 0.35f;

        if (y > inicioFaixas) {

            float k = (y - inicioFaixas) / (CHAO - inicioFaixas);
            int espessuraFaixa = 1 + (int)(k * 3.0f);
            int posicao = (int)(y - inicioFaixas + deslocamento) % 6;

            if (posicao < espessuraFaixa) {
                continue;
            }
        }

        float t = degrau((float)(y - (cy - raio)) / (float)(CHAO - (cy - raio)), 7);

        DrawRectangle(cx - (int)meiaLargura, y, (int)(meiaLargura * 2.0f), 1, misturarCores(topo, base, t));
    }
}

// =====================================
// SILHUETAS DAS DUAS ARENAS NO HORIZONTE
// =====================================
static void desenharSilhuetaColiseu(void) {

    Color pedra = COR_SILHUETA;
    Color arco = { 120, 48, 100, 255 };

    DrawRectangle(0, 110, 100, CHAO - 110, pedra);

    // RUINA
    triangulo((Vector2){ 100, 110 }, (Vector2){ 100, 124 }, (Vector2){ 115, 124 }, pedra);
    DrawRectangle(100, 124, 15, CHAO - 124, pedra);

    for (int x = 0; x < 88; x += 9) {
        DrawRectangle(x, 106, 5, 4, pedra);
    }

    // DUAS FILEIRAS DE ARCOS
    for (int linha = 0; linha < 2; linha++) {

        int y = 117 + linha * 15;

        for (int x = 3; x < 112; x += 11) {

            if (linha == 0 && x > 92) {
                continue;
            }

            DrawRectangle(x, y, 6, 9, arco);
            DrawCircle(x + 3, y, 3, arco);
        }
    }

    DrawRectangle(0, 128, 115, 2, pedra);
}

static void desenharSilhuetaRodaGigante(float tempo) {

    Vector2 centro = { 267.0f, 104.0f };
    float raio = 31.0f;
    float giro = tempo * 0.2f;
    Color cor = COR_SILHUETA;

    DrawLineEx(centro, (Vector2){ centro.x - 21.0f, CHAO }, 3, cor);
    DrawLineEx(centro, (Vector2){ centro.x + 21.0f, CHAO }, 3, cor);

    for (int i = 0; i < 10; i++) {

        float angulo = giro + i * (2.0f * PI / 10.0f);
        Vector2 ponta = { centro.x + cosf(angulo) * raio, centro.y + sinf(angulo) * raio };

        DrawLineEx(centro, ponta, 1, cor);
        DrawRectangle((int)ponta.x - 3, (int)ponta.y + 1, 6, 4, cor);

        // LUZINHAS PISCANDO
        int acesa = ((i + (int)(tempo * 3.0f)) % 3) == 0;

        DrawRectangle((int)ponta.x, (int)ponta.y - 1, 2, 2, acesa ? COR_AMARELO : Fade(COR_AMARELO, 0.25f));
    }

    DrawRing(centro, raio - 2.0f, raio, 0.0f, 360.0f, 40, cor);
    DrawCircleV(centro, 4.0f, cor);

    // BARRACA AO LADO
    DrawRectangle(298, 131, 24, CHAO - 131, cor);
    triangulo((Vector2){ 310, 119 }, (Vector2){ 295, 132 }, (Vector2){ 324, 132 }, cor);
}

static void desenharMorros(void) {

    for (int i = 0; i < 24; i++) {

        int cx = i * 15 - 4;
        int raio = 7 + ruido(i, 21) % 9;

        DrawCircle(cx, CHAO + 2, (float)raio, (Color){ 80, 32, 82, 255 });
    }
}

// =====================================
// CHAO DA ARENA COM GRADE EM PERSPECTIVA
// =====================================
static void desenharChaoMenu(float tempo) {

    int alturaChao = BASE_H - CHAO;

    // CHAO EM FAIXAS
    for (int y = CHAO; y < BASE_H; y += 3) {

        float t = degrau((y - CHAO) / (float)alturaChao, 5);

        DrawRectangle(0, y, BASE_W, 3, misturarCores((Color){ 54, 24, 62, 255 }, (Color){ 20, 10, 28, 255 }, t));
    }

    // LINHAS QUE SAEM DO HORIZONTE
    for (int i = -12; i <= 12; i++) {

        DrawLineV(
            (Vector2){ BASE_W / 2.0f + i * 8.0f, (float)CHAO },
            (Vector2){ BASE_W / 2.0f + i * 50.0f, (float)BASE_H },
            Fade(COR_LARANJA, 0.35f)
        );
    }

    // LINHAS HORIZONTAIS VINDO EM DIRECAO A TELA
    float avanco = fmodf(tempo * 0.6f, 1.0f);

    for (int k = 0; k < 6; k++) {

        float t = (k + avanco) / 6.0f;
        int y = CHAO + (int)(t * t * alturaChao);

        DrawRectangle(0, y, BASE_W, 1, Fade(COR_LARANJA, 0.15f + 0.3f * t));
    }

    // BORDA BRILHANTE DO HORIZONTE
    DrawRectangle(0, CHAO - 1, BASE_W, 1, (Color){ 255, 170, 90, 255 });
}

// =====================================
// OS QUATRO LUTADORES
// =====================================
static void desenharLutador(const LutadorMenu *lutador, int indice, int emDestaque, float tempo) {

    float x = lutador->x;

    // SOMBRA NO CHAO
    DrawEllipse((int)x, CHAO + 1, 12, 2, Fade(BLACK, 0.45f));

    // SE O SPRITE NAO FOI ENCONTRADO, DESENHA UMA SILHUETA SIMPLES NO LUGAR
    if (lutador->sprite.id == 0) {

        DrawRectangle((int)x - 4, CHAO - 34, 8, 34, Fade(lutador->cor, 0.6f));
        DrawCircle((int)x, CHAO - 38, 5, Fade(lutador->cor, 0.6f));
        return;
    }

    // ESPELHA O SPRITE PARA OS LUTADORES DO LADO DIREITO
    float larguraOrigem = lutador->olhaDireita ? TAMANHO_FRAME : -TAMANHO_FRAME;
    float centro = lutador->olhaDireita ? CENTRO_NO_FRAME : (TAMANHO_FRAME - CENTRO_NO_FRAME);

    Vector2 destino = { floorf(x - centro), (float)(CHAO - PE_NO_FRAME) };

    // AURA DO LUTADOR EM DESTAQUE (atras dele)
    if (emDestaque && lutador->aura.id != 0) {

        int frameAura = (int)(tempo * 12.0f) % lutador->framesAura;
        Rectangle origemAura = { (float)(frameAura * TAMANHO_FRAME), 0, larguraOrigem, TAMANHO_FRAME };

        DrawTextureRec(lutador->aura, origemAura, deslocar(destino, 0, 1), WHITE);
    }

    // ANIMACAO "PARADO" (cada lutador comeca num frame diferente)
    int frame = (int)(tempo * 6.0f + indice) % lutador->framesParado;
    Rectangle origem = { (float)(frame * TAMANHO_FRAME), 0, larguraOrigem, TAMANHO_FRAME };

    DrawTextureRec(lutador->sprite, origem, destino, WHITE);
}

static void desenharNomeLutador(const LutadorMenu *lutador, int emDestaque, float tempo) {

    Font fonte = GetFontDefault();
    Vector2 medida = MeasureTextEx(fonte, lutador->nome, 10, 1);
    Vector2 pos = { floorf(lutador->x - medida.x / 2.0f), (float)(CHAO + 5) };

    DrawTextEx(fonte, lutador->nome, deslocar(pos, 1, 1), 10, 1, COR_SOMBRA);
    DrawTextEx(fonte, lutador->nome, pos, 10, 1, emDestaque ? lutador->cor : Fade(lutador->cor, 0.45f));

    // SETINHA PULANDO EM CIMA DO LUTADOR EM DESTAQUE
    if (emDestaque) {

        int y = CHAO - 52 + (int)(sinf(tempo * 6.0f) * 2.0f);
        int cx = (int)lutador->x;

        triangulo((Vector2){ (float)cx - 3, (float)y }, (Vector2){ (float)cx + 4, (float)y }, (Vector2){ (float)cx + 0.5f, (float)y + 4 }, lutador->cor);
    }
}

// =====================================
// BRASAS E LINHAS DE VELOCIDADE
// =====================================
static void desenharParticulas(float tempo) {

    for (int i = 0; i < 36; i++) {

        float velocidade = 8.0f + (float)(ruido(i, 31) % 16);
        float percurso = fmodf(tempo * velocidade + (float)(ruido(i, 32) % BASE_H), (float)BASE_H);

        int y = BASE_H - (int)percurso;
        int x = ruido(i, 33) % BASE_W + (int)(sinf(tempo * 1.5f + i) * 4.0f);
        float alfa = 1.0f - percurso / BASE_H;

        // CADA PARTICULA TEM A COR DE UM DOS QUATRO ELEMENTOS
        Color cor = lutadores[i % TOTAL_LUTADORES].cor;

        DrawPixel(x, y, Fade(cor, alfa));

        if (i % 5 == 0) {
            DrawPixel(x + 1, y, Fade(cor, alfa * 0.6f));
        }
    }
}

static void desenharLinhasVelocidade(float tempo) {

    Vector2 centro = { BASE_W / 2.0f, 40.0f };

    for (int i = 0; i < 24; i++) {

        float angulo = i * (2.0f * PI / 24.0f) + tempo * 0.08f;
        float inicio = 50.0f + (float)(ruido(i, 41) % 14);

        Vector2 a = { centro.x + cosf(angulo) * inicio, centro.y + sinf(angulo) * inicio * 0.6f };
        Vector2 b = { centro.x + cosf(angulo) * 240.0f, centro.y + sinf(angulo) * 140.0f };

        DrawLineV(a, b, Fade(WHITE, 0.06f));
    }
}

// =====================================
// TITULO
// =====================================
static void textoComContorno(const char *texto, Vector2 pos, float tamanho, float espaco,
                             int contorno, int sombra, Color corContorno, Color corSombra, Color cor) {

    Font fonte = GetFontDefault();

    DrawTextEx(fonte, texto, deslocar(pos, (float)sombra, (float)sombra), tamanho, espaco, corSombra);

    for (int dx = -contorno; dx <= contorno; dx++) {
        for (int dy = -contorno; dy <= contorno; dy++) {
            DrawTextEx(fonte, texto, deslocar(pos, (float)dx, (float)dy), tamanho, espaco, corContorno);
        }
    }

    DrawTextEx(fonte, texto, pos, tamanho, espaco, cor);
}

static void desenharTitulo(float tempo) {

    Font fonte = GetFontDefault();

    // ENTRADA: O TITULO CAI DO TOPO E QUICA
    float entrada = easeSaida(tempoIntro / DURACAO_INTRO);
    float queda = floorf((1.0f - entrada) * -80.0f);

    // TREMIDA RAPIDA LOGO DEPOIS DO IMPACTO
    float tremida = 0.0f;

    if (tempoIntro > DURACAO_INTRO && tempoIntro < DURACAO_INTRO + 0.25f) {
        tremida = roundf(sinf(tempoIntro * 90.0f) * 2.0f);
    }

    // "ZONA DE" (tamanho 10 = fonte original da raylib, pixel perfeito)
    const char *linha1 = "ZONA DE";
    Vector2 med1 = MeasureTextEx(fonte, linha1, 10, 4);
    Vector2 pos1 = { floorf(BASE_W / 2.0f - med1.x / 2.0f + tremida), 8.0f + queda };

    textoComContorno(linha1, pos1, 10, 4, 1, 2, COR_SOMBRA, Fade(BLACK, 0.5f), (Color){ 255, 240, 220, 255 });

    // "COMBATE" (tamanho 40 = fonte 4x maior, ainda pixel perfeito)
    const char *linha2 = "COMBATE";
    float tam2 = 40.0f, esp2 = 4.0f;
    Vector2 med2 = MeasureTextEx(fonte, linha2, tam2, esp2);
    Vector2 pos2 = { floorf(BASE_W / 2.0f - med2.x / 2.0f - tremida), 21.0f + queda };

    textoComContorno(linha2, pos2, tam2, esp2, 2, 4, COR_SOMBRA, (Color){ 140, 20, 40, 230 }, (Color){ 235, 70, 35, 255 });

    // DEGRADE EM FAIXAS: AMARELO EM CIMA, LARANJA NO MEIO
    BeginScissorMode((int)pos2.x, (int)pos2.y, (int)med2.x + 4, 20);
    DrawTextEx(fonte, linha2, pos2, tam2, esp2, COR_LARANJA);
    EndScissorMode();

    BeginScissorMode((int)pos2.x, (int)pos2.y, (int)med2.x + 4, 8);
    DrawTextEx(fonte, linha2, pos2, tam2, esp2, COR_AMARELO);
    EndScissorMode();

    // BRILHO PASSANDO PELO TITULO A CADA 3 SEGUNDOS
    float ciclo = fmodf(tempo, 3.0f) / 1.2f;

    if (ciclo < 1.0f && tempoIntro > DURACAO_INTRO) {

        int xBrilho = (int)(pos2.x - 20.0f + ciclo * (med2.x + 40.0f));

        BeginScissorMode(xBrilho, (int)pos2.y, 8, (int)tam2);
        DrawTextEx(fonte, linha2, pos2, tam2, esp2, Fade(WHITE, 0.8f));
        EndScissorMode();
    }

    // SUBTITULO COM OS QUATRO ELEMENTOS NAS CORES DOS LUTADORES
    const char *elementos[4] = { "RAIO", "AGUA", "FOGO", "VENTO" };
    float larguraTotal = 0.0f;

    for (int i = 0; i < 4; i++) {
        larguraTotal += MeasureTextEx(fonte, elementos[i], 10, 1).x;
    }

    larguraTotal += 3 * 12.0f;

    float x = floorf(BASE_W / 2.0f - larguraTotal / 2.0f);
    float y = 66.0f + queda;

    DrawRectangle((int)x - 8, (int)y - 3, (int)larguraTotal + 16, 15, Fade(COR_SOMBRA, 0.8f));
    DrawRectangle((int)x - 8, (int)y - 3, (int)larguraTotal + 16, 1, COR_LARANJA);
    DrawRectangle((int)x - 8, (int)y + 11, (int)larguraTotal + 16, 1, COR_LARANJA);

    for (int i = 0; i < 4; i++) {

        DrawTextEx(fonte, elementos[i], (Vector2){ x, y }, 10, 1, lutadores[i].cor);
        x += MeasureTextEx(fonte, elementos[i], 10, 1).x;

        if (i < 3) {
            DrawRectangle((int)x + 5, (int)y + 4, 2, 2, Fade(WHITE, 0.6f));
            x += 12.0f;
        }
    }
}

// =====================================
// BOTOES
// =====================================
static void desenharBotao(int i, float tempo) {

    float s = destaque[i];
    Rectangle area = areaBotao(i);

    // ENTRADA: OS BOTOES SOBEM UM DEPOIS DO OUTRO
    float atrasoBotao = DURACAO_INTRO * 0.6f + i * 0.08f;
    float aparecer = (tempoIntro - atrasoBotao) / 0.25f;

    if (aparecer <= 0.0f) {
        return;
    }

    if (aparecer > 1.0f) {
        aparecer = 1.0f;
    }

    float subida = floorf((1.0f - aparecer) * 14.0f);

    float x0 = floorf(area.x);
    float x1 = floorf(area.x + area.width - INCLINACAO);
    float y0 = area.y + subida;
    float y1 = y0 + ALTURA_BOTAO;

    Vector2 a = { x0 + INCLINACAO, y0 };
    Vector2 b = { x1 + INCLINACAO, y0 };
    Vector2 c = { x1, y1 };
    Vector2 d = { x0, y1 };

    Color cor = CORES_OPCOES[i];
    float pulso = 0.5f + 0.5f * sinf(tempo * 6.0f);

    // BRILHO PULSANTE DO SELECIONADO
    if (s > 0.05f) {

        float g = 2.0f + floorf(pulso * 2.0f);

        quadrilatero(deslocar(a, -g, -g), deslocar(b, g, -g), deslocar(c, g, g), deslocar(d, -g, g),
                     Fade(cor, 0.3f * s * aparecer));
    }

    // SOMBRA, FUNDO E COR DO SELECIONADO
    quadrilatero(deslocar(a, 2, 2), deslocar(b, 2, 2), deslocar(c, 2, 2), deslocar(d, 2, 2), Fade(BLACK, 0.5f * aparecer));
    quadrilatero(a, b, c, d, Fade(COR_SOMBRA, 0.9f * aparecer));
    quadrilatero(a, b, c, d, Fade(cor, 0.9f * s * aparecer));

    // FAIXA CLARA EM CIMA
    quadrilatero(a, b, deslocar(b, -1, 4), deslocar(a, -1, 4), Fade(WHITE, (0.06f + 0.2f * s) * aparecer));

    // BORDA
    Color borda = Fade(misturarCores((Color){ 150, 110, 160, 255 }, WHITE, s), aparecer);

    DrawLineV(a, b, borda);
    DrawLineV(b, c, borda);
    DrawLineV(c, d, borda);
    DrawLineV(d, a, borda);

    // TEXTO
    Font fonte = GetFontDefault();
    Vector2 medida = MeasureTextEx(fonte, TEXTOS_OPCOES[i], 10, 2);
    Vector2 posTexto = {
        floorf((x0 + x1 + INCLINACAO) / 2.0f - medida.x / 2.0f),
        floorf((y0 + y1) / 2.0f - medida.y / 2.0f) + 1
    };

    Color corTexto = Fade(misturarCores((Color){ 175, 160, 185, 255 }, WHITE, s), aparecer);

    DrawTextEx(fonte, TEXTOS_OPCOES[i], deslocar(posTexto, 1, 1), 10, 2, Fade(BLACK, 0.6f * aparecer));
    DrawTextEx(fonte, TEXTOS_OPCOES[i], posTexto, 10, 2, corTexto);

    // NUMERO DO ATALHO
    DrawText(TextFormat("%d", i + 1), (int)d.x + 4, (int)y0 + 4, 10, Fade(WHITE, (0.3f + 0.4f * s) * aparecer));

    // SETAS ANIMADAS DOS DOIS LADOS DO SELECIONADO
    if (s > 0.5f) {

        float vai = floorf(sinf(tempo * 8.0f) * 2.0f);
        float meioY = floorf((y0 + y1) / 2.0f);
        float xe = d.x - 6.0f - vai;
        float xd = b.x + 2.0f + vai;
        Color corSeta = Fade(COR_AMARELO, (s - 0.5f) * 2.0f * aparecer);

        triangulo((Vector2){ xe - 4, meioY - 4 }, (Vector2){ xe - 4, meioY + 4 }, (Vector2){ xe + 1, meioY }, corSeta);
        triangulo((Vector2){ xd + 4, meioY - 4 }, (Vector2){ xd + 4, meioY + 4 }, (Vector2){ xd - 1, meioY }, corSeta);
    }
}

// =====================================
// RODAPE COM OS CONTROLES
// =====================================
static void desenharRodape(float tempo) {

    float piscar = 0.55f + 0.45f * sinf(tempo * 3.0f);

    DrawRectangle(0, BASE_H - 13, BASE_W, 13, Fade(BLACK, 0.55f));

    const char *controles = "SETAS/W S - ESCOLHER    ENTER - CONFIRMAR";
    int largura = MeasureText(controles, 10);

    DrawText(controles, BASE_W / 2 - largura / 2, BASE_H - 11, 10, Fade(WHITE, piscar));
}

// =====================================
// DESENHAR MENU COMPLETO
// =====================================
void desenharMenuInicial(void) {

    carregarRecursos();

    float tempo = (float)GetTime();

    // O LUTADOR EM DESTAQUE MUDA A CADA 2,5 SEGUNDOS
    int lutadorDestaque = (int)(tempo / 2.5f) % TOTAL_LUTADORES;

    // TUDO E DESENHADO NA TELA PEQUENA DE 320x180...
    BeginTextureMode(telaPixel);

    ClearBackground(BLACK);

    desenharCeu(tempo);
    desenharSol(tempo);
    desenharLinhasVelocidade(tempo);
    desenharMorros();
    desenharSilhuetaColiseu();
    desenharSilhuetaRodaGigante(tempo);
    desenharChaoMenu(tempo);

    for (int i = 0; i < TOTAL_LUTADORES; i++) {
        desenharLutador(&lutadores[i], i, i == lutadorDestaque, tempo);
    }

    desenharParticulas(tempo);

    for (int i = 0; i < TOTAL_LUTADORES; i++) {
        desenharNomeLutador(&lutadores[i], i == lutadorDestaque, tempo);
    }

    desenharTitulo(tempo);

    for (int i = 0; i < TOTAL_OPCOES; i++) {
        desenharBotao(i, tempo);
    }

    desenharRodape(tempo);

    // CLARAO BRANCO QUANDO O TITULO BATE NO LUGAR
    if (tempoIntro > DURACAO_INTRO * 0.7f && tempoIntro < DURACAO_INTRO + 0.35f) {

        float t = (tempoIntro - DURACAO_INTRO * 0.7f) / (DURACAO_INTRO * 0.3f + 0.35f);

        DrawRectangle(0, 0, BASE_W, BASE_H, Fade(WHITE, 0.55f * (1.0f - t)));
    }

    EndTextureMode();

    // ...E DEPOIS AMPLIADO PARA A JANELA (3x em 960x540)
    float escala = escalaTela();
    Vector2 origem = origemTela(escala);

    // BLEND PRE-MULTIPLICADO: as partes semitransparentes desenhadas na tela pequena
    // deixam o canal alfa dela menor que 255; sem isso elas apareceriam mais escuras
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

    DrawTexturePro(
        telaPixel.texture,
        (Rectangle){ 0, 0, (float)BASE_W, -(float)BASE_H },   // altura negativa: render texture vem de cabeca para baixo
        (Rectangle){ origem.x, origem.y, BASE_W * escala, BASE_H * escala },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    EndBlendMode();
}
