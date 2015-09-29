#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/stat.h>
#include "comum.h"

pacote_t pacote;
pacote_t resposta;
int sockfd;

// Monta pacote e retorna 0 se recebe pacote válido. Senão -1.
int esperando_pacote(int sockfd)
{
    char tam, seq, tipo;
    char buffer[TAM_MAX_PACOTE];

    ssize_t recebe_tamanho =  recv(sockfd, buffer, TAM_MAX_PACOTE, 0);
    if (recebe_tamanho < 0)
    {
        perror("recv: erro");
        return -1;
    }
    else
    {
        //verifica se pacote é valido
        if (buffer[0] == PREAMBULO)
        {
            tam = ((buffer[1] >> 4) & 0xF);
            seq = (buffer[1] & 0xF);
            tipo  = ((buffer[2] >> 4) & 0xF);
            monta_pacote(buffer+3, tam, seq, tipo, &pacote);

            return 0;
        }
        else
        {
            return -1;
        }
    }
}

int main()
{
    pacote_t pacote_dados;
    char interface[8] = "eth0";
    char comando[32], dados[TAM_MAX_DADOS], buffer[TAM_MAX_DADOS], tipo, tam;
    sockfd = abre_socket(interface);
    char erros[4] = {INEXISTENTE, PERM_NEGADA, ESP_INSUFICIENTE, DESCONHECIDO};
    struct stat buff;
    int i;
    FILE *f;

    system("clear");
    printf("Servidor escutando...\n\n");

    //escuta para sempre e bloqueia enquanto espera resposta
    while (1)
    {
        int ret = esperando_pacote(sockfd);
        if (ret < 0) // deu erro ou não encontrou pacote válido
            continue;

        // Pacote recebido
        le_pacote("Pacote recebido pelo servidor", pacote);

        tipo = tipo_pacote(pacote);
        tam = tam_pacote(pacote);
        // Interpreta pacote e executa respectivo comando.
        // TODO colocar essa tripa toda na função interpreta_comando(tipo)
        switch (tipo) {
            case CD:
                printf("CD\n");
                strcpy(dados, pacote.dados+3);
                for (i=0; i<tam; i++)
                    dados[i] = pacote.dados[3+i];
                if (tam < TAM_MAX_DADOS)
                    dados[i] = '\0';
                ret = chdir(dados);
                if (!ret) {
                    monta_pacote(NULL, 0, 0, SUCESSO, &resposta);
                    envia_pacote(sockfd, resposta, 0);
                }
                else{
                    erros_stat(errno, erros, sockfd);
                }
                printf("\n");
                break;
            case LS:
                printf("LS\n");

                char comando[32];

                for (i=0; i<tam; i++)
                    dados[i] = pacote.dados[3+i];
                if (tam < TAM_MAX_DADOS)
                    dados[i] = '\0';

                char path[TAM_MAX_DADOS];
                while(sscanf(dados, "%s %s %s %s %s", path, path, path, path, path) == 5);

                if (stat(path, &buff) < 0) {
                    erros_stat(errno, erros, sockfd);
                    break;
                }
                strcpy(comando, "/bin/ls ");
                strcat(comando, dados);
                f = popen(comando, "r");
                int n, i=0;
                while ((n = fread(buffer, 1, 15, f)) && (n > 0)) {
                    if (n < 15)
                        buffer[n] = '\0';
                    monta_pacote(buffer, (char) n, (char) i%SEQUENCIA, MOSTRA_NA_TELA, &pacote_dados);
                    envia_pacote(sockfd, pacote_dados, 1);
                    i++;
                }
                monta_pacote(NULL, 0, 0, FIM_TEXTO, &pacote_dados);
                envia_pacote(sockfd, pacote_dados, 0);

                fclose(f);
                break;
            case PUT:
                printf("PUT\n");
                char lixo[1] = {PUT};
                monta_pacote(lixo, 1, 0, SUCESSO, &resposta);
                envia_pacote(sockfd, resposta, 1);
                break;
            case GET:
                printf("GET\n");
                for (i=0; i<tam; i++) {
                    dados[i] = pacote.dados[3+i];
                    printf("%c", dados[i]);
                }

                int stat_ret = stat(dados, &buff);
                if (stat_ret < 0)
                    erros_stat(errno, erros, sockfd);
                else {
                    // Envia pacote contendo tamanho do arquivo em bytes.
                    char tam_arquivo[4];
                    monta_tam_arq(buff, tam_arquivo);
                    monta_pacote(tam_arquivo, 4, 1, TAM_ARQUIVO, &resposta);
                    ret = envia_pacote(sockfd, resposta, 1);
                    if (ret == SUCESSO) {
                        // Se recebeu SUCESSO é hora de ler o arquivo e enviar em pacotes pro cliente
                        i = 0;

                        monta_pacote(dados, tam, 0, CRIA_ARQ, &pacote_dados);
                        ret = envia_pacote(sockfd, pacote_dados, 1);

                        if (ret == NACK) {
                            printf("Operação cancelada pelo usuário.\n");
                            break;
                        }
                        else if (ret == ACK) {
                            // Envia arquivo
                            // n é a quantidade de bytes lidos no buffer
                            int n;
                            FILE *fp = fopen(dados, "r");
                            while ((n = fread(buffer, 1, 15, fp)) && (n > 0)) {
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
                printf("\n");
                break;
            case PWD:
                printf("PWD\n");
                f = popen("/bin/pwd", "r");
                i=0;
                while ((n = fread(buffer, 1, 15, f)) && (n > 0)) {
                    monta_pacote(buffer, (char) n, (char) i%SEQUENCIA, MOSTRA_NA_TELA, &pacote_dados);
                    int ret = envia_pacote(sockfd, pacote_dados, 1);
                    while (ret < 0)
                        ret = envia_pacote(sockfd, pacote_dados, 1);
                    i++;
                    if (ret == ACK)
                        continue;
                }
                monta_pacote(NULL, 0, 0, FIM_TEXTO, &pacote_dados);
                envia_pacote(sockfd, pacote_dados, 0);

                fclose(f);
                break;
            case CAT:
                printf("CAT\n");
                strcpy(dados, pacote.dados+3);
                stat_ret = stat(dados, &buff);
                if (stat_ret < 0)
                    erros_stat(errno, erros, sockfd);
                else {
                    char comando[TAM_MAX_DADOS+10];

                    for (i=0; i<tam; i++)
                        dados[i] = pacote.dados[3+i];
                    if (tam < TAM_MAX_DADOS)
                        dados[i] = '\0';

                    char path[TAM_MAX_DADOS];
                    while(sscanf(dados, "%s %s %s %s %s", path, path, path, path, path) == 5);

                    if (stat(path, &buff) < 0) {
                        erros_stat(errno, erros, sockfd);
                        break;
                    }
                    strcpy(comando, "/bin/cat ");
                    strcat(comando, dados);
                    f = popen(comando, "r");
                    int n, i=0;
                    while ((n = fread(buffer, 1, 15, f)) && (n > 0)) {
                        if (n < 15)
                            buffer[n] = '\0';
                        monta_pacote(buffer, (char) n, (char) i%SEQUENCIA, MOSTRA_NA_TELA, &pacote_dados);
                        envia_pacote(sockfd, pacote_dados, 1);
                        i++;
                    }
                    monta_pacote(NULL, 0, 0, FIM_TEXTO, &pacote_dados);
                    envia_pacote(sockfd, pacote_dados, 0);

                    fclose(f);
                }
                break;
        }
    }

    strcpy(comando, "");
    close(sockfd);

    return 0;
}
