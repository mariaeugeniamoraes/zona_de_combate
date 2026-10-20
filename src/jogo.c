#include "raylib.h"
#include "jogo.h"

void desenharMenu(void) {

    DrawText("ZONA DE COMBATE", 280, 100, 40, WHITE);

    DrawText("1 - JOGAR", 390, 230, 30, WHITE);

    DrawText("2 - RANKING", 390, 300, 30, WHITE);

    DrawText("3 - SAIR", 390, 370, 30, WHITE);
}