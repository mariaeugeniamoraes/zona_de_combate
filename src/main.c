#include "raylib.h"
#include "jogo.h"
#include "cenarios.h"
#include "interface.h"
#include "efeitos.h"
#include <stddef.h>

// =====================================
// PREPARAR VISUAL DE UMA NOVA PARTIDA
// =====================================
static void prepararVisualPartida(Efeito **listaEfeitos) {

    liberarEfeitos(listaEfeitos);

    // ARENA SORTEADA A CADA PARTIDA
    carregarArena((ArenaId)GetRandomValue(0, TOTAL_ARENAS - 1));

    anunciarTexto(listaEfeitos, "ROUND 1", YELLOW, 0.0f, 1.1f);
    anunciarTexto(listaEfeitos, "LUTEM!", RED, 1.1f, 0.8f);
}

int main(void) {

    // CONFIGURACAO DA JANELA
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Zona de Combate");

    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    // ESTADO INICIAL
    EstadoJogo estado = MENU;

    // PERSONAGENS SELECIONADOS
    int personagemJogador1 = 0;
    int personagemJogador2 = 0;

    // DADOS DA PARTIDA
    Partida partida = {
        .roundAtual = 1,
        .vitoriasJogador1 = 0,
        .vitoriasJogador2 = 0,
        .vidaJogador1 = 100,
        .vidaJogador2 = 100,
        .tempoRestante = 60.0f
    };

    int vencedorPartida = 0;

    // ARENA E EFEITOS VISUAIS
    carregarArena(ARENA_PARQUE);

    Efeito *listaEfeitos = NULL;

    // ENERGIA PROVISORIA (sera substituida pela energia dos lutadores - parte 1)
    float energiaJogador1 = 0.0f;
    float energiaJogador2 = 0.0f;

    // POSICOES PROVISORIAS DOS LUTADORES (parte 1)
    Vector2 posicaoJogador1 = { 300.0f, ALTURA_CHAO };
    Vector2 posicaoJogador2 = { 660.0f, ALTURA_CHAO };

    // LOOP PRINCIPAL
    while (!WindowShouldClose()) {

        // =====================================
        // MENU PRINCIPAL
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
        // SELECAO DE PERSONAGENS
        // =====================================
        else if (estado == SELECAO) {

            // JOGADOR 1
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

            // JOGADOR 2
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

            // INICIAR A PARTIDA
            if (personagemJogador1 != 0 &&
                personagemJogador2 != 0 &&
                IsKeyPressed(KEY_ENTER)) {

                iniciarPartida(&partida);

                vencedorPartida = 0;

                energiaJogador1 = 0.0f;
                energiaJogador2 = 0.0f;

                prepararVisualPartida(&listaEfeitos);

                estado = LUTA;
            }
        }

        // =====================================
        // SISTEMA DE LUTA
        // =====================================
        else if (estado == LUTA) {

            float dt = GetFrameTime();

            // ATUALIZAR O CRONOMETRO
            if (partida.tempoRestante > 0.0f) {

                partida.tempoRestante -= dt;

                if (partida.tempoRestante < 0.0f) {
                    partida.tempoRestante = 0.0f;
                }
            }

            // DANO PROVISORIO NO JOGADOR 1
            if (IsKeyPressed(KEY_A) &&
                partida.vidaJogador1 > 0) {

                partida.vidaJogador1 -= 10;

                efeitoImpacto(&listaEfeitos, (Vector2){ posicaoJogador1.x, posicaoJogador1.y - 70.0f });

                energiaJogador2 += 20.0f;
            }

            // DANO PROVISORIO NO JOGADOR 2
            if (IsKeyPressed(KEY_L) &&
                partida.vidaJogador2 > 0) {

                partida.vidaJogador2 -= 10;

                efeitoImpacto(&listaEfeitos, (Vector2){ posicaoJogador2.x, posicaoJogador2.y - 70.0f });

                energiaJogador1 += 20.0f;
            }

            if (energiaJogador1 > 100.0f) energiaJogador1 = 100.0f;
            if (energiaJogador2 > 100.0f) energiaJogador2 = 100.0f;

            // TESTE PROVISORIO DOS ESPECIAIS (J1 atacando o J2)
            Vector2 maoJogador1 = { posicaoJogador1.x + 30.0f, posicaoJogador1.y - 70.0f };

            if (IsKeyPressed(KEY_ONE)) {
                efeitoEspecial(&listaEfeitos, EFEITO_RAIO, maoJogador1, 1);
            }
            else if (IsKeyPressed(KEY_TWO)) {
                efeitoEspecial(&listaEfeitos, EFEITO_FOGO, maoJogador1, 1);
            }
            else if (IsKeyPressed(KEY_THREE)) {
                efeitoEspecial(&listaEfeitos, EFEITO_AGUA, maoJogador1, 1);
            }
            else if (IsKeyPressed(KEY_FOUR)) {
                efeitoEspecial(&listaEfeitos, EFEITO_VENTO, maoJogador1, 1);
            }

            // VERIFICAR FIM DO ROUND
            int roundFinalizado = 0;
            int fimPorTempo = 0;

            // JOGADOR 1 PERDEU TODA A VIDA
            if (partida.vidaJogador1 <= 0) {

                partida.vitoriasJogador2++;

                roundFinalizado = 1;
            }

            // JOGADOR 2 PERDEU TODA A VIDA
            else if (partida.vidaJogador2 <= 0) {

                partida.vitoriasJogador1++;

                roundFinalizado = 1;
            }

            // TEMPO ESGOTADO
            else if (partida.tempoRestante <= 0.0f) {

                if (partida.vidaJogador1 >
                    partida.vidaJogador2) {

                    partida.vitoriasJogador1++;
                }
                else if (partida.vidaJogador2 >
                         partida.vidaJogador1) {

                    partida.vitoriasJogador2++;
                }

                // EMPATE: NINGUEM PONTUA
                roundFinalizado = 1;
                fimPorTempo = 1;
            }

            // ANUNCIO DO FIM DO ROUND
            if (roundFinalizado == 1) {

                if (fimPorTempo == 1) {
                    anunciarTexto(&listaEfeitos, "TEMPO ESGOTADO", ORANGE, 0.0f, 1.3f);
                }
                else {
                    anunciarTexto(&listaEfeitos, "K.O.", RED, 0.0f, 1.3f);
                    efeitoFlash(&listaEfeitos, WHITE, 0.25f);
                }
            }

            // =====================================
            // VERIFICAR VENCEDOR DA PARTIDA
            // =====================================

            // JOGADOR 1 VENCEU DOIS ROUNDS
            if (partida.vitoriasJogador1 >= 2) {

                vencedorPartida = 1;

                salvarVitoria(personagemJogador1);

                liberarEfeitos(&listaEfeitos);

                estado = VITORIA;
            }

            // JOGADOR 2 VENCEU DOIS ROUNDS
            else if (partida.vitoriasJogador2 >= 2) {

                vencedorPartida = 2;

                salvarVitoria(personagemJogador2);

                liberarEfeitos(&listaEfeitos);

                estado = VITORIA;
            }

            // PROXIMO ROUND
            else if (roundFinalizado == 1) {

                reiniciarRound(&partida);

                anunciarTexto(
                    &listaEfeitos,
                    TextFormat("ROUND %d", partida.roundAtual),
                    YELLOW, 1.3f, 1.1f
                );

                anunciarTexto(&listaEfeitos, "LUTEM!", RED, 2.4f, 0.8f);
            }

            // ANIMACOES DO HUD E DOS EFEITOS
            atualizarHUD(&partida, dt);
            atualizarEfeitos(&listaEfeitos, dt);
        }

        // =====================================
        // VOLTAR AO MENU
        // =====================================
        if (estado != MENU && IsKeyPressed(KEY_ESCAPE)) {

            liberarEfeitos(&listaEfeitos);

            estado = MENU;
        }

        // =====================================
        // DESENHAR AS TELAS
        // =====================================
        BeginDrawing();

        ClearBackground(BLACK);

        // MENU
        if (estado == MENU) {

            desenharMenu();
        }

        // SELECAO
        else if (estado == SELECAO) {

            desenharSelecao(
                personagemJogador1,
                personagemJogador2
            );
        }

        // =====================================
        // TELA DE LUTA
        // =====================================
        else if (estado == LUTA) {

            // CENARIO
            desenharArena();

            // LUTADORES PROVISORIOS (serao substituidos pela parte 1)
            Color cor1 = corPersonagem(personagemJogador1);
            Color cor2 = corPersonagem(personagemJogador2);

            DrawRectangle((int)posicaoJogador1.x - 25, (int)posicaoJogador1.y - 110, 50, 110, Fade(cor1, 0.8f));
            DrawRectangleLines((int)posicaoJogador1.x - 25, (int)posicaoJogador1.y - 110, 50, 110, BLACK);

            DrawRectangle((int)posicaoJogador2.x - 25, (int)posicaoJogador2.y - 110, 50, 110, Fade(cor2, 0.8f));
            DrawRectangleLines((int)posicaoJogador2.x - 25, (int)posicaoJogador2.y - 110, 50, 110, BLACK);

            // EFEITOS VISUAIS
            desenharEfeitos(listaEfeitos);

            // HUD
            desenharHUD(
                &partida,
                personagemJogador1,
                personagemJogador2,
                energiaJogador1,
                energiaJogador2
            );

            // CONTROLES PROVISORIOS
            DrawText(
                "A/L - DANO NO J1/J2   1-4 - TESTAR ESPECIAIS   ESC - MENU",
                170, 512, 18, Fade(WHITE, 0.85f)
            );
        }

        // =====================================
        // TELA DE VITORIA
        // =====================================
        else if (estado == VITORIA) {

            desenharVitoria(
                vencedorPartida,
                partida.vitoriasJogador1,
                partida.vitoriasJogador2
            );
        }

        // =====================================
        // TELA DE RANKING
        // =====================================
        else if (estado == RANKING) {

            desenharRanking();
        }

        EndDrawing();
    }

    liberarEfeitos(&listaEfeitos);

    CloseWindow();

    return 0;
}