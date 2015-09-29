#include "utils.h"

void cria_pacote(char *dados, char tam, char seq, char tipo, pacote_t *pacote)
{
    // Bytes especificados mais 3 de cabeçalho e preambulo
    pacote->tam = tam;
    pacote->dados[0] = INICIO;
    pacote->dados[1] = (tam << 2) | (seq >> 3 & 0xF);
    pacote->dados[2] = (sq << 5) | (tipo & 0xF);
    if (tam > 0) {
        int i;
        for (i=0; i<tam; i++)
            pacote->dados[3+i] = dados[i];
    }
    pacote->dados[tam+3] = paridade(tam, seq, tipo, dados);
}