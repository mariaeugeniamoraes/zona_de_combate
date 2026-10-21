
#include "raylib.h"
#include "jogo.h"

int main(void) {

    InitWindow(960, 540, "Zona de Combate");

    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    EstadoJogo estado = MENU;

    int personagemJogador1 = 0;
    int personagemJogador2 = 0;

    float tempoRestante = 60.0f;

    int roundAtual = 1;
    int vitoriasJogador1 = 0;
    int vitoriasJogador2 = 0;
    int vencedorPartida = 0;
    int vidaJogador1 = 100;
    int vidaJogador2 = 100;

    while (!WindowShouldClose()) {

        // MENU PRINCIPAL
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

        // SELECAO DOS PERSONAGENS
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

            // INICIAR A PARTIDA
            if (personagemJogador1 != 0 &&
                personagemJogador2 != 0) {

                if (IsKeyPressed(KEY_ENTER)) {

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

        // SISTEMA DE LUTA E ROUNDS
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

            // VARIAVEL QUE INDICA O FIM DO ROUND
            int roundFinalizado = 0;


            // VERIFICAR SE ALGUM JOGADOR FICOU SEM VIDA
            if (vidaJogador1 <= 0) {
                vitoriasJogador2++;
                roundFinalizado = 1;
            }
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

           // EM CASO DE EMPATE, NINGUEM GANHA PONTO
           roundFinalizado = 1;
        }

            // VERIFICAR O VENCEDOR DA PARTIDA
            if (vitoriasJogador1 >= 2) {
                vencedorPartida = 1;
                salvarVitoria(personagemJogador1);
                estado = VITORIA;
            }
            else if (vitoriasJogador2 >= 2) {
                     vencedorPartida = 2;
                     salvarVitoria(personagemJogador2);
                     estado = VITORIA;
            }

        // INICIAR O PROXIMO ROUND
        else if (roundFinalizado == 1) {
                 roundAtual++;
                 tempoRestante = 60.0f;
                 vidaJogador1 = 100;
                 vidaJogador2 = 100;
        }

            // VERIFICAR SE JOGADOR 1 GANHOU A PARTIDA
            if (vitoriasJogador1 == 2) {

                vencedorPartida = 1;
                estado = VITORIA;
            }

            // VERIFICAR SE JOGADOR 2 GANHOU A PARTIDA
            else if (vitoriasJogador2 == 2) {

                vencedorPartida = 2;
                estado = VITORIA;
            }

            // INICIAR O PROXIMO ROUND
            else if (roundFinalizado == 1) {

                roundAtual++;
                tempoRestante = 60.0f;
            }
        }

        // VOLTAR AO MENU COM ESC
        if (estado != MENU && IsKeyPressed(KEY_ESCAPE)) {

            estado = MENU;
        }

        // INICIO DO DESENHO
        BeginDrawing();

        ClearBackground(BLACK);

        // TELA DO MENU PRINCIPAL
        if (estado == MENU) {

            desenharMenu();
        }

        // TELA DE SELECAO DE PERSONAGENS
        else if (estado == SELECAO) {

            DrawText("ESCOLHA SEU PERSONAGEM",
                     200, 50, 30, WHITE);

            // OPCOES DO JOGADOR 1
            DrawText("JOGADOR 1", 100, 130, 25, WHITE);

            DrawText("1 - KAI (RAIO)", 100, 180, 20, YELLOW);
            DrawText("2 - LUNA (FOGO)", 100, 220, 20, RED);
            DrawText("3 - NERO (AGUA)", 100, 260, 20, BLUE);
            DrawText("4 - AYRA (VENTO)", 100, 300, 20, GREEN);

            // OPCOES DO JOGADOR 2
            DrawText("JOGADOR 2", 550, 130, 25, WHITE);

            DrawText("Q - KAI (RAIO)", 550, 180, 20, YELLOW);
            DrawText("W - LUNA (FOGO)", 550, 220, 20, RED);
            DrawText("E - NERO (AGUA)", 550, 260, 20, BLUE);
            DrawText("R - AYRA (VENTO)", 550, 300, 20, GREEN);

            // MOSTRAR PERSONAGEM DO JOGADOR 1
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

            // MOSTRAR PERSONAGEM DO JOGADOR 2
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

        // TELA DE LUTA
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
            DrawRectangle(100, 120, vidaJogador1 * 250 / 100, 25, GREEN);

            // BARRA DE VIDA DO JOGADOR 2
            DrawRectangle(610, 120, 250, 25, DARKGRAY);
            DrawRectangle(610, 120, vidaJogador2 * 250 / 100, 25, GREEN);

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

            DrawText("Z - VITORIA J1",
                     150, 350, 22, GREEN);

            DrawText("M - VITORIA J2",
                     600, 350, 22, GREEN);

            DrawText("ESC - VOLTAR AO MENU",
                     330, 450, 20, LIGHTGRAY);
        }

        // TELA DE VITORIA
        else if (estado == VITORIA) {

            DrawText("FIM DE PARTIDA!",
                     300, 120, 35, YELLOW);

            if (vencedorPartida == 1) {

                DrawText("JOGADOR 1 VENCEU!",
                         280, 250, 30, WHITE);
            }
            else if (vencedorPartida == 2) {

                DrawText("JOGADOR 2 VENCEU!",
                         280, 250, 30, WHITE);
            }

            DrawText(
                TextFormat("PLACAR FINAL: %d X %d",
                           vitoriasJogador1,
                           vitoriasJogador2),
                320, 320, 25, WHITE
            );

            DrawText("PRESSIONE ESC PARA VOLTAR",
                     290, 400, 20, LIGHTGRAY);
        }

        // TELA DE RANKING
        else if (estado == RANKING) {
            desenharRanking();
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
