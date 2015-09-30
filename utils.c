#include "utils.h"

void cria_pacote(char *dados, char tam, char seq, char tipo, pacote_t *pacote) {
  pacote->tam = tam;
  pacote->dados[0] = INICIO;
  pacote->dados[1] = (tam << 2) | (seq >> 3 & 0x3);
  pacote->dados[2] = (seq << 5) | (tipo & 0x1F);
  if (tam > 0) {
    int i;
    for (i=0; i<tam; i++)
      pacote->dados[3+i] = dados[i];
  }
  pacote->dados[tam+3] = paridade(pacote->dados);
}

void le_pacote(char *s, pacote_t pacote) {
  int i;
  printf("%s tam=%d ", s, (int) ((pacote.dados[1] >> 2) & 0x3F));
  printf("seq=%d ", (int) (((pacote.dados[1] & 0x3) << 3) & (pacote.dados[2] >> 5)));
  printf("tipo=%d ", (int) ((pacote.dados[2]) & 0x1F));
  printf("dados=");
  for (i = 0; i < pacote.tam; i++) {
    printf("%c", pacote.dados[3+i]);
  }
  printf(" paridade=%d\n", pacote.dados[pacote.tam+3]);
}

int paridade(char *dados) {
  char par = dados[1];
  int i;
  for (i = 1; i < ((int) ((dados[1] >> 2) & 0x3F)) + 2; i++) {
    par = par ^ dados[i+1];
  }
  return par;
}