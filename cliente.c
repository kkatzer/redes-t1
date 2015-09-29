#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void help() {
  printf("Modo de uso\n\n");
  printf("Comandos Locais:\n");
  printf("cd [diretório]\n");
  printf("ls [opções] [diretório]\n");
  printf("clear\n");
  printf("exit\n\n");
  printf("Comandos Remotos:\n");
  printf("cdr [diretório]\n");
  printf("lsr [opções] [diretório]\n");
  printf("get [arquivo]\n");
  printf("put [arquivo]\n\n");
  return;
}

int main(int argc, char **argv) {
  char str[64];

  system("clear");
  printf("Cliente\n");
  printf("Digite \"ajuda\" para mais informações.\n\n");

  int leitura = 1, scanf_return;

  char comando[16];

  while (leitura) {
    printf("> ");

    scanf_return = scanf("%[^\n]s", str);
    if (scanf_return == 1) {
      sscanf(str, "%s", comando);
      sscanf(str, "%s", comando);
    }
    else {
      strcpy(comando, "");
    }
    if (!strcmp(comando, "ajuda")) {
      help();
    }
    else if (!strcmp(comando, "cd")) {
      if (strlen(str) < 4) {
        chdir("");
      }
      else {
        chdir(str+3);
      }
    }
    else if (!strcmp(comando, "ls")) {
      system(str);
    }
    else if (!strcmp(comando, "clear")) {
      system(str);
    }
    else if (!strcmp(comando, "exit")) {
      leitura = 0;
    }
    else {
      printf("Comando inválido.\n");
      printf("Digite \"ajuda\" para mais informações.\n");
    }
    getchar(); // Pega '\n'.
  }

  return 0;
}