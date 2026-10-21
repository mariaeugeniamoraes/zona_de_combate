
#include "raylib.h"
#include "jogo.h"

int main(void) {

    InitWindow(960, 540, "Zona de Combate");

    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    EstadoJogo estado = MENU;

    // PERSONAGENS SELECIONADOS
    int personagemJogador1 = 0;
    int personagemJogador2 = 0;

    // CRONOMETRO
    float tempoRestante = 60.0f;

    // SISTEMA DE ROUNDS
    int roundAtual = 1;
    int vitoriasJogador1 = 0;
    int vitoriasJogador2 = 0;
    int vencedorPartida = 0;

    // VIDA DOS JOGADORES
    int vidaJogador1 = 100;
    int vidaJogador2 = 100;

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
        // SELECAO DOS PERSONAGENS
        // =====================================
        else if (estado == SELECAO) {

            // JOGADOR 1: TECLAS 1, 2, 3 E 4
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

            // JOGADOR 2: TECLAS Q, W, E E R
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

            // INICIAR A PARTIDA COM ENTER
            if (personagemJogador1 != 0 &&
                personagemJogador2 != 0) {

                if (IsKeyPressed(KEY_ENTER)) {

                    // REINICIAR OS VALORES
                    tempoRestante = 60.0f;

                    roundAtual = 1;

                    vitoriasJogador1 = 0;
                    vitoriasJogador2 = 0;

                    vencedorPartida = 0;

                    vidaJogador1 = 100;
                    vidaJogador2 = 100;

                    estado = LUTA;
                }
            }
        }

        // =====================================
        // SISTEMA DE LUTA E ROUNDS
        // =====================================
        else if (estado == LUTA) {

            // ATUALIZAR O CRONOMETRO
            if (tempoRestante > 0.0f) {

                tempoRestante -= GetFrameTime();

                if (tempoRestante < 0.0f) {
                    tempoRestante = 0.0f;
                }
            }

            // TESTE DE DANO NO JOGADOR 1
            if (IsKeyPressed(KEY_A) && vidaJogador1 > 0) {

                vidaJogador1 -= 10;
            }

            // TESTE DE DANO NO JOGADOR 2
            if (IsKeyPressed(KEY_L) && vidaJogador2 > 0) {

                vidaJogador2 -= 10;
            }

            // VERIFICAR SE O ROUND TERMINOU
            int roundFinalizado = 0;

            // JOGADOR 1 FICOU SEM VIDA
            if (vidaJogador1 <= 0) {

                vitoriasJogador2++;
                roundFinalizado = 1;
            }

            // JOGADOR 2 FICOU SEM VIDA
            else if (vidaJogador2 <= 0) {

                vitoriasJogador1++;
                roundFinalizado = 1;
            }

            // VERIFICAR SE O TEMPO ACABOU
            else if (tempoRestante <= 0.0f) {

                if (vidaJogador1 > vidaJogador2) {

                    vitoriasJogador1++;
                }
                else if (vidaJogador2 > vidaJogador1) {

                    vitoriasJogador2++;
                }

                // EMPATE: NINGUEM GANHA PONTO
                roundFinalizado = 1;
            }

            // VERIFICAR SE JOGADOR 1 VENCEU A PARTIDA
            if (vitoriasJogador1 >= 2) {

                vencedorPartida = 1;

                // SALVAR VITORIA NO RANKING
                salvarVitoria(personagemJogador1);

                estado = VITORIA;
            }

            // VERIFICAR SE JOGADOR 2 VENCEU A PARTIDA
            else if (vitoriasJogador2 >= 2) {

                vencedorPartida = 2;

                // SALVAR VITORIA NO RANKING
                salvarVitoria(personagemJogador2);

                estado = VITORIA;
            }

            // INICIAR O PROXIMO ROUND
            else if (roundFinalizado == 1) {

                // AVANCAR APENAS UM ROUND
                roundAtual++;

                // REINICIAR CRONOMETRO E VIDA
                tempoRestante = 60.0f;

                vidaJogador1 = 100;
                vidaJogador2 = 100;
            }
        }

        // =====================================
        // VOLTAR AO MENU COM ESC
        // =====================================
        if (estado != MENU && IsKeyPressed(KEY_ESCAPE)) {

            estado = MENU;
        }

        // =====================================
        // INICIO DO DESENHO
        // =====================================
        BeginDrawing();

        ClearBackground(BLACK);

        // TELA DO MENU PRINCIPAL
        if (estado == MENU) {

            desenharMenu();
        }

        // TELA DE SELECAO DE PERSONAGENS
        else if (estado == SELECAO) {

            desenharSelecao(personagemJogador1, personagemJogador2);
        }
            // PERSONAGEM ESCOLHIDO PELO JOGADOR 1
            if (personagemJogador1 == 1) {

                DrawText("J1: KAI", 100, 390, 25, YELLOW);
            }
            else if (personagemJogador1 == 2) {

                DrawText("J1: LUNA", 100, 390, 25, RED);
            }
            else if (personagemJogador1 == 3) {

                DrawText("J1: NERO", 100, 390, 25, BLUE);
            }
            else if (personagemJogador1 == 4) {

                DrawText("J1: AYRA", 100, 390, 25, GREEN);
            }

            // PERSONAGEM ESCOLHIDO PELO JOGADOR 2
            if (personagemJogador2 == 1) {

                DrawText("J2: KAI", 550, 390, 25, YELLOW);
            }
            else if (personagemJogador2 == 2) {

                DrawText("J2: LUNA", 550, 390, 25, RED);
            }
            else if (personagemJogador2 == 3) {

                DrawText("J2: NERO", 550, 390, 25, BLUE);
            }
            else if (personagemJogador2 == 4) {

                DrawText("J2: AYRA", 550, 390, 25, GREEN);
            }

            // VERIFICAR SE OS DOIS ESTAO PRONTOS
            if (personagemJogador1 != 0 &&
                personagemJogador2 != 0) {

                DrawText("OS DOIS JOGADORES ESTAO PRONTOS!",
                         230, 450, 22, WHITE);

                DrawText("PRESSIONE ENTER PARA LUTAR",
                         290, 490, 20, GREEN);
            }
        }

        // =====================================
        // TELA DE LUTA
        // =====================================
        else if (estado == LUTA) {

            // CRONOMETRO
            DrawText(
                TextFormat("TEMPO: %d", (int)tempoRestante),
                400, 30, 30, WHITE
            );

            // ROUND ATUAL
            DrawText(
                TextFormat("ROUND %d", roundAtual),
                400, 80, 30, YELLOW
            );

            // BARRA DE VIDA DO JOGADOR 1
            DrawRectangle(100, 120, 250, 25, DARKGRAY);

            DrawRectangle(
                100, 120,
                vidaJogador1 * 250 / 100,
                25, GREEN
            );

            // BARRA DE VIDA DO JOGADOR 2
            DrawRectangle(610, 120, 250, 25, DARKGRAY);

            DrawRectangle(
                610, 120,
                vidaJogador2 * 250 / 100,
                25, GREEN
            );

            // PLACAR DO JOGADOR 1
            DrawText(
                TextFormat("J1: %d", vitoriasJogador1),
                100, 30, 25, WHITE
            );

            // PLACAR DO JOGADOR 2
            DrawText(
                TextFormat("J2: %d", vitoriasJogador2),
                750, 30, 25, WHITE
            );

            DrawText("ZONA DE COMBATE - LUTA",
                     220, 160, 30, WHITE);

            DrawText("PREPARAR PARA LUTAR!",
                     270, 250, 25, YELLOW);

            // CONTROLES PROVISORIOS DE DANO
            DrawText("A - DANO NO J1",
                     150, 350, 22, GREEN);

            DrawText("L - DANO NO J2",
                     600, 350, 22, GREEN);

            DrawText("ESC - VOLTAR AO MENU",
                     330, 450, 20, LIGHTGRAY);
        }

        // =====================================
        // TELA DE VITORIA
        // =====================================
        else if (estado == VITORIA) {

            desenharVitoria(
                vencedorPartida,
                vitoriasJogador1,
                vitoriasJogador2
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
