
#include "raylib.h"
#include "jogo.h"
#include <stdio.h>

// =====================================
// DESENHAR MENU PRINCIPAL
// =====================================
void desenharMenu(void) {

    DrawText("ZONA DE COMBATE", 280, 100, 40, WHITE);

    DrawText("1 - JOGAR", 390, 230, 30, WHITE);
    DrawText("2 - RANKING", 390, 300, 30, WHITE);
    DrawText("3 - SAIR", 390, 370, 30, WHITE);
}

// =====================================
// INICIAR UMA NOVA PARTIDA
// =====================================
void iniciarPartida(Partida *partida) {

    partida->roundAtual = 1;

    partida->vitoriasJogador1 = 0;
    partida->vitoriasJogador2 = 0;

    partida->vidaJogador1 = 100;
    partida->vidaJogador2 = 100;

    partida->tempoRestante = 60.0f;
}

// =====================================
// REINICIAR O ROUND
// =====================================
void reiniciarRound(Partida *partida) {

    partida->roundAtual++;

    partida->vidaJogador1 = 100;
    partida->vidaJogador2 = 100;

    partida->tempoRestante = 60.0f;
}

// =====================================
// SALVAR VITORIA NO RANKING
// =====================================
void salvarVitoria(int personagemVencedor) {

    FILE *arquivo = fopen("dados/ranking.txt", "a");

    if (arquivo == NULL) {
        printf("Erro ao abrir o ranking!\n");
        return;
    }

    fprintf(arquivo, "%d\n", personagemVencedor);

    fclose(arquivo);
}

// =====================================
// DESENHAR RANKING
// =====================================
void desenharRanking(void) {

    RegistroRanking ranking[4] = {
        {"KAI", 0},
        {"LUNA", 0},
        {"NERO", 0},
        {"AYRA", 0}
    };

    int personagem;

    FILE *arquivo = fopen("dados/ranking.txt", "r");

    if (arquivo != NULL) {

        while (fscanf(arquivo, "%d", &personagem) == 1) {

            if (personagem >= 1 && personagem <= 4) {
                ranking[personagem - 1].vitorias++;
            }
        }

        fclose(arquivo);
    }

    // ORDENAR RANKING POR VITORIAS
    for (int i = 0; i < 3; i++) {

        for (int j = 0; j < 3 - i; j++) {

            if (ranking[j].vitorias <
                ranking[j + 1].vitorias) {

                RegistroRanking auxiliar = ranking[j];

                ranking[j] = ranking[j + 1];

                ranking[j + 1] = auxiliar;
            }
        }
    }

    DrawText(
        "RANKING DE VITORIAS",
        270, 70, 35, YELLOW
    );

    for (int i = 0; i < 4; i++) {

        DrawText(
            TextFormat(
                "%d - %s: %d vitorias",
                i + 1,
                ranking[i].nome,
                ranking[i].vitorias
            ),
            300,
            170 + i * 55,
            25,
            WHITE
        );
    }

    DrawText(
        "PRESSIONE ESC PARA VOLTAR",
        290, 430, 20, LIGHTGRAY
    );
}

// =====================================
// DESENHAR TELA DE VITORIA
// =====================================
void desenharVitoria(
    int vencedorPartida,
    int vitoriasJogador1,
    int vitoriasJogador2
) {

    DrawText(
        "FIM DE PARTIDA!",
        300, 120, 35, YELLOW
    );

    if (vencedorPartida == 1) {

        DrawText(
            "JOGADOR 1 VENCEU!",
            280, 250, 30, WHITE
        );
    }
    else if (vencedorPartida == 2) {

        DrawText(
            "JOGADOR 2 VENCEU!",
            280, 250, 30, WHITE
        );
    }

    DrawText(
        TextFormat(
            "PLACAR FINAL: %d X %d",
            vitoriasJogador1,
            vitoriasJogador2
        ),
        320, 320, 25, WHITE
    );

    DrawText(
        "PRESSIONE ESC PARA VOLTAR",
        290, 400, 20, LIGHTGRAY
    );
}

// =====================================
// DESENHAR SELECAO DE PERSONAGENS
// =====================================
void desenharSelecao(
    int personagemJogador1,
    int personagemJogador2
) {

    DrawText(
        "ESCOLHA SEU PERSONAGEM",
        200, 50, 30, WHITE
    );

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

    // VERIFICAR SE AMBOS ESTAO PRONTOS
    if (personagemJogador1 != 0 &&
        personagemJogador2 != 0) {

        DrawText(
            "OS DOIS JOGADORES ESTAO PRONTOS!",
            230, 450, 22, WHITE
        );

        DrawText(
            "PRESSIONE ENTER PARA LUTAR",
            290, 490, 20, GREEN
        );
    }
}
