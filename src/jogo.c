#include "raylib.h"
#include "jogo.h"
#include <stdio.h>

void desenharMenu(void) {

    DrawText("ZONA DE COMBATE", 280, 100, 40, WHITE);

    DrawText("1 - JOGAR", 390, 230, 30, WHITE);

    DrawText("2 - RANKING", 390, 300, 30, WHITE);

    DrawText("3 - SAIR", 390, 370, 30, WHITE);
}

// FUNCAO PARA SALVAR UMA VITORIA
void salvarVitoria(int personagemVencedor) {

    FILE *arquivo = fopen("dados/ranking.txt", "a");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de ranking!\n");
        return;
    }

    fprintf(arquivo, "%d\n", personagemVencedor);

    fclose(arquivo);
}
// FUNCAO PARA MOSTRAR O RANKING

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

    // ORDENAR DO MAIOR PARA O MENOR
    for (int i = 0; i < 3; i++) {

        for (int j = 0; j < 3 - i; j++) {

            if (ranking[j].vitorias < ranking[j + 1].vitorias) {

                RegistroRanking auxiliar = ranking[j];

                ranking[j] = ranking[j + 1];

                ranking[j + 1] = auxiliar;
            }
        }
    }

    // DESENHAR O RANKING
    DrawText("RANKING DE VITORIAS", 270, 70, 35, YELLOW);

    for (int i = 0; i < 4; i++) {

        DrawText(
            TextFormat("%d - %s: %d vitorias",
                       i + 1,
                       ranking[i].nome,
                       ranking[i].vitorias),
            300, 170 + i * 55, 25, WHITE
        );
    }

    DrawText("PRESSIONE ESC PARA VOLTAR",
             290, 430, 20, LIGHTGRAY);
}


    