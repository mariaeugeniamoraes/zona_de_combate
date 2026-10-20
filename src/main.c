
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

            // INICIAR A LUTA COM ENTER
            if (personagemJogador1 != 0 && personagemJogador2 != 0) {

                if (IsKeyPressed(KEY_ENTER)) {
                    tempoRestante = 60.0f;
                    estado = LUTA;
                }
                 
            }
        }

        // VOLTAR AO MENU COM ESC
        if (estado != MENU && IsKeyPressed(KEY_ESCAPE)) {
            estado = MENU;
        }

        if (estado == LUTA && tempoRestante > 0) {
            tempoRestante -= GetFrameTime();
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

            DrawText("ESCOLHA SEU PERSONAGEM", 200, 50, 30, WHITE);

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

            // MOSTRAR ESCOLHA DO JOGADOR 1
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

            // MOSTRAR ESCOLHA DO JOGADOR 2
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

            // VERIFICAR SE OS DOIS JOGADORES ESCOLHERAM
            if (personagemJogador1 != 0 && personagemJogador2 != 0) {

                DrawText("OS DOIS JOGADORES ESTAO PRONTOS!",
                         230, 450, 22, WHITE);

                DrawText("PRESSIONE ENTER PARA LUTAR",
                         290, 490, 20, GREEN);
            }
        }

        // TELA PROVISORIA DE LUTA
        else if (estado == LUTA) {

            DrawText("ZONA DE COMBATE - LUTA",
                     220, 100, 30, WHITE);

            DrawText("PREPARAR PARA LUTAR!",
                     270, 250, 25, YELLOW);

            DrawText("PRESSIONE ESC PARA VOLTAR",
                     290, 400, 20, LIGHTGRAY);
            DrawText(
                TextFormat("TEMPO: %d", (int)tempoRestante),
                400, 30, 30, WHITE
            );
        }

        // TELA DE RANKING
        else if (estado == RANKING) {

            DrawText("RANKING", 390, 200, 40, WHITE);

            DrawText("PRESSIONE ESC PARA VOLTAR",
                     290, 400, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
