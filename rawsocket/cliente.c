#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/stat.h>
#include "comum.h"

void ajuda() {
    printf("Uso:\n");
    printf("Comandos locais:\n");
    printf("cd [diretorio]\n");
    printf("cat [arquivo]\n");
    printf("ls [opções] [arquivo]\n");
    printf("pwd\n");
    printf("clear\n\n");

    printf("Comandos remotos:\n");
    printf("cdr [diretorio]\n");
    printf("catr [arquivo]\n");
    printf("lsr [opções] [arquivo]\n");
    printf("pwdr\n");
    printf("get [arquivo]\n\n");
    printf("put [arquivo]\n\n");
}

int main()
{
    char * interface = "eth0";
    char erros[4] = {INEXISTENTE, PERM_NEGADA, ESP_INSUFICIENTE, DESCONHECIDO};
    char str[64];
    int sockfd = abre_socket(interface);

    //Pacote
    pacote_t pacote, resposta, pacote_dados;
    char dados[TAM_MAX_DADOS];
    char arg2[TAM_MAX_DADOS];

    system("clear");
    printf("Cliente\n");
    printf("Digite \"ajuda\" para mais informações.\n");

    int leitura = 1, scanf_ret;
    char comando[16];
    // Lê comandos
    while (leitura) {
        printf("> ");

        scanf_ret = scanf("%[^\n]s", str);
        if (scanf_ret == 1) {
            sscanf(str, "%s", comando);
            sscanf(str, "%s", comando);
        }
        else {
            strcpy(comando, "");
        }

        if (!strcmp(comando, "cd")) {
            chdir(str);
        }
        if (!strcmp(comando, "cat")) {
            system(str);
        }
        else if (!strcmp(comando, "ls")) {
            system(str);
        }
        else if (!strcmp(comando, "pwd")) {
            system(str);
        }
        else if (!strcmp(comando, "clear")) {
            system(str);
        }
        else if (!strcmp(comando, "cdr")) {
            sscanf(str, "%s %s", dados, arg2);
            printf("%s %s\n", dados, arg2);
            sscanf(str, "%s %[^\n]s", dados, arg2);
            int tam_dados = strlen(dados);
            if (tam_dados <= (TAM_MAX_DADOS)) {
                monta_pacote(arg2, (char) strlen(arg2), 0, CD, &pacote);
                envia_pacote(sockfd, pacote, 1);
            }
            else
                printf("Campo dados deve possuir até %d caracteres.\n", TAM_MAX_DADOS);
        }
        else if (!strcmp(comando, "catr")) {
            sscanf(str, "%s %[^\n]s", dados, dados);
            monta_pacote(dados, (char) strlen(dados), 0, CAT, &pacote);
            envia_pacote(sockfd, pacote, 1);
        }
        else if (!strcmp(comando, "pwdr")) {
            sscanf(str, "%s %[^\n]s", dados, dados);
            monta_pacote(dados, (char) strlen(dados), 0, PWD, &pacote);
            envia_pacote(sockfd, pacote, 1);
        }
        else if (!strcmp(comando, "lsr")) {
            sscanf(str, "%s %[^\n]s", dados, dados);
            monta_pacote(dados, (char) strlen(dados), 0, LS, &pacote);
            envia_pacote(sockfd, pacote, 1);
        }
        else if (!strcmp(comando, "put")) {
            sscanf(str, "%s %[^\n]s", dados, dados);
            monta_pacote(dados, (char) strlen(dados), 0, PUT, &pacote);
            int ret = envia_pacote(sockfd, pacote, 1);

            if (ret == SUCESSO) {
                struct stat buff;
                int stat_ret = stat(dados, &buff);
                if (stat_ret < 0)
                    erros_stat(stat_ret, erros, sockfd);
                else {
                    char tam_arquivo[4], buffer[TAM_MAX_DADOS];
                    monta_tam_arq(buff, tam_arquivo);
                    monta_pacote(tam_arquivo, 4, 1, TAM_ARQUIVO, &resposta);

                    if (envia_pacote(sockfd, resposta, 1) == SUCESSO) {
                        monta_pacote(dados, (char) strlen(dados), 0, CRIA_ARQ, &pacote_dados);
                        if (envia_pacote(sockfd, pacote_dados, 1) == ACK) {

                            // Envia pacote
                            int n, i=0;
                            FILE *fp = fopen(dados, "r");
                            while((n=fread(buffer, 1, 15, fp)) && (n>0)) {

                                monta_pacote(buffer, (char) n, (char) (i%SEQUENCIA), DADOS, &pacote_dados);
                                envia_pacote(sockfd, pacote_dados, 1);
                                i++;
                            }

                            // Notifica término de arquivo.
                            monta_pacote(NULL, 0, 0, FIM_DADOS, &pacote_dados);
                            envia_pacote(sockfd, pacote_dados, 0);
                        }
                    }
                }
            }
        }
        else if (!strcmp(comando, "get")) {
            sscanf(str, "%s %[^\n]s", dados, dados);
            monta_pacote(dados, (char) strlen(dados), 0, GET, &pacote);
            envia_pacote(sockfd, pacote, 1);
        }
        else if (!strcmp(comando, "quit")) {
            leitura = 0;
        }
        else {
            printf("Sintaxe inválida.\n");
            printf("Digite \"ajuda\" para mais informações.\n");
        }
        getchar(); // Pega '\n'.
    }

    close(sockfd);

    return 0;
}
