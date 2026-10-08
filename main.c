#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "raylib.h"

typedef enum {
    ESTADO_MENU,
    ESTADO_DIGITANDO_NOME,
    ESTADO_DISTRIBUINDO,
    TURNO_JOGADOR,
    TURNO_DEALER,
    FIM_JOGO
} Estado;

typedef struct {
    int naipe; // 0: Paus, 1: Copas, 2: Espadas, 3: Ouros
    int valor;
} Carta;

// Função para buscar ou carregar o saldo de um jogador no arquivo
int carregar_saldo(const char* nome) {
    FILE *file = fopen("salvos.txt", "r");
    if (!file) return 0; // Se o arquivo não existe, o saldo inicial é 0

    char linha[100];
    char nome_arquivo[50];
    int saldo_arquivo;
    int saldo_encontrado = 0;

    while (fgets(linha, sizeof(linha), file)) {
        if (sscanf(linha, "%49s %d", nome_arquivo, &saldo_arquivo) == 2) {
            if (strcmp(nome_arquivo, nome) == 0) {
                saldo_encontrado = saldo_arquivo;
                break;
            }
        }
    }
    fclose(file);
    return saldo_encontrado;
}

// Função para salvar ou atualizar o saldo de um jogador no arquivo
void salvar_saldo(const char* nome, int novo_saldo) {
    FILE *file = fopen("salvos.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    
    bool encontrado = false;

    if (file) {
        char linha[100];
        char nome_arquivo[50];
        int saldo_arquivo;

        while (fgets(linha, sizeof(linha), file)) {
            if (sscanf(linha, "%49s %d", nome_arquivo, &saldo_arquivo) == 2) {
                if (strcmp(nome_arquivo, nome) == 0) {
                    fprintf(temp, "%s %d\n", nome, novo_saldo);
                    encontrado = true;
                } else {
                    fprintf(temp, "%s %d\n", nome_arquivo, saldo_arquivo);
                }
            }
        }
        fclose(file);
    }

    if (!encontrado && temp) {
        fprintf(temp, "%s %d\n", nome, novo_saldo);
    }

    if (temp) {
        fclose(temp);
        remove("salvos.txt");
        rename("temp.txt", "salvos.txt");
    }
}

// Calcula a soma da mão, ajustando o valor do Ás se necessário
int calcular_mao(Carta mao[], int quantidade) {
    int soma = 0;
    int num_A = 0;
    for (int i = 0; i < quantidade; i++) {
        switch (mao[i].valor) {
            case 1:
                num_A++;
                soma += 11;
                break;
            case 11:
            case 12:
            case 13:
                soma += 10;
                break;
            default:
                soma += mao[i].valor;
                break;
        }
    }

    for (int i = 0; i < num_A; i++) {
        if (soma <= 21) break;
        soma -= 10;
    }
    return soma;
}

bool BotaoClicado(Rectangle bounds) {
    return CheckCollisionPointRec(GetMousePosition(), bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void DesenharBotao(Rectangle bounds, const char* text) {
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    DrawRectangleRec(bounds, isHovered ? GRAY : LIGHTGRAY);
    DrawRectangleLinesEx(bounds, 2.0f, BLACK);
    
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    
    float textX = bounds.x + (bounds.width - textWidth) / 2.0f;
    float textY = bounds.y + (bounds.height - fontSize) / 2.0f;
    
    DrawText(text, (int)textX, (int)textY, fontSize, BLACK);
}

void desenhar_mao(Carta mao[], int quantidade, int x_inicial, int y, bool esconder) {
    int x = x_inicial;

    for (int i = 0; i < quantidade; i++) {
        Rectangle rect = { (float)x, (float)y, 70, 100 };

        if (esconder && i == 1) {
            DrawRectangleRec(rect, MAROON);
            DrawRectangleLinesEx(rect, 2.0f, WHITE);
            DrawRectangle(x + 10, y + 10, 50, 80, DARKGRAY);
            x += 80;
            continue;
        }

        DrawRectangleRec(rect, RAYWHITE);
        DrawRectangleLinesEx(rect, 2.0f, BLACK);

        Color corCarta = (mao[i].naipe == 1 || mao[i].naipe == 3) ? RED : BLACK;

        const char* strValor = "";
        switch (mao[i].valor) {
            case 1: strValor = "A"; break;
            case 11: strValor = "J"; break;
            case 12: strValor = "Q"; break;
            case 13: strValor = "K"; break;
            default: strValor = TextFormat("%d", mao[i].valor); break;
        }

        const char* strNaipe = "";
        switch (mao[i].naipe) {
            case 0: strNaipe = "Paus"; break;
            case 1: strNaipe = "Copas"; break;
            case 2: strNaipe = "Esp"; break;
            case 3: strNaipe = "Ouros"; break;
        }

        DrawText(strValor, x + 8, y + 6, 18, corCarta);
        int widthNaipe = MeasureText(strNaipe, 14);
        DrawText(strNaipe, x + (70 - widthNaipe) / 2, y + 42, 14, corCarta);
        DrawText(strValor, x + 48, y + 74, 18, corCarta);

        x += 80;
    }

    if (esconder) {
        if (quantidade == 0) {
            DrawText("Soma: ?", x_inicial, y + 110, 20, YELLOW);
        } else {
            DrawText(TextFormat("Soma: %d", calcular_mao(mao, 1)), x_inicial, y + 110, 20, YELLOW);
        }
    } else {
        DrawText(TextFormat("Soma: %d", calcular_mao(mao, quantidade)), x_inicial, y + 110, 20, YELLOW);
    }
}

int main(void) {
    srand(time(NULL));
    InitWindow(800, 600, "Blackjack");
    SetTargetFPS(60);

    Carta baralho[52];
    int idx = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 1; j < 14; j++) {
            baralho[idx].valor = j;
            baralho[idx].naipe = i;
            idx++;
        }
    }

    Carta mao_jogador[12];
    Carta mao_dealer[12];
    int num_jogador = 0;
    int num_dealer = 0;
    int topo = 0;
    
    Estado estado = ESTADO_MENU; 
    int vencedor = -1;
    bool precisa_reiniciar = false;
    
    char nomeJogador[50] = "";
    int letraCount = 0;
    int dinheiro = 0;
    bool premio_entregue = false;

    float tempoDealer = 0.0f;
    float tempoDistribuicao = 0.0f;
    int etapaDistribuicao = 0;

    Rectangle btnComprar = {150, 520, 120, 40};
    Rectangle btnParar = {290, 520, 120, 40};
    
    Rectangle btnRestart = {150, 295, 220, 45};
    Rectangle btnSairFim = {390, 295, 220, 45};

    Rectangle btnMenuJogar = {300, 260, 200, 50};
    Rectangle btnMenuSair = {300, 330, 200, 50};
    Rectangle caixaTexto = {250, 265, 300, 45};
    Rectangle btnConfirmarNome = {300, 330, 200, 45};

    while (!WindowShouldClose()) {
        
        if (precisa_reiniciar) {
            num_jogador = 0;
            num_dealer = 0;
            topo = 0;
            vencedor = -1;
            tempoDealer = 0.0f;
            tempoDistribuicao = 0.0f;
            etapaDistribuicao = 0;
            premio_entregue = false;
            estado = ESTADO_DISTRIBUINDO;

            for (int i = 51; i > 0; i--) {
                int j = rand() % (i + 1);
                Carta temp = baralho[i];
                baralho[i] = baralho[j];
                baralho[j] = temp;
            }

            precisa_reiniciar = false;
        }

        if (estado == ESTADO_MENU) {
            if (BotaoClicado(btnMenuJogar)) {
                estado = ESTADO_DIGITANDO_NOME;
                letraCount = 0;
                nomeJogador[0] = '\0';
            }
            if (BotaoClicado(btnMenuSair)) {
                break;
            }
        }
        else if (estado == ESTADO_DIGITANDO_NOME) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (letraCount < 49)) {
                    nomeJogador[letraCount] = (char)key;
                    nomeJogador[letraCount + 1] = '\0';
                    letraCount++;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                letraCount--;
                if (letraCount < 0) letraCount = 0;
                nomeJogador[letraCount] = '\0';
            }

            if ((BotaoClicado(btnConfirmarNome) || IsKeyPressed(KEY_ENTER)) && letraCount > 0) {
                dinheiro = carregar_saldo(nomeJogador);
                precisa_reiniciar = true;
            }
        }
        else if (estado == ESTADO_DISTRIBUINDO) {
            tempoDistribuicao += GetFrameTime();
            
            if (tempoDistribuicao >= 0.4f) {
                tempoDistribuicao = 0.0f;

                if (etapaDistribuicao == 0) {
                    mao_jogador[num_jogador++] = baralho[topo++];
                } 
                else if (etapaDistribuicao == 1) {
                    mao_dealer[num_dealer++] = baralho[topo++];
                } 
                else if (etapaDistribuicao == 2) {
                    mao_jogador[num_jogador++] = baralho[topo++];
                } 
                else if (etapaDistribuicao == 3) {
                    mao_dealer[num_dealer++] = baralho[topo++];

                    int ptsJog = calcular_mao(mao_jogador, num_jogador);
                    int ptsDeal = calcular_mao(mao_dealer, num_dealer);

                    bool playerBJ = (ptsJog == 21);
                    bool dealerBJ = (ptsDeal == 21);

                    if (playerBJ || dealerBJ) {
                        estado = FIM_JOGO;
                        if (playerBJ && dealerBJ) vencedor = 0;
                        else if (playerBJ) vencedor = 3;
                        else vencedor = 4;
                    } else {
                        estado = TURNO_JOGADOR;
                    }
                }
                etapaDistribuicao++;
            }
        }
        else if (estado == TURNO_DEALER) {
            tempoDealer += GetFrameTime();
            
            if (tempoDealer >= 0.8f) {
                tempoDealer = 0.0f;
                if (calcular_mao(mao_dealer, num_dealer) < 17) {
                    mao_dealer[num_dealer++] = baralho[topo++];
                } else {
                    estado = FIM_JOGO;
                }
            }
        }
        else if (estado == TURNO_JOGADOR) {
            if (BotaoClicado(btnComprar)) {
                mao_jogador[num_jogador++] = baralho[topo++];
                if (calcular_mao(mao_jogador, num_jogador) > 21) {
                    estado = FIM_JOGO;
                }
            }
            if (BotaoClicado(btnParar)) {
                estado = TURNO_DEALER;
                tempoDealer = 0.0f;
            }
        } 
        else if (estado == FIM_JOGO) {
            if (BotaoClicado(btnRestart)) {
                precisa_reiniciar = true;
            }
            if (BotaoClicado(btnSairFim)) {
                estado = ESTADO_MENU;
            }
            
            if (vencedor < 3) {
                int ptsJog = calcular_mao(mao_jogador, num_jogador);
                int ptsDeal = calcular_mao(mao_dealer, num_dealer);

                if (ptsJog > 21) vencedor = 2;
                else if (ptsDeal > 21) vencedor = 1;
                else if (ptsJog > ptsDeal) vencedor = 1;
                else if (ptsDeal > ptsJog) vencedor = 2;
                else vencedor = 0;
            }

            // Gerenciamento do Dinheiro e Salvamento (Permitindo valores negativos)
            if (!premio_entregue) {
                if (vencedor == 1) {
                    dinheiro += 100; // Vitória normal: +100
                } else if (vencedor == 3) {
                    dinheiro += 200; // Blackjack: +200
                } else if (vencedor == 2) {
                    dinheiro -= 100; // Derrota normal: -100
                } else if (vencedor == 4) {
                    dinheiro -= 200; // Derrota de Blackjack do dealer: -200
                }
                
                salvar_saldo(nomeJogador, dinheiro); // Salva o saldo (mesmo se negativo)
                premio_entregue = true;
            }
        }

        BeginDrawing();
        ClearBackground(DARKGREEN);

        if (estado == ESTADO_MENU) {
            int titleWidth = MeasureText("BLACKJACK", 60);
            DrawText("BLACKJACK", (800 - titleWidth) / 2, 140, 60, GOLD);

            DesenharBotao(btnMenuJogar, "JOGAR");
            DesenharBotao(btnMenuSair, "SAIR");
        } 
        else if (estado == ESTADO_DIGITANDO_NOME) {
            int titleWidth = MeasureText("DIGITE SEU NOME", 30);
            DrawText("DIGITE SEU NOME", (800 - titleWidth) / 2, 180, 30, GOLD);

            DrawRectangleRec(caixaTexto, LIGHTGRAY);
            DrawRectangleLinesEx(caixaTexto, 2.0f, BLACK);
            DrawText(nomeJogador, (int)caixaTexto.x + 10, (int)caixaTexto.y + 12, 20, BLACK);

            DesenharBotao(btnConfirmarNome, "ENTRAR");
        }
        else {
            DrawText("Mesa do Dealer", 150, 40, 20, LIGHTGRAY);
            DrawText("Sua Mão", 150, 350, 20, LIGHTGRAY);

            const char* textoInfo = TextFormat("Jogador: %s | Saldo: $%d", nomeJogador, dinheiro);
            int larguraTextoInfo = MeasureText(textoInfo, 18);
            DrawText(textoInfo, 800 - larguraTextoInfo - 20, 20, 18, GOLD);

            bool esconderCartaDealer = (estado == TURNO_JOGADOR || (estado == ESTADO_DISTRIBUINDO && num_dealer < 2));
            
            desenhar_mao(mao_dealer, num_dealer, 150, 70, esconderCartaDealer);
            desenhar_mao(mao_jogador, num_jogador, 150, 380, false);

            if (estado == TURNO_JOGADOR) {
                DesenharBotao(btnComprar, "COMPRAR");
                DesenharBotao(btnParar, "PARAR");
            } 
            else if (estado == FIM_JOGO) {
                if (vencedor == 1) {
                    DrawText("VOCÊ VENCEU!", 150, 240, 40, GOLD);
                } 
                else if (vencedor == 2) {
                    DrawText("O DEALER VENCEU!", 152, 242, 40, BLACK);
                    DrawText("O DEALER VENCEU!", 150, 240, 40, RED);
                } 
                else if (vencedor == 3) {
                    DrawText("BLACKJACK! VOCÊ VENCEU!", 152, 242, 40, BLACK);
                    DrawText("BLACKJACK! VOCÊ VENCEU!", 150, 240, 40, GOLD);
                }
                else if (vencedor == 4) {
                    DrawText("BLACKJACK DO DEALER!", 152, 242, 40, BLACK);
                    DrawText("BLACKJACK DO DEALER!", 150, 240, 40, RED);
                }
                else {
                    DrawText("EMPATE!", 150, 240, 40, RAYWHITE);
                }

                DesenharBotao(btnRestart, "JOGAR NOVAMENTE");
                DesenharBotao(btnSairFim, "MENU PRINCIPAL");
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}