
#include "raylib.h"
#include "jogo.h"

int main(void) {

    // CONFIGURACAO DA JANELA
    InitWindow(960, 540, "Zona de Combate");

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

                estado = LUTA;
            }
        }

        // =====================================
        // SISTEMA DE LUTA
        // =====================================
        else if (estado == LUTA) {

            // ATUALIZAR O CRONOMETRO
            if (partida.tempoRestante > 0.0f) {

                partida.tempoRestante -= GetFrameTime();

                if (partida.tempoRestante < 0.0f) {
                    partida.tempoRestante = 0.0f;
                }
            }

            // DANO PROVISORIO NO JOGADOR 1
            if (IsKeyPressed(KEY_A) &&
                partida.vidaJogador1 > 0) {

                partida.vidaJogador1 -= 10;
            }

            // DANO PROVISORIO NO JOGADOR 2
            if (IsKeyPressed(KEY_L) &&
                partida.vidaJogador2 > 0) {

                partida.vidaJogador2 -= 10;
            }

            // VERIFICAR FIM DO ROUND
            int roundFinalizado = 0;

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
            }

            // =====================================
            // VERIFICAR VENCEDOR DA PARTIDA
            // =====================================

            // JOGADOR 1 VENCEU DOIS ROUNDS
            if (partida.vitoriasJogador1 >= 2) {

                vencedorPartida = 1;

                salvarVitoria(personagemJogador1);

                estado = VITORIA;
            }

            // JOGADOR 2 VENCEU DOIS ROUNDS
            else if (partida.vitoriasJogador2 >= 2) {

                vencedorPartida = 2;

                salvarVitoria(personagemJogador2);

                estado = VITORIA;
            }

            // PROXIMO ROUND
            else if (roundFinalizado == 1) {

                reiniciarRound(&partida);
            }
        }

        // =====================================
        // VOLTAR AO MENU
        // =====================================
        if (estado != MENU && IsKeyPressed(KEY_ESCAPE)) {

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

            // CRONOMETRO
            DrawText(
                TextFormat(
                    "TEMPO: %d",
                    (int)partida.tempoRestante
                ),
                400, 30, 30, WHITE
            );

            // ROUND
            DrawText(
                TextFormat(
                    "ROUND %d",
                    partida.roundAtual
                ),
                400, 80, 30, YELLOW
            );

            // BARRA DE VIDA DO JOGADOR 1
            DrawRectangle(
                100, 120, 250, 25, DARKGRAY
            );

            DrawRectangle(
                100,
                120,
                partida.vidaJogador1 * 250 / 100,
                25,
                GREEN
            );

            // BARRA DE VIDA DO JOGADOR 2
            DrawRectangle(
                610, 120, 250, 25, DARKGRAY
            );

            DrawRectangle(
                610,
                120,
                partida.vidaJogador2 * 250 / 100,
                25,
                GREEN
            );

            // PLACAR DO JOGADOR 1
            DrawText(
                TextFormat(
                    "J1: %d",
                    partida.vitoriasJogador1
                ),
                100, 30, 25, WHITE
            );

            // PLACAR DO JOGADOR 2
            DrawText(
                TextFormat(
                    "J2: %d",
                    partida.vitoriasJogador2
                ),
                750, 30, 25, WHITE
            );

            // TITULO
            DrawText(
                "ZONA DE COMBATE - LUTA",
                220, 160, 30, WHITE
            );

            DrawText(
                "PREPARAR PARA LUTAR!",
                270, 250, 25, YELLOW
            );

            // CONTROLES PROVISORIOS
            DrawText(
                "A - DANO NO J1",
                150, 350, 22, GREEN
            );

            DrawText(
                "L - DANO NO J2",
                600, 350, 22, GREEN
            );

            DrawText(
                "ESC - VOLTAR AO MENU",
                330, 450, 20, LIGHTGRAY
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

    CloseWindow();

    return 0;
}
