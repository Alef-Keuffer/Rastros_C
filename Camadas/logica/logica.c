#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "Camadas/dados/modificar_estado.h"
#include "Camadas/dados/dados.h"
#include "Camadas/dados/acessar_estado.h"
#include "Camadas/interface/interface.h"

int jogar(ESTADO *e, COORDENADA c) {
    atualizar_tab(e, c);
    atualizar_ultima_jogada(e, c);
    atualizar_jogadas(e, c);
    atualizar_num_jogadas(e);
    atualizar_jogador_atual(e);
    return 1;
}

int jogada_valida(ESTADO *e, COORDENADA c){
    int casa = obter_casa(e, c.linha, c.coluna), res;
    COORDENADA ultJog = obter_ultimajogada(e);
    int difLinha = abs(c.linha - ultJog.linha);
    int difColuna = abs(c.coluna - ultJog.coluna);

    res = (casa != PRETA && casa != BRANCA && (difLinha == 0 || difLinha == 1) && (difColuna == 0 || difColuna == 1));

    return res;
}

int fim_de_jogo(ESTADO *e){
    int i, j, res, num_vazias = 0;
    int jog = JogadorAtual(e);
    COORDENADA c = obter_ultimajogada(e);

    for(i = -1; i < 2; i++){
        for(j = -1; j < 2; j++){
            if(c.linha + i >= 0 && c.linha + i <= 7 && c.coluna + j >= 0 && c.coluna + j <= 7 && obter_casa(e, c.linha + i, c.coluna + j) != PRETA && obter_casa(e, c.linha + i, c.coluna + j) != BRANCA) num_vazias++;
        }
    }

    if((num_vazias == 0 && jog == 2) || (c.linha == 0 && c.coluna == 0)) res = 1;
    else if((num_vazias == 0 && jog == 1) || (c.linha == 7 && c.coluna == 7)) res = 2;
    else res = 0;

    return res;
}

void movs(ESTADO *e, FILE *file){
    int a, i;
    for (a=0, i=1; a<NumJogadas(e); i++, a++)
        imprime_movs1(file, i, obter_subcoordenadas(e, a, 1, COLUNA) +'a', obter_subcoordenadas(e, a, 1, LINHA) +'1', obter_subcoordenadas(e, a, 2, COLUNA) + 'a', obter_subcoordenadas(e, a, 2, LINHA) + '1');

    if(JogadorAtual(e) == 2) imprime_movs2(file, i, obter_subcoordenadas(e, a, 1, COLUNA) +'a', obter_subcoordenadas(e, a, 1, LINHA) +'1');
}

void pos(ESTADO *e, int position, int state){
    int num;
    int i, ultimo = 2;
    COORDENADA ultjogada;

    if(state == 0) gr(e, "ficheiropos");
    else ler(e, "ficheiropos");

    num = NumJogadas(e);
    for(i = position; i <= num; i++){
        if(i < num || JogadorAtual(e) == 2) limpar_casas(e, i);
    }
    if(position == NumJogadas(e) + 1 && JogadorAtual(e) == 2) ultimo = 1;

    if(position == 0) ultjogada.linha = ultjogada.coluna = 4;
    else ultjogada = obter_coordenada(e, position - 1, ultimo);

    colocar_branca(e, ultjogada);
    atualizar_ultima_jogada(e, ultjogada);
    armazenar_jogador(e, ultimo);
    if(JogadorAtual(e) == 1) armazenar_num_jogadas(e, position);
    else armazenar_num_jogadas(e, position - 1);
}

int coord_to_int(COORDENADA c){
    int x;
    x = c.linha * 10 + c.coluna;
    return x;
}

COORDENADA int_to_coord(int x){
    COORDENADA c;
    c.linha = x/10;
    c.coluna = x % 10;
    return c;
}

int avaliar_vitoria(ESTADO *e){
    int res = 0;
    if(JogadorAtual(e) == fim_de_jogo(e)) res = 1;
    else if(fim_de_jogo(e) != 0) res = -1;
    return res;
}

int desfazer_ultjogada(ESTADO *e){
    branca_para_vazia(e);
    nova_ultjogada(e);
    colocar_branca(e, obter_ultimajogada(e));
    modificar_numjogadas(e);
    atualizar_jogador_atual(e);
    return 1;
}

int array_jogadaspossiveis(ESTADO *e, COORDENADA c, COORDENADA moveList[]){
    int i, j, moveCount = 0;
    for(i = -1; i < 2; i++){
        for(j = -1; j < 2; j++){
            int row = c.linha + i;
            int col = c.coluna + j;
            if(row >= 0 && row <= 7 && col >= 0 && col <= 7 && (obter_casa(e, row, col) == VAZIO || obter_casa(e, row, col) == UM || obter_casa(e, row, col) == DOIS)) {
                moveList[moveCount].linha = row;
                moveList[moveCount].coluna = col;
                moveCount++;
            }
        }
    }
    return moveCount;
}

COORDENADA maisproximaFim(ESTADO *e, COORDENADA moveList[], int moveCount){
    int index, dist, distMin = 20;
    COORDENADA move, bestMove, fim1 = {0, 0}, fim2 = {7, 7};
    for(index = 0; index < moveCount; index++){
        move = moveList[index];
        if(JogadorAtual(e) == 1) dist = abs(move.linha - fim1.linha) + abs(move.coluna - fim1.coluna);
        else dist = abs(move.linha - fim2.linha) + abs(move.coluna - fim2.coluna);
        if(dist <= distMin){
            distMin = dist;
            bestMove = move;
        }
    }
    return bestMove;
}

int minMax(ESTADO *e, int profundidade){
    COORDENADA moveList[8];
    int moveCount = 0;
    int bestScore = -2;
    int score = -2;
    COORDENADA bestMove = {-1, -1};
    COORDENADA move;
    int index, lose = 0;
    COORDENADA c = obter_ultimajogada(e);

    if(profundidade > 0){
        score = avaliar_vitoria(e);
        if(score != 0) return score;
    }

    moveCount = array_jogadaspossiveis(e, c, moveList);

    for(index = 0; index < moveCount && profundidade < 8; index++){
        move = moveList[index];
        jogar(e, move);
        profundidade++;
        score = -minMax(e, profundidade);
        if(score > bestScore){
            bestScore = score;
            bestMove = move;
        }
        if(score == -1) lose = 1;
        desfazer_ultjogada(e);
        profundidade--;
    }

    if(profundidade == 0 && bestScore == 0 && lose == 0) bestMove = maisproximaFim(e, moveList, moveCount);

    if(profundidade != 0 && profundidade < 8) return bestScore;
    else if(profundidade >= 8) return 0;
    else return coord_to_int(bestMove);
}

void jog(ESTADO *e){
    int x;
    COORDENADA c;

    x = minMax(e, 0);
    c = int_to_coord(x);
    jogar(e, c);
}

/*Função aux ao comando Jog2*/
COORDENADA randomJog (ESTADO *e)
{
    COORDENADA c = obter_ultimajogada(e);
    COORDENADA moveList[8];
    int moveCount = 0;

    moveCount = array_jogadaspossiveis(e, c, moveList);
    int n;
    srand(time(NULL));
    n = (rand() % (moveCount));

    return moveList[n];
}

/*Comando Jog2 que faz uma jogada aleatória.*/
void jog2(ESTADO *e)
{
    COORDENADA c;
    c = randomJog (e);
    jogar(e, c);
}
