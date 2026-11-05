#include "janela.h"
#include <math.h>

static RenderTexture2D telaVirtual;
static int telaCriada = 0;
static int quadroAtivo = 0;

// =====================================
// ESCALA E POSICAO DA TELA VIRTUAL NA JANELA
// =====================================
static float escalaAtual(void) {

    return fminf(
        GetScreenWidth() / (float)JANELA_LARGURA,
        GetScreenHeight() / (float)JANELA_ALTURA
    );
}

static Vector2 origemAtual(float escala) {

    return (Vector2){
        floorf((GetScreenWidth() - JANELA_LARGURA * escala) / 2.0f),
        floorf((GetScreenHeight() - JANELA_ALTURA * escala) / 2.0f)
    };
}

// =====================================
// CRIAR A JANELA
// =====================================
void iniciarJanela(const char *titulo) {

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(JANELA_LARGURA, JANELA_ALTURA, titulo);

    SetWindowMinSize(JANELA_LARGURA / 2, JANELA_ALTURA / 2);

    telaVirtual = LoadRenderTexture(JANELA_LARGURA, JANELA_ALTURA);

    // FILTRO "POINT": MANTEM OS PIXELS NITIDOS NA AMPLIACAO
    SetTextureFilter(telaVirtual.texture, TEXTURE_FILTER_POINT);

    telaCriada = 1;
}

// =====================================
// ATUALIZAR (TELA CHEIA E MOUSE)
// =====================================
void atualizarJanela(void) {

    // F11 OU ALT+ENTER: ALTERNA TELA CHEIA
    if (IsKeyPressed(KEY_F11) ||
        ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_ENTER))) {

        ToggleBorderlessWindowed();
    }

    // O MOUSE PASSA A DEVOLVER COORDENADAS DA TELA VIRTUAL (960x540)
    float escala = escalaAtual();
    Vector2 origem = origemAtual(escala);

    SetMouseOffset((int)-origem.x, (int)-origem.y);
    SetMouseScale(1.0f / escala, 1.0f / escala);
}

// =====================================
// COMECO E FIM DE CADA QUADRO
// =====================================
void comecarQuadro(void) {

    BeginDrawing();

    BeginTextureMode(telaVirtual);
    ClearBackground(BLACK);

    quadroAtivo = 1;
}

void retomarQuadro(void) {

    if (quadroAtivo) {
        BeginTextureMode(telaVirtual);
    }
}

void terminarQuadro(void) {

    EndTextureMode();

    quadroAtivo = 0;

    float escala = escalaAtual();
    Vector2 origem = origemAtual(escala);

    ClearBackground(BLACK);

    // BLEND PRE-MULTIPLICADO: as partes semitransparentes nao ficam escuras
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

    DrawTexturePro(
        telaVirtual.texture,
        (Rectangle){ 0, 0, (float)JANELA_LARGURA, -(float)JANELA_ALTURA },
        (Rectangle){ origem.x, origem.y, JANELA_LARGURA * escala, JANELA_ALTURA * escala },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    EndBlendMode();

    EndDrawing();
}

// =====================================
// FECHAR
// =====================================
void fecharJanela(void) {

    if (telaCriada) {
        UnloadRenderTexture(telaVirtual);
        telaCriada = 0;
    }

    CloseWindow();
}