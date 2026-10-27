
#include "raylib.h"
#include "jogo.h"
#include "cenarios.h"
#include "interface.h"
#include "efeito.h"
#include "personagens.h"

#include <stdbool.h>
#include <stddef.h>
#include <math.h>

// =====================================
// CONFIGURACOES
// =====================================

#define VIDA_MAXIMA 100
#define ENERGIA_MAXIMA 100.0f

#define DANO_SOCO 10
#define DANO_CHUTE 15
#define DANO_FOGO 25

#define CUSTO_FOGO 50.0f

#define ALCANCE_SOCO 85.0f
#define ALCANCE_CHUTE 115.0f

#define TEMPO_SOCO 0.5f
#define TEMPO_CHUTE 0.625f
#define TEMPO_ESPECIAL 1.0f
#define TEMPO_DANO 0.375f

#define VELOCIDADE_FOGO 460.0f
#define RAIO_FOGO 19.0f

#define TEMPO_FIM_ROUND 1.6f

// Converte a escolha do menu para o identificador dos sprites.
static int obterIndicePersonagem(int personagem) {
switch (personagem) {
case 1:
return PERSONAGEM_KAI;

    case 2:
        return PERSONAGEM_LUNA;

    case 3:
        return PERSONAGEM_NERO;

    case 4:
        return PERSONAGEM_AYRA;

    default:
        return PERSONAGEM_LUNA;
}

}
// =====================================
// ESTRUTURA DA BOLA DE FOGO
// =====================================

typedef struct {
    bool ativa;
    Vector2 posicao;
    int direcao;
    int dono;
    float tempo;
} BolaFogo;

// =====================================
// VERIFICAR ALCANCE DOS GOLPES
// =====================================

static bool golpeAcertou(
    Vector2 atacante,
    Vector2 defensor,
    float alcance,
    bool viradoEsquerda
) {
    float dx = defensor.x - atacante.x;
    float dy = fabsf(defensor.y - atacante.y);

    if (viradoEsquerda && dx >= 0.0f) {
        return false;
    }

    if (!viradoEsquerda && dx <= 0.0f) {
        return false;
    }

    return fabsf(dx) <= alcance && dy <= 65.0f;
}

// =====================================
// COLISAO ENTRE OS PERSONAGENS
// =====================================

static void impedirSobreposicao(
    Vector2 *j1,
    Vector2 *j2
) {
    const float distanciaMinima = 70.0f;
    const float esquerda = 65.0f;
    const float direita = LARGURA_TELA - 65.0f;

    float dx = j2->x - j1->x;

    // Permite passar por cima do adversario
    // quando existe separacao vertical suficiente.
    if (fabsf(j2->y - j1->y) > 85.0f) {
        return;
    }

    if (fabsf(dx) >= distanciaMinima) {
        return;
    }

    float falta = distanciaMinima - fabsf(dx);

    if (dx >= 0.0f) {
        j1->x -= falta / 2.0f;
        j2->x += falta / 2.0f;
    }
    else {
        j1->x += falta / 2.0f;
        j2->x -= falta / 2.0f;
    }

    if (j1->x < esquerda) {
        j1->x = esquerda;
        j2->x = esquerda + distanciaMinima;
    }

    if (j2->x < esquerda) {
        j2->x = esquerda;
        j1->x = esquerda + distanciaMinima;
    }

    if (j1->x > direita) {
        j1->x = direita;
        j2->x = direita - distanciaMinima;
    }

    if (j2->x > direita) {
        j2->x = direita;
        j1->x = direita - distanciaMinima;
    }
}

// =====================================
// APLICAR DANO
// =====================================

static void aplicarDano(int *vida, int dano) {
    if (vida == NULL || dano <= 0) {
        return;
    }

    *vida -= dano;

    if (*vida < 0) {
        *vida = 0;
    }
}

// =====================================
// ATUALIZAR TEMPORIZADOR
// =====================================

static void atualizarTempo(float *tempo, float dt) {
    if (*tempo > 0.0f) {
        *tempo -= dt;

        if (*tempo < 0.0f) {
            *tempo = 0.0f;
        }
    }
}

// =====================================
// APLICAR GOLPE NORMAL
// =====================================

static bool executarGolpe(
    Vector2 atacante,
    Vector2 defensor,
    bool viradoEsquerda,
    float alcance,
    int dano,
    bool defendendo,
    int *vida,
    float *energia,
    float *tempoDano,
    Efeito **efeitos
) {
    if (!golpeAcertou(
        atacante,
        defensor,
        alcance,
        viradoEsquerda
    )) {
        return false;
    }

    aplicarDano(
        vida,
        defendendo ? dano / 5 : dano
    );

    *energia += (float)dano;

    if (*energia > ENERGIA_MAXIMA) {
        *energia = ENERGIA_MAXIMA;
    }

    if (!defendendo && *vida > 0) {
        *tempoDano = TEMPO_DANO;
    }

    efeitoImpacto(
        efeitos,
        (Vector2){
            defensor.x,
            defensor.y - 70.0f
        }
    );

    return true;
}

// =====================================
// CRIAR BOLA DE FOGO
// =====================================

static void criarBolaFogo(
    BolaFogo *bola,
    Vector2 personagem,
    bool viradoEsquerda,
    int dono
) {
    bola->ativa = true;
    bola->direcao = viradoEsquerda ? -1 : 1;
    bola->dono = dono;
    bola->tempo = 0.0f;

    bola->posicao = (Vector2){
        personagem.x + bola->direcao * 55.0f,
        personagem.y - 75.0f
    };
}

// =====================================
// ATUALIZAR BOLA DE FOGO
// =====================================

static void atualizarBolaFogo(
    BolaFogo *bola,
    Vector2 adversario,
    bool adversarioDefendendo,
    int *vidaAdversario,
    float *tempoDanoAdversario,
    float *tempoAtaqueAdversario,
    Efeito **efeitos,
    float dt
) {
    if (!bola->ativa) {
        return;
    }

    float xAnterior = bola->posicao.x;

    bola->posicao.x +=
        bola->direcao * VELOCIDADE_FOGO * dt;

    bola->tempo += dt;

    // A bola deve cruzar a faixa horizontal do corpo.
    float esquerdaCorpo = adversario.x - 28.0f;
    float direitaCorpo = adversario.x + 28.0f;

    float inicioTrajeto = fminf(
        xAnterior,
        bola->posicao.x
    );

    float fimTrajeto = fmaxf(
        xAnterior,
        bola->posicao.x
    );

    bool cruzouCorpo =
        fimTrajeto + RAIO_FOGO >= esquerdaCorpo &&
        inicioTrajeto - RAIO_FOGO <= direitaCorpo;

    // Centro aproximado do corpo do adversario.
    float centroY = adversario.y - 70.0f;

    bool mesmaAltura =
        fabsf(bola->posicao.y - centroY) <= 58.0f;

    if (cruzouCorpo && mesmaAltura) {

        int dano = adversarioDefendendo
            ? DANO_FOGO / 5
            : DANO_FOGO;

        aplicarDano(vidaAdversario, dano);

        if (
            !adversarioDefendendo &&
            *vidaAdversario > 0
        ) {
            *tempoDanoAdversario = TEMPO_DANO;
            *tempoAtaqueAdversario = 0.0f;
        }

        efeitoImpacto(
            efeitos,
            (Vector2){
                adversario.x,
                adversario.y - 70.0f
            }
        );

        efeitoFlash(
            efeitos,
            ORANGE,
            0.12f
        );

        bola->ativa = false;
        return;
    }

    // Desaparece quando sai da arena.
    if (
        bola->posicao.x < -50.0f ||
        bola->posicao.x > LARGURA_TELA + 50.0f
    ) {
        bola->ativa = false;
    }
}

// =====================================
// DESENHAR BOLA DE FOGO
// =====================================

static void desenharBolaFogo(const BolaFogo *bola) {
    if (!bola->ativa) {
        return;
    }

    Vector2 centro = bola->posicao;

    float oscilacao = sinf(bola->tempo * 25.0f);

    // Rastro
    for (int i = 1; i <= 5; i++) {

        Vector2 rastro = {
            centro.x - bola->direcao * i * 12.0f,
            centro.y + sinf(bola->tempo * 18.0f + i) * 5.0f
        };

        DrawCircleV(
            rastro,
            RAIO_FOGO - i * 2.2f,
            Fade(ORANGE, 0.45f - i * 0.06f)
        );
    }

    // Brilho externo
    DrawCircleV(
        centro,
        29.0f + oscilacao * 3.0f,
        Fade(ORANGE, 0.25f)
    );

    // Chama principal
    DrawCircleV(
        centro,
        RAIO_FOGO + oscilacao * 2.0f,
        RED
    );

    DrawCircleV(
        centro,
        15.0f,
        ORANGE
    );

    DrawCircleV(
        centro,
        9.0f,
        YELLOW
    );

    DrawCircleV(
        centro,
        4.0f,
        WHITE
    );
}

// =====================================
// REINICIAR LUTADORES
// =====================================

static void reiniciarLutadores(
    Vector2 *p1,
    Vector2 *p2,
    EstadoAnimacaoLuna *a1,
    EstadoAnimacaoLuna *a2,
    float *ataque1,
    float *ataque2,
    float *dano1,
    float *dano2,
    float *vy1,
    float *vy2,
    bool *pulando1,
    bool *pulando2,
    BolaFogo *fogo1,
    BolaFogo *fogo2
) {
    *p1 = (Vector2){300.0f, ALTURA_CHAO};
    *p2 = (Vector2){660.0f, ALTURA_CHAO};

    iniciarAnimacaoLuna(a1);
    iniciarAnimacaoLuna(a2);

    *ataque1 = 0.0f;
    *ataque2 = 0.0f;

    *dano1 = 0.0f;
    *dano2 = 0.0f;

    *vy1 = 0.0f;
    *vy2 = 0.0f;

    *pulando1 = false;
    *pulando2 = false;

    fogo1->ativa = false;
    fogo2->ativa = false;
}

// =====================================
// PREPARAR ARENA
// =====================================

static void prepararVisualPartida(Efeito **efeitos) {
    liberarEfeitos(efeitos);

    carregarArena(
        (ArenaId)GetRandomValue(0, TOTAL_ARENAS - 1)
    );

    anunciarTexto(
        efeitos,
        "ROUND 1",
        YELLOW,
        0.0f,
        1.1f
    );

    anunciarTexto(
        efeitos,
        "LUTEM!",
        RED,
        1.1f,
        0.8f
    );
}

// =====================================
// FUNCAO PRINCIPAL
// =====================================

int main(void) {

    InitWindow(
        LARGURA_TELA,
        ALTURA_TELA,
        "Zona de Combate"
    );

    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    carregarPersonagens();  
    EstadoJogo estado = MENU;

    int personagemJogador1 = 0;
    int personagemJogador2 = 0;

    Partida partida = {
        .roundAtual = 1,
        .vitoriasJogador1 = 0,
        .vitoriasJogador2 = 0,
        .vidaJogador1 = VIDA_MAXIMA,
        .vidaJogador2 = VIDA_MAXIMA,
        .tempoRestante = 60.0f
    };

    int vencedorPartida = 0;
    int vencedorRound = 0;

    carregarArena(ARENA_PARQUE);

    Efeito *listaEfeitos = NULL;

    float energiaJogador1 = 0.0f;
    float energiaJogador2 = 0.0f;

    Vector2 posicaoJogador1 = {
        300.0f,
        ALTURA_CHAO
    };

    Vector2 posicaoJogador2 = {
        660.0f,
        ALTURA_CHAO
    };

    const float velocidadeJogador = 220.0f;
    const float gravidade = 1200.0f;
    const float forcaPulo = -520.0f;

    bool jogador1ViradoEsquerda = false;
    bool jogador2ViradoEsquerda = true;

    EstadoAnimacaoLuna animacaoJogador1;
    EstadoAnimacaoLuna animacaoJogador2;

    iniciarAnimacaoLuna(&animacaoJogador1);
    iniciarAnimacaoLuna(&animacaoJogador2);

    float ataqueJogador1 = 0.0f;
    float ataqueJogador2 = 0.0f;

    float tempoDanoJogador1 = 0.0f;
    float tempoDanoJogador2 = 0.0f;

    float velocidadeVerticalJogador1 = 0.0f;
    float velocidadeVerticalJogador2 = 0.0f;

    bool jogador1Pulando = false;
    bool jogador2Pulando = false;

    // =====================================
    // PROJETEIS INDEPENDENTES
    // =====================================

    BolaFogo fogoJogador1 = {0};
    BolaFogo fogoJogador2 = {0};

    // =====================================
    // CONTROLE DO FIM DO ROUND
    // =====================================

    bool roundEncerrado = false;
    float tempoFimRound = 0.0f;
    float tempoAnimacaoFinal = 0.0f;

    // =====================================
    // LOOP PRINCIPAL
    // =====================================

    while (!WindowShouldClose()) {

        // =====================================
        // PAUSA
        // =====================================

        if (estado == LUTA && IsKeyPressed(KEY_P)) {
            estado = PAUSA;
        }
        else if (estado == PAUSA && IsKeyPressed(KEY_P)) {
            estado = LUTA;
        }

        // =====================================
        // MENU
        // =====================================

        if (estado == MENU) {

            if (IsKeyPressed(KEY_ONE)) {

                personagemJogador1 = 0;
                personagemJogador2 = 0;

                estado = SELECAO;
            }
            else if (IsKeyPressed(KEY_TWO)) {

                estado = RANKING;
            }
            else if (IsKeyPressed(KEY_THREE)) {

                break;
            }
        }

        // =====================================
        // SELECAO
        // =====================================

        else if (estado == SELECAO) {

            if (personagemJogador1 == 0) {

                if (IsKeyPressed(KEY_ONE)) {
                    personagemJogador1 = 1;
                }
                else if (IsKeyPressed(KEY_TWO)) {
                    personagemJogador1 = 2;
                }
                else if (IsKeyPressed(KEY_THREE)) {
                    personagemJogador1 = 3;
                }
                else if (IsKeyPressed(KEY_FOUR)) {
                    personagemJogador1 = 4;
                }
            }

            if (personagemJogador2 == 0) {

                if (IsKeyPressed(KEY_Q)) {
                    personagemJogador2 = 1;
                }
                else if (IsKeyPressed(KEY_W)) {
                    personagemJogador2 = 2;
                }
                else if (IsKeyPressed(KEY_E)) {
                    personagemJogador2 = 3;
                }
                else if (IsKeyPressed(KEY_R)) {
                    personagemJogador2 = 4;
                }
            }

            // =====================================
            // INICIAR PARTIDA
            // =====================================

            if (
                personagemJogador1 != 0 &&
                personagemJogador2 != 0 &&
                IsKeyPressed(KEY_ENTER)
            ) {

                iniciarPartida(&partida);

                vencedorPartida = 0;
                vencedorRound = 0;

                roundEncerrado = false;
                tempoFimRound = 0.0f;
                tempoAnimacaoFinal = 0.0f;

                energiaJogador1 = 0.0f;
                energiaJogador2 = 0.0f;

                reiniciarLutadores(
                    &posicaoJogador1,
                    &posicaoJogador2,
                    &animacaoJogador1,
                    &animacaoJogador2,
                    &ataqueJogador1,
                    &ataqueJogador2,
                    &tempoDanoJogador1,
                    &tempoDanoJogador2,
                    &velocidadeVerticalJogador1,
                    &velocidadeVerticalJogador2,
                    &jogador1Pulando,
                    &jogador2Pulando,
                    &fogoJogador1,
                    &fogoJogador2
                );

                jogador1ViradoEsquerda = false;
                jogador2ViradoEsquerda = true;

                prepararVisualPartida(&listaEfeitos);

                estado = LUTA;
            }
        }

        // =====================================
        // LUTA
        // =====================================

        else if (estado == LUTA) {

            float dt = GetFrameTime();

            if (dt > 0.05f) {
                dt = 0.05f;
            }

            // =====================================
            // ANIMACOES DE FIM DE ROUND
            // =====================================

            if (roundEncerrado) {

                tempoFimRound -= dt;
                tempoAnimacaoFinal += dt;

                // Os sprites possuem 6 frames.
                // Apos completar a animacao, fixa
                // o ultimo quadro da derrota.
               
                if (
                    animacaoJogador1.animacao == ANIMACAO_DERROTADA &&
                    tempoAnimacaoFinal >= 0.75f
                ) {
                    animacaoJogador1.frameAtual = 5;
                    animacaoJogador1.tempoAnimacao = 0.0f;
                }
                else {
                    atualizarAnimacaoPersonagem(
                        &animacaoJogador1,
                        dt
                    );
                }


                if (
                        animacaoJogador2.animacao == ANIMACAO_DERROTADA &&
                        tempoAnimacaoFinal >= 0.75f
                    ) {
                        animacaoJogador2.frameAtual = 5;
                        animacaoJogador2.tempoAnimacao = 0.0f;
                    }
                    else {
                        atualizarAnimacaoPersonagem(
                            &animacaoJogador2,
                            dt
                        );
                    }

                atualizarEfeitos(&listaEfeitos, dt);

                if (tempoFimRound <= 0.0f) {

                    if (vencedorRound == 1) {
                        partida.vitoriasJogador1++;
                    }
                    else if (vencedorRound == 2) {
                        partida.vitoriasJogador2++;
                    }

                    if (partida.vitoriasJogador1 >= 2) {

                        vencedorPartida = 1;

                        salvarVitoria(personagemJogador1);

                        liberarEfeitos(&listaEfeitos);

                        estado = VITORIA;
                    }
                    else if (partida.vitoriasJogador2 >= 2) {

                        vencedorPartida = 2;

                        salvarVitoria(personagemJogador2);

                        liberarEfeitos(&listaEfeitos);

                        estado = VITORIA;
                    }
                    else {

                        reiniciarRound(&partida);

                        reiniciarLutadores(
                            &posicaoJogador1,
                            &posicaoJogador2,
                            &animacaoJogador1,
                            &animacaoJogador2,
                            &ataqueJogador1,
                            &ataqueJogador2,
                            &tempoDanoJogador1,
                            &tempoDanoJogador2,
                            &velocidadeVerticalJogador1,
                            &velocidadeVerticalJogador2,
                            &jogador1Pulando,
                            &jogador2Pulando,
                            &fogoJogador1,
                            &fogoJogador2
                        );

                        jogador1ViradoEsquerda = false;
                        jogador2ViradoEsquerda = true;

                        roundEncerrado = false;
                        vencedorRound = 0;

                        tempoAnimacaoFinal = 0.0f;

                        liberarEfeitos(&listaEfeitos);

                        anunciarTexto(
                            &listaEfeitos,
                            TextFormat(
                                "ROUND %d",
                                partida.roundAtual
                            ),
                            YELLOW,
                            0.0f,
                            1.1f
                        );

                        anunciarTexto(
                            &listaEfeitos,
                            "LUTEM!",
                            RED,
                            1.1f,
                            0.8f
                        );
                    }
                }
            }

            // =====================================
            // ROUND EM ANDAMENTO
            // =====================================

            else {

                bool andando1 = false;
                bool andando2 = false;

                atualizarTempo(&ataqueJogador1, dt);
                atualizarTempo(&ataqueJogador2, dt);

                atualizarTempo(&tempoDanoJogador1, dt);
                atualizarTempo(&tempoDanoJogador2, dt);

                // =====================================
                // PULOS
                // =====================================

                if (
                    IsKeyPressed(KEY_W) &&
                    !jogador1Pulando &&
                    ataqueJogador1 <= 0.0f &&
                    tempoDanoJogador1 <= 0.0f
                ) {
                    jogador1Pulando = true;
                    velocidadeVerticalJogador1 = forcaPulo;
                }

                if (
                    IsKeyPressed(KEY_UP) &&
                    !jogador2Pulando &&
                    ataqueJogador2 <= 0.0f &&
                    tempoDanoJogador2 <= 0.0f
                ) {
                    jogador2Pulando = true;
                    velocidadeVerticalJogador2 = forcaPulo;
                }

                // =====================================
                // GRAVIDADE
                // =====================================

                if (jogador1Pulando) {

                    velocidadeVerticalJogador1 += gravidade * dt;

                    posicaoJogador1.y +=
                        velocidadeVerticalJogador1 * dt;

                    if (posicaoJogador1.y >= ALTURA_CHAO) {

                        posicaoJogador1.y = ALTURA_CHAO;
                        velocidadeVerticalJogador1 = 0.0f;
                        jogador1Pulando = false;
                    }
                }

                if (jogador2Pulando) {

                    velocidadeVerticalJogador2 += gravidade * dt;

                    posicaoJogador2.y +=
                        velocidadeVerticalJogador2 * dt;

                    if (posicaoJogador2.y >= ALTURA_CHAO) {

                        posicaoJogador2.y = ALTURA_CHAO;
                        velocidadeVerticalJogador2 = 0.0f;
                        jogador2Pulando = false;
                    }
                }

                // =====================================
                // AGACHAMENTO
                // =====================================

                bool agachando1 =
                    IsKeyDown(KEY_C) &&
                    !jogador1Pulando;

                bool agachando2 =
                    IsKeyDown(KEY_RIGHT_SHIFT) &&
                    !jogador2Pulando;

                // =====================================
                // MOVIMENTACAO
                // =====================================

                if (
                    ataqueJogador1 <= 0.0f &&
                    tempoDanoJogador1 <= 0.0f &&
                    !agachando1
                ) {

                    if (IsKeyDown(KEY_A)) {
                        posicaoJogador1.x -= velocidadeJogador * dt;
                        andando1 = true;
                    }

                    if (IsKeyDown(KEY_D)) {
                        posicaoJogador1.x += velocidadeJogador * dt;
                        andando1 = true;
                    }
                }

                if (
                    ataqueJogador2 <= 0.0f &&
                    tempoDanoJogador2 <= 0.0f &&
                    !agachando2
                ) {

                    if (IsKeyDown(KEY_LEFT)) {
                        posicaoJogador2.x -= velocidadeJogador * dt;
                        andando2 = true;
                    }

                    if (IsKeyDown(KEY_RIGHT)) {
                        posicaoJogador2.x += velocidadeJogador * dt;
                        andando2 = true;
                    }
                }

                // =====================================
                // LIMITES DA ARENA
                // =====================================

                if (posicaoJogador1.x < 65.0f) {
                    posicaoJogador1.x = 65.0f;
                }

                if (posicaoJogador1.x > LARGURA_TELA - 65.0f) {
                    posicaoJogador1.x = LARGURA_TELA - 65.0f;
                }

                if (posicaoJogador2.x < 65.0f) {
                    posicaoJogador2.x = 65.0f;
                }

                if (posicaoJogador2.x > LARGURA_TELA - 65.0f) {
                    posicaoJogador2.x = LARGURA_TELA - 65.0f;
                }

                impedirSobreposicao(
                    &posicaoJogador1,
                    &posicaoJogador2
                );

                // =====================================
                // DIRECAO AUTOMATICA
                // =====================================

                jogador1ViradoEsquerda =
                    posicaoJogador2.x < posicaoJogador1.x;

                jogador2ViradoEsquerda =
                    posicaoJogador1.x < posicaoJogador2.x;

                // =====================================
                // DEFESA
                // =====================================

                bool defendendo1 =
                    IsKeyDown(KEY_S) &&
                    !jogador1Pulando &&
                    ataqueJogador1 <= 0.0f &&
                    tempoDanoJogador1 <= 0.0f;

                bool defendendo2 =
                    IsKeyDown(KEY_DOWN) &&
                    !jogador2Pulando &&
                    ataqueJogador2 <= 0.0f &&
                    tempoDanoJogador2 <= 0.0f;
                // =====================================
                // LUNA - JOGADOR 1
                // =====================================

                                if (tempoDanoJogador1 > 0.0f) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_DANO
                    );
                }
                else if (jogador1Pulando) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_PULANDO
                    );
                }
                else if (defendendo1) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_DEFENDENDO
                    );
                }
                else if (agachando1) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_AGACHANDO
                    );
                }
                else if (ataqueJogador1 > 0.0f) {

                    // Aguarda terminar a animacao.
                }
                else if (IsKeyPressed(KEY_F)) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_SOCANDO
                    );

                    ataqueJogador1 = TEMPO_SOCO;

                    bool acertou = executarGolpe(
                        posicaoJogador1,
                        posicaoJogador2,
                        jogador1ViradoEsquerda,
                        ALCANCE_SOCO,
                        DANO_SOCO,
                        defendendo2,
                        &partida.vidaJogador2,
                        &energiaJogador1,
                        &tempoDanoJogador2,
                        &listaEfeitos
                    );

                    if (acertou && !defendendo2) {
                        ataqueJogador2 = 0.0f;
                    }
                }
                else if (IsKeyPressed(KEY_G)) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_CHUTANDO
                    );

                    ataqueJogador1 = TEMPO_CHUTE;

                    bool acertou = executarGolpe(
                        posicaoJogador1,
                        posicaoJogador2,
                        jogador1ViradoEsquerda,
                        ALCANCE_CHUTE,
                        DANO_CHUTE,
                        defendendo2,
                        &partida.vidaJogador2,
                        &energiaJogador1,
                        &tempoDanoJogador2,
                        &listaEfeitos
                    );

                    if (acertou && !defendendo2) {
                        ataqueJogador2 = 0.0f;
                    }
                }
                else if (
                    IsKeyPressed(KEY_E) &&
                    energiaJogador1 >= CUSTO_FOGO &&
                    !fogoJogador1.ativa
                ) {

                    energiaJogador1 -= CUSTO_FOGO;

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_ESPECIAL
                    );

                    ataqueJogador1 = TEMPO_ESPECIAL;

                    criarBolaFogo(
                        &fogoJogador1,
                        posicaoJogador1,
                        jogador1ViradoEsquerda,
                        1
                    );
                }
                else if (andando1) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_CAMINHANDO
                    );
                }
                else {

                    definirAnimacaoPersonagem(
                        &animacaoJogador1,
                        ANIMACAO_PARADA
                    );
                }

                atualizarAnimacaoPersonagem(
                    &animacaoJogador1,
                    dt
                );


                // =====================================
                // COMBATE - JOGADOR 2
                // =====================================

                if (tempoDanoJogador2 > 0.0f) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_DANO
                    );
                }
                else if (jogador2Pulando) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_PULANDO
                    );
                }
                else if (defendendo2) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_DEFENDENDO
                    );
                }
                else if (agachando2) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_AGACHANDO
                    );
                }
                else if (ataqueJogador2 > 0.0f) {

                    // Aguarda terminar a animacao.
                }
                else if (IsKeyPressed(KEY_K)) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_SOCANDO
                    );

                    ataqueJogador2 = TEMPO_SOCO;

                    bool acertou = executarGolpe(
                        posicaoJogador2,
                        posicaoJogador1,
                        jogador2ViradoEsquerda,
                        ALCANCE_SOCO,
                        DANO_SOCO,
                        defendendo1,
                        &partida.vidaJogador1,
                        &energiaJogador2,
                        &tempoDanoJogador1,
                        &listaEfeitos
                    );

                    if (acertou && !defendendo1) {
                        ataqueJogador1 = 0.0f;
                    }
                }
                else if (IsKeyPressed(KEY_L)) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_CHUTANDO
                    );

                    ataqueJogador2 = TEMPO_CHUTE;

                    bool acertou = executarGolpe(
                        posicaoJogador2,
                        posicaoJogador1,
                        jogador2ViradoEsquerda,
                        ALCANCE_CHUTE,
                        DANO_CHUTE,
                        defendendo1,
                        &partida.vidaJogador1,
                        &energiaJogador2,
                        &tempoDanoJogador1,
                        &listaEfeitos
                    );

                    if (acertou && !defendendo1) {
                        ataqueJogador1 = 0.0f;
                    }
                }
                else if (
                    IsKeyPressed(KEY_O) &&
                    energiaJogador2 >= CUSTO_FOGO &&
                    !fogoJogador2.ativa
                ) {

                    energiaJogador2 -= CUSTO_FOGO;

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_ESPECIAL
                    );

                    ataqueJogador2 = TEMPO_ESPECIAL;

                    criarBolaFogo(
                        &fogoJogador2,
                        posicaoJogador2,
                        jogador2ViradoEsquerda,
                        2
                    );
                }
                else if (andando2) {

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_CAMINHANDO
                    );
                }
                else {

                    definirAnimacaoPersonagem(
                        &animacaoJogador2,
                        ANIMACAO_PARADA
                    );
                }

                atualizarAnimacaoPersonagem(
                    &animacaoJogador2,
                    dt
                );

                // =====================================
                // ATUALIZAR AS BOLAS DE FOGO
                // =====================================

                atualizarBolaFogo(
                    &fogoJogador1,
                    posicaoJogador2,
                    defendendo2,
                    &partida.vidaJogador2,
                    &tempoDanoJogador2,
                    &ataqueJogador2,
                    &listaEfeitos,
                    dt
                );

                atualizarBolaFogo(
                    &fogoJogador2,
                    posicaoJogador1,
                    defendendo1,
                    &partida.vidaJogador1,
                    &tempoDanoJogador1,
                    &ataqueJogador1,
                    &listaEfeitos,
                    dt
                );

                // =====================================
                // CRONOMETRO
                // =====================================

                if (partida.tempoRestante > 0.0f) {

                    partida.tempoRestante -= dt;

                    if (partida.tempoRestante < 0.0f) {
                        partida.tempoRestante = 0.0f;
                    }
                }

                // =====================================
                // EFEITOS PROVISORIOS
                // =====================================

                Vector2 origemEfeito = {
                    posicaoJogador1.x +
                        (jogador1ViradoEsquerda
                            ? -30.0f : 30.0f),
                    posicaoJogador1.y - 70.0f
                };

                int direcao =
                    jogador1ViradoEsquerda ? -1 : 1;

                if (IsKeyPressed(KEY_ONE)) {

                    efeitoEspecial(
                        &listaEfeitos,
                        EFEITO_RAIO,
                        origemEfeito,
                        direcao
                    );
                }
                else if (IsKeyPressed(KEY_TWO)) {

                    efeitoEspecial(
                        &listaEfeitos,
                        EFEITO_FOGO,
                        origemEfeito,
                        direcao
                    );
                }
                else if (IsKeyPressed(KEY_THREE)) {

                    efeitoEspecial(
                        &listaEfeitos,
                        EFEITO_AGUA,
                        origemEfeito,
                        direcao
                    );
                }
                else if (IsKeyPressed(KEY_FOUR)) {

                    efeitoEspecial(
                        &listaEfeitos,
                        EFEITO_VENTO,
                        origemEfeito,
                        direcao
                    );
                }

                // =====================================
                // VERIFICAR FIM DO ROUND
                // =====================================

                bool acabou = false;
                bool acabouPorTempo = false;

                if (
                    partida.vidaJogador1 <= 0 &&
                    partida.vidaJogador2 <= 0
                ) {

                    vencedorRound = 0;
                    acabou = true;
                }
                else if (partida.vidaJogador1 <= 0) {

                    vencedorRound = 2;
                    acabou = true;
                }
                else if (partida.vidaJogador2 <= 0) {

                    vencedorRound = 1;
                    acabou = true;
                }
                else if (partida.tempoRestante <= 0.0f) {

                    acabou = true;
                    acabouPorTempo = true;

                    if (
                        partida.vidaJogador1 >
                        partida.vidaJogador2
                    ) {
                        vencedorRound = 1;
                    }
                    else if (
                        partida.vidaJogador2 >
                        partida.vidaJogador1
                    ) {
                        vencedorRound = 2;
                    }
                    else {
                        vencedorRound = 0;
                    }
                }

                // =====================================
                // VITORIA E DERROTA
                // =====================================

                if (acabou) {

                    roundEncerrado = true;
                    tempoFimRound = TEMPO_FIM_ROUND;
                    tempoAnimacaoFinal = 0.0f;

                    // Remove projeteis restantes
                    fogoJogador1.ativa = false;
                    fogoJogador2.ativa = false;

                    if (acabouPorTempo) {

                        anunciarTexto(
                            &listaEfeitos,
                            "TEMPO ESGOTADO",
                            ORANGE,
                            0.0f,
                            1.2f
                        );
                    }
                    else {

                        anunciarTexto(
                            &listaEfeitos,
                            "K.O.",
                            RED,
                            0.0f,
                            1.2f
                        );

                        efeitoFlash(
                            &listaEfeitos,
                            WHITE,
                            0.25f
                        );
                    }

                    // JOGADOR 1 VENCE
                    if (vencedorRound == 1) {

                        definirAnimacaoPersonagem(
                            &animacaoJogador1,
                            ANIMACAO_VITORIA
                        );

                        definirAnimacaoPersonagem(
                            &animacaoJogador2,
                            ANIMACAO_DERROTADA
                        );
                    }

                    // JOGADOR 2 VENCE
                    else if (vencedorRound == 2) {

                        definirAnimacaoPersonagem(
                            &animacaoJogador2,
                            ANIMACAO_VITORIA
                        );

                        definirAnimacaoPersonagem(
                            &animacaoJogador1,
                            ANIMACAO_DERROTADA
                        );
                    }

                    // EMPATE
                    else {

                        definirAnimacaoPersonagem(
                            &animacaoJogador1,
                            ANIMACAO_PARADA
                        );

                        definirAnimacaoPersonagem(
                            &animacaoJogador2,
                            ANIMACAO_PARADA
                        );
                    }
                }

                atualizarHUD(&partida, dt);
                atualizarEfeitos(&listaEfeitos, dt);
            }
        }
        // =====================================
        // VOLTAR AO MENU
        // =====================================

        if (
            estado != MENU &&
            IsKeyPressed(KEY_ESCAPE)
        ) {

            liberarEfeitos(&listaEfeitos);

            fogoJogador1.ativa = false;
            fogoJogador2.ativa = false;

            roundEncerrado = false;

            estado = MENU;
        }

        // =====================================
        // DESENHAR
        // =====================================

        BeginDrawing();
        ClearBackground(BLACK);

        if (estado == MENU) {

            desenharMenu();
        }
        else if (estado == SELECAO) {

            desenharSelecao(
                personagemJogador1,
                personagemJogador2
            );
        }
        else if (
            estado == LUTA ||
            estado == PAUSA
        ) {

            desenharArena();

            Color cor1 = corPersonagem(personagemJogador1);
            Color cor2 = corPersonagem(personagemJogador2);

            // =====================================
            // JOGADOR 1
            // =====================================

            desenharPersonagem(
                obterIndicePersonagem(personagemJogador1),
                posicaoJogador1,
                jogador1ViradoEsquerda,
                &animacaoJogador1
            );


                        // =====================================
                        // JOGADOR 2
                        // =====================================


                        desenharPersonagem(
                            obterIndicePersonagem(personagemJogador2),
                            posicaoJogador2,
                            jogador2ViradoEsquerda,
                            &animacaoJogador2
                        );

            // =====================================
            // DESENHAR PROJETEIS
            // =====================================

            desenharBolaFogo(&fogoJogador1);
            desenharBolaFogo(&fogoJogador2);

            // =====================================
            // EFEITOS E HUD
            // =====================================

            desenharEfeitos(listaEfeitos);

            
            desenharHUD(
                &partida,
                obterIndicePersonagem(personagemJogador1),
                obterIndicePersonagem(personagemJogador2),
                energiaJogador1,
                energiaJogador2
            );


            DrawText(
                "J1: A/D W F/G S C E | J2: SETAS K/L SHIFT O | P: PAUSA",
                32,
                512,
                15,
                Fade(WHITE, 0.85f)
            );

            // =====================================
            // PAUSA
            // =====================================

            if (estado == PAUSA) {

                DrawRectangle(
                    0,
                    0,
                    GetScreenWidth(),
                    GetScreenHeight(),
                    Fade(BLACK, 0.75f)
                );

                const char *titulo = "JOGO PAUSADO";

                DrawText(
                    titulo,
                    (
                        GetScreenWidth() -
                        MeasureText(titulo, 40)
                    ) / 2,
                    190,
                    40,
                    YELLOW
                );

                const char *continuar = "P - CONTINUAR";

                DrawText(
                    continuar,
                    (
                        GetScreenWidth() -
                        MeasureText(continuar, 25)
                    ) / 2,
                    290,
                    25,
                    WHITE
                );

                const char *voltar =
                    "ESC - VOLTAR AO MENU";

                DrawText(
                    voltar,
                    (
                        GetScreenWidth() -
                        MeasureText(voltar, 22)
                    ) / 2,
                    350,
                    22,
                    LIGHTGRAY
                );
            }
        }
        else if (estado == VITORIA) {

            desenharVitoria(
                vencedorPartida,
                partida.vitoriasJogador1,
                partida.vitoriasJogador2
            );
        }
        else if (estado == RANKING) {

            desenharRanking();
        }

        EndDrawing();
    }

    // =====================================
    // LIBERAR RECURSOS
    // =====================================

    liberarEfeitos(&listaEfeitos);
    descarregarPersonagens();

    CloseWindow();

    return 0;
}
