#include "interface.h"
#include <math.h>

// =====================================
// MEDIDAS DO HUD
// =====================================
#define MARGEM          30
#define LARGURA_VIDA    360
#define ALTURA_VIDA     22
#define Y_VIDA          40
#define LARGURA_ENERGIA 240
#define ALTURA_ENERGIA  10
#define Y_ENERGIA       70
#define Y_ROUNDS        96
#define VELOCIDADE_RASTRO 45.0f

static const char *NOMES[5] = {
    "LUNA", "AYRA", "KAI", "NERO", "???"
};

// RASTRO CLARO QUE DESCE DEVAGAR ATRAS DA BARRA DE VIDA
static float rastroVida1 = 100.0f;
static float rastroVida2 = 100.0f;

// =====================================
// FUNCOES AUXILIARES
// =====================================
static int limitarInt(int valor, int minimo, int maximo) {

    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

static float limitarFloat(float valor, float minimo, float maximo) {

    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

static float aproximarRastro(float rastro, int vida, float dt) {

    // VIDA SUBIU (novo round): o rastro acompanha na hora
    if (rastro < vida) {
        return (float)vida;
    }

    rastro -= VELOCIDADE_RASTRO * dt;

    if (rastro < vida) {
        rastro = (float)vida;
    }

    return rastro;
}

static Color corDaVida(int vida) {

    if (vida > 50) return (Color){ 60, 200, 90, 255 };
    if (vida > 25) return (Color){ 240, 200, 40, 255 };
    return (Color){ 230, 60, 50, 255 };
}

// =====================================
// DADOS DOS PERSONAGENS
// =====================================
const char *nomePersonagem(int personagem) {
    if (personagem < 0 || personagem >= TOTAL_PERSONAGENS) {
        return "???";
    }

    return NOMES[personagem];
}



Color corPersonagem(int personagem) {
    switch (personagem) {
        case PERSONAGEM_KAI:
            return YELLOW;

        case PERSONAGEM_LUNA:
            return RED;

        case PERSONAGEM_AYRA:
            return GREEN;

        case PERSONAGEM_NERO:
            return BLUE;

        default:
            return GRAY;
    }
}



// =====================================
// ATUALIZAR HUD
// =====================================
void atualizarHUD(const Partida *partida, float dt) {

    int vida1 = limitarInt(partida->vidaJogador1, 0, 100);
    int vida2 = limitarInt(partida->vidaJogador2, 0, 100);

    rastroVida1 = aproximarRastro(rastroVida1, vida1, dt);
    rastroVida2 = aproximarRastro(rastroVida2, vida2, dt);
}

// =====================================
// PARTES DO HUD
// =====================================
static void desenharBarraVida(int x, int vida, float rastro, int ancoradaDireita) {

    int larguraVida = vida * LARGURA_VIDA / 100;
    int larguraRastro = (int)(rastro * LARGURA_VIDA / 100.0f);

    // A BARRA DO J1 ESVAZIA PARA A ESQUERDA E A DO J2 PARA A DIREITA
    int xVida = ancoradaDireita ? x + LARGURA_VIDA - larguraVida : x;
    int xRastro = ancoradaDireita ? x + LARGURA_VIDA - larguraRastro : x;

    // FUNDO
    DrawRectangle(x - 3, Y_VIDA - 3, LARGURA_VIDA + 6, ALTURA_VIDA + 6, (Color){ 20, 20, 30, 230 });
    DrawRectangle(x, Y_VIDA, LARGURA_VIDA, ALTURA_VIDA, (Color){ 60, 25, 30, 255 });

    // RASTRO
    DrawRectangle(xRastro, Y_VIDA, larguraRastro, ALTURA_VIDA, (Color){ 255, 240, 220, 255 });

    // VIDA ATUAL
    DrawRectangle(xVida, Y_VIDA, larguraVida, ALTURA_VIDA, corDaVida(vida));
    DrawRectangle(xVida, Y_VIDA, larguraVida, 6, Fade(WHITE, 0.25f));

    // BORDA
    DrawRectangleLines(x - 3, Y_VIDA - 3, LARGURA_VIDA + 6, ALTURA_VIDA + 6, (Color){ 230, 230, 240, 255 });
}

static void desenharBarraEnergia(int x, float energia, int ancoradaDireita, float tempo) {

    energia = limitarFloat(energia, 0.0f, 100.0f);

    int largura = (int)(energia * LARGURA_ENERGIA / 100.0f);
    int xEnergia = ancoradaDireita ? x + LARGURA_ENERGIA - largura : x;
    Color cor = { 60, 140, 255, 255 };

    // ENERGIA CHEIA: A BARRA PULSA E MOSTRA "ESPECIAL!"
    if (energia >= 100.0f) {

        float pulso = 0.5f + 0.5f * sinf(tempo * 10.0f);

        cor = (Color){ (unsigned char)(80 + pulso * 140), (unsigned char)(170 + pulso * 70), 255, 255 };

        DrawRectangle(
            x - 6, Y_ENERGIA - 6, LARGURA_ENERGIA + 12, ALTURA_ENERGIA + 12,
            Fade(SKYBLUE, 0.3f * pulso)
        );

        const char *texto = "ESPECIAL!";
        int larguraTexto = MeasureText(texto, 14);
        int xTexto = ancoradaDireita ? x - 10 - larguraTexto : x + LARGURA_ENERGIA + 10;

        DrawText(texto, xTexto, Y_ENERGIA - 2, 14, cor);
    }

    DrawRectangle(x - 2, Y_ENERGIA - 2, LARGURA_ENERGIA + 4, ALTURA_ENERGIA + 4, (Color){ 20, 20, 30, 230 });
    DrawRectangle(xEnergia, Y_ENERGIA, largura, ALTURA_ENERGIA, cor);
}

static void desenharVitoriasRound(int xInicial, int vitorias, int sentido, Color cor) {

    for (int i = 0; i < 2; i++) {

        int cx = xInicial + sentido * (8 + i * 22);

        DrawCircle(cx, Y_ROUNDS, 7, (Color){ 20, 20, 30, 230 });

        if (i < vitorias) {
            DrawCircle(cx, Y_ROUNDS, 5, cor);
        }

        DrawCircleLines(cx, Y_ROUNDS, 7, WHITE);
    }
}

static void desenharRelogio(const Partida *partida, float tempo) {

    int cx = GetScreenWidth() / 2;

    DrawRectangle(cx - 50, 20, 100, 62, (Color){ 20, 20, 30, 230 });
    DrawRectangleLines(cx - 50, 20, 100, 62, WHITE);

    int segundos = (int)ceilf(partida->tempoRestante);

    if (segundos < 0) {
        segundos = 0;
    }

    int tamanho = 44;
    Color cor = WHITE;

    // ULTIMOS 10 SEGUNDOS: O NUMERO FICA VERMELHO E PULSA
    if (segundos <= 10 && segundos > 0) {

        float pulso = 0.5f + 0.5f * sinf(tempo * 12.0f);

        tamanho = 44 + (int)(pulso * 6.0f);
        cor = (Color){ 255, (unsigned char)(80 + pulso * 60), 70, 255 };
    }

    const char *textoTempo = TextFormat("%02d", segundos);
    int largura = MeasureText(textoTempo, tamanho);

    DrawText(textoTempo, cx - largura / 2, 51 - tamanho / 2, tamanho, cor);

    const char *textoRound = TextFormat("ROUND %d", partida->roundAtual);
    largura = MeasureText(textoRound, 18);

    DrawText(textoRound, cx - largura / 2, 88, 18, YELLOW);
}

// =====================================
// DESENHAR HUD COMPLETO
// =====================================
void desenharHUD(
    const Partida *partida,
    int personagemJogador1,
    int personagemJogador2,
    float energiaJogador1,
    float energiaJogador2
) {

    float tempo = (float)GetTime();
    int larguraTela = GetScreenWidth();

    int vida1 = limitarInt(partida->vidaJogador1, 0, 100);
    int vida2 = limitarInt(partida->vidaJogador2, 0, 100);

    int xVida2 = larguraTela - MARGEM - LARGURA_VIDA;
    int xEnergia2 = larguraTela - MARGEM - LARGURA_ENERGIA;

    // FAIXA ESCURA NO TOPO PARA DAR LEITURA SOBRE O CENARIO
    DrawRectangleGradientV(0, 0, larguraTela, 120, Fade(BLACK, 0.6f), Fade(BLACK, 0.0f));

    // NOMES
    const char *nome1 = nomePersonagem(personagemJogador1);
    const char *nome2 = nomePersonagem(personagemJogador2);

    DrawText(nome1, MARGEM, 14, 20, corPersonagem(personagemJogador1));
    DrawText(nome2, larguraTela - MARGEM - MeasureText(nome2, 20), 14, 20, corPersonagem(personagemJogador2));

    // VIDA
    desenharBarraVida(MARGEM, vida1, rastroVida1, 0);
    desenharBarraVida(xVida2, vida2, rastroVida2, 1);

    // ENERGIA
    desenharBarraEnergia(MARGEM, energiaJogador1, 0, tempo);
    desenharBarraEnergia(xEnergia2, energiaJogador2, 1, tempo);

    // ROUNDS VENCIDOS
    desenharVitoriasRound(MARGEM, partida->vitoriasJogador1, 1, corPersonagem(personagemJogador1));
    desenharVitoriasRound(larguraTela - MARGEM, partida->vitoriasJogador2, -1, corPersonagem(personagemJogador2));

    // CRONOMETRO E ROUND
    desenharRelogio(partida, tempo);
}