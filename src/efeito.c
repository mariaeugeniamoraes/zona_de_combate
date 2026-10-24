#include "efeitos.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// =====================================
// FUNCOES AUXILIARES
// =====================================
static int ruido(int a, int b) {

    unsigned int h = (unsigned int)a * 73856093u ^ (unsigned int)b * 19349663u;

    h ^= h >> 13;
    h *= 0x5bd1e995u;
    h ^= h >> 15;

    return (int)(h & 0x7fffffff);
}

static Color misturarCores(Color a, Color b, float t) {

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

// =====================================
// CRIAR UM NO E INSERIR NO FIM DA LISTA
// =====================================
static Efeito *criarEfeito(TipoEfeito tipo, Vector2 posicao, float duracao, float atraso, Color cor) {

    Efeito *novo = (Efeito *)malloc(sizeof(Efeito));

    if (novo == NULL) {
        return NULL;
    }

    novo->tipo = tipo;
    novo->posicao = posicao;
    novo->duracao = duracao;
    novo->atraso = atraso;
    novo->tempoVivo = 0.0f;
    novo->direcao = 1;
    novo->semente = GetRandomValue(0, 100000);
    novo->cor = cor;
    novo->texto[0] = '\0';
    novo->proximo = NULL;

    return novo;
}

static void inserirNoFim(Efeito **lista, Efeito *novo) {

    if (novo == NULL) {
        return;
    }

    // PERCORRE A LISTA COM PONTEIRO PARA PONTEIRO ATE O ULTIMO "proximo"
    Efeito **atual = lista;

    while (*atual != NULL) {
        atual = &(*atual)->proximo;
    }

    *atual = novo;
}

// =====================================
// FUNCOES DE CRIACAO
// =====================================
void anunciarTexto(Efeito **lista, const char *texto, Color cor, float atraso, float duracao) {

    Vector2 centro = { GetScreenWidth() / 2.0f, GetScreenHeight() * 0.40f };
    Efeito *novo = criarEfeito(EFEITO_TEXTO, centro, duracao, atraso, cor);

    if (novo == NULL) {
        return;
    }

    strncpy(novo->texto, texto, sizeof(novo->texto) - 1);
    novo->texto[sizeof(novo->texto) - 1] = '\0';

    inserirNoFim(lista, novo);
}

void efeitoImpacto(Efeito **lista, Vector2 posicao) {

    inserirNoFim(lista, criarEfeito(EFEITO_FAISCA, posicao, 0.3f, 0.0f, (Color){ 255, 230, 120, 255 }));
}

void efeitoFlash(Efeito **lista, Color cor, float duracao) {

    inserirNoFim(lista, criarEfeito(EFEITO_FLASH, (Vector2){ 0.0f, 0.0f }, duracao, 0.0f, cor));
}

void efeitoEspecial(Efeito **lista, TipoEfeito elemento, Vector2 posicao, int direcao) {

    float duracao = 0.7f;
    Color cor = WHITE;

    switch (elemento) {
        case EFEITO_RAIO:  duracao = 0.6f; cor = (Color){ 200, 235, 255, 255 }; break;
        case EFEITO_FOGO:  duracao = 0.7f; cor = (Color){ 255, 140, 40, 255 };  break;
        case EFEITO_AGUA:  duracao = 0.8f; cor = (Color){ 80, 170, 255, 255 };  break;
        case EFEITO_VENTO: duracao = 0.7f; cor = (Color){ 90, 225, 190, 255 };  break;
        default: return;
    }

    Efeito *novo = criarEfeito(elemento, posicao, duracao, 0.0f, cor);

    if (novo == NULL) {
        return;
    }

    novo->direcao = (direcao < 0) ? -1 : 1;

    inserirNoFim(lista, novo);
}

// =====================================
// ATUALIZAR: AVANCA O TEMPO E REMOVE OS QUE ACABARAM
// =====================================
void atualizarEfeitos(Efeito **lista, float dt) {

    Efeito **atual = lista;

    while (*atual != NULL) {

        Efeito *efeito = *atual;

        // AINDA ESPERANDO PARA APARECER
        if (efeito->atraso > 0.0f) {

            efeito->atraso -= dt;
            atual = &efeito->proximo;
            continue;
        }

        efeito->tempoVivo += dt;

        // TERMINOU: TIRA DA LISTA E LIBERA A MEMORIA
        if (efeito->tempoVivo >= efeito->duracao) {

            *atual = efeito->proximo;
            free(efeito);
        }
        else {
            atual = &efeito->proximo;
        }
    }
}

// =====================================
// DESENHO DE CADA TIPO
// =====================================
static void desenharTexto(const Efeito *efeito) {

    float t = efeito->tempoVivo;
    float restante = efeito->duracao - t;

    // ENTRA CRESCENDO E SAI SUMINDO
    float escala = (t < 0.15f) ? 0.4f + 0.6f * (t / 0.15f) : 1.0f;
    float alfa = (restante < 0.3f) ? restante / 0.3f : 1.0f;

    int tamanho = (int)(72 * escala);
    int largura = MeasureText(efeito->texto, tamanho);
    int x = (int)efeito->posicao.x - largura / 2;
    int y = (int)efeito->posicao.y - tamanho / 2;

    Color contorno = Fade(BLACK, alfa);

    DrawText(efeito->texto, x - 3, y - 3, tamanho, contorno);
    DrawText(efeito->texto, x + 3, y - 3, tamanho, contorno);
    DrawText(efeito->texto, x - 3, y + 3, tamanho, contorno);
    DrawText(efeito->texto, x + 3, y + 3, tamanho, contorno);
    DrawText(efeito->texto, x, y, tamanho, Fade(efeito->cor, alfa));
}

static void desenharFaisca(const Efeito *efeito, float progresso) {

    float alfa = 1.0f - progresso;
    float raioInterno = 6.0f + 28.0f * progresso;
    float raioExterno = raioInterno + 22.0f * (1.0f - progresso) + 6.0f;

    for (int i = 0; i < 10; i++) {

        float angulo = (i * 36.0f + (float)(ruido(efeito->semente, i) % 20)) * DEG2RAD;

        Vector2 inicio = {
            efeito->posicao.x + cosf(angulo) * raioInterno,
            efeito->posicao.y + sinf(angulo) * raioInterno
        };

        Vector2 fim = {
            efeito->posicao.x + cosf(angulo) * raioExterno,
            efeito->posicao.y + sinf(angulo) * raioExterno
        };

        DrawLineEx(inicio, fim, 3, Fade(efeito->cor, alfa));
    }

    if (progresso < 0.4f) {
        DrawCircleV(efeito->posicao, 16.0f * (1.0f - progresso / 0.4f), Fade(WHITE, alfa));
    }
}

static void desenharFlash(const Efeito *efeito, float progresso) {

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(efeito->cor, 0.6f * (1.0f - progresso)));
}

static void desenharRaio(const Efeito *efeito, float progresso) {

    float alfa = (progresso < 0.7f) ? 1.0f : (1.0f - progresso) / 0.3f;
    float comprimento = 340.0f * fminf(1.0f, progresso * 4.0f);
    int segmentos = (int)(comprimento / 20.0f);

    Color brilho = { 80, 160, 255, 255 };
    Vector2 anterior = efeito->posicao;

    // O ZIGUEZAGUE MUDA A CADA FRAME, DANDO A SENSACAO DE ELETRICIDADE
    for (int i = 1; i <= segmentos; i++) {

        Vector2 ponto = {
            efeito->posicao.x + efeito->direcao * i * 20.0f,
            efeito->posicao.y + (float)GetRandomValue(-14, 14)
        };

        DrawLineEx(anterior, ponto, 8, Fade(brilho, alfa * 0.45f));
        DrawLineEx(anterior, ponto, 3, Fade(efeito->cor, alfa));

        anterior = ponto;
    }

    DrawCircleV(efeito->posicao, 14.0f + 4.0f * sinf(efeito->tempoVivo * 40.0f), Fade(brilho, alfa * 0.7f));
    DrawCircleV(anterior, 8.0f, Fade(efeito->cor, alfa));
}

static void desenharFogo(const Efeito *efeito, float progresso) {

    float t = efeito->tempoVivo;

    Color amarelo = { 255, 230, 90, 255 };
    Color laranja = { 255, 140, 40, 255 };
    Color vermelho = { 220, 50, 30, 255 };

    for (int i = 0; i < 16; i++) {

        int r1 = ruido(efeito->semente, i);
        int r2 = ruido(efeito->semente, i + 100);

        float velocidade = 60.0f + (float)(r1 % 140);
        float subida = 20.0f + (float)(r2 % 70);

        float x = efeito->posicao.x + efeito->direcao * (10.0f + velocidade * t);
        float y = efeito->posicao.y + (float)(r1 % 40 - 20) - subida * t * 1.5f;
        float raio = 16.0f * (1.0f - progresso) + 3.0f + (float)(r2 % 6);

        Color cor = (progresso < 0.4f)
            ? misturarCores(amarelo, laranja, progresso / 0.4f)
            : misturarCores(laranja, vermelho, (progresso - 0.4f) / 0.6f);

        DrawCircleV((Vector2){ x, y }, raio, Fade(cor, (1.0f - progresso) * 0.9f));
    }
}

static void desenharAgua(const Efeito *efeito, float progresso) {

    float t = efeito->tempoVivo;
    float alfa = 1.0f - progresso;

    // ONDAS SE ESPALHANDO
    for (int k = 0; k < 3; k++) {

        float tk = t - k * 0.12f;

        if (tk < 0.0f) {
            continue;
        }

        float raio = 10.0f + tk * 120.0f;

        DrawRing(efeito->posicao, raio - 3.0f, raio, 0.0f, 360.0f, 36, Fade(efeito->cor, alfa * 0.8f));
    }

    // GOTAS SENDO ARREMESSADAS
    for (int i = 0; i < 12; i++) {

        int r1 = ruido(efeito->semente, i);
        int r2 = ruido(efeito->semente, i + 50);

        float vx = (40.0f + (float)(r1 % 160)) * efeito->direcao;
        float vy = 120.0f + (float)(r2 % 160);

        Vector2 gota = {
            efeito->posicao.x + vx * t,
            efeito->posicao.y - vy * t + 420.0f * t * t
        };

        DrawCircleV(gota, 4.0f, Fade((Color){ 170, 220, 255, 255 }, alfa));
    }
}

static void desenharVento(const Efeito *efeito, float progresso) {

    float t = efeito->tempoVivo;
    float alfa = 1.0f - progresso;

    for (int i = 0; i < 6; i++) {

        int r1 = ruido(efeito->semente, i);
        int r2 = ruido(efeito->semente, i + 30);

        float baseY = efeito->posicao.y - 50.0f + i * 20.0f + (float)(r1 % 10);
        float baseX = efeito->posicao.x + efeito->direcao * (t * 420.0f + (float)(r2 % 40));

        Vector2 anterior = { baseX, baseY + 6.0f * sinf(t * 12.0f + i) };

        // CADA RAJADA E UMA LINHA ONDULADA COM CAUDA QUE SOME
        for (int k = 1; k <= 10; k++) {

            Vector2 ponto = {
                baseX - efeito->direcao * k * 9.0f,
                baseY + 6.0f * sinf(k * 0.7f + t * 12.0f + i)
            };

            float espessura = 3.0f * (1.0f - k / 10.0f) + 1.0f;

            float alfaLinha = alfa * (1.0f - k / 12.0f);

            // SOMBRA ESCURA + LINHA CLARA: APARECE EM FUNDO CLARO E ESCURO
            DrawLineEx(anterior, ponto, espessura + 2.0f, Fade((Color){ 20, 70, 60, 255 }, alfaLinha * 0.6f));
            DrawLineEx(anterior, ponto, espessura, Fade(efeito->cor, alfaLinha));

            anterior = ponto;
        }
    }
}

// =====================================
// DESENHAR TODOS OS EFEITOS DA LISTA
// =====================================
void desenharEfeitos(const Efeito *lista) {

    for (const Efeito *efeito = lista; efeito != NULL; efeito = efeito->proximo) {

        if (efeito->atraso > 0.0f) {
            continue;
        }

        float progresso = efeito->tempoVivo / efeito->duracao;

        switch (efeito->tipo) {
            case EFEITO_TEXTO:  desenharTexto(efeito);            break;
            case EFEITO_FAISCA: desenharFaisca(efeito, progresso); break;
            case EFEITO_FLASH:  desenharFlash(efeito, progresso);  break;
            case EFEITO_RAIO:   desenharRaio(efeito, progresso);   break;
            case EFEITO_FOGO:   desenharFogo(efeito, progresso);   break;
            case EFEITO_AGUA:   desenharAgua(efeito, progresso);   break;
            case EFEITO_VENTO:  desenharVento(efeito, progresso);  break;
        }
    }
}

// =====================================
// LIBERAR TODA A LISTA
// =====================================
void liberarEfeitos(Efeito **lista) {

    Efeito *atual = *lista;

    while (atual != NULL) {

        Efeito *proximo = atual->proximo;

        free(atual);

        atual = proximo;
    }

    *lista = NULL;
}

int contarEfeitos(const Efeito *lista) {

    int total = 0;

    for (const Efeito *efeito = lista; efeito != NULL; efeito = efeito->proximo) {
        total++;
    }

    return total;
}