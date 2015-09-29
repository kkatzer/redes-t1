#include "comum.h"

pacote_t pkt;
char nome_arq[TAM_MAX_DADOS];
FILE *fp;

// Abre raw socket com a interface especificada, faz o bind e habilita o modo
// promíscuo
int abre_socket(char * interface)
{
    struct ifreq ifr;
    struct sockaddr_ll sock_addr;
    struct packet_mreq pack;

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0) {
        fprintf(stderr, "Erro ao abrir socket. Provavelmente falta de ");
        fprintf(stderr, "permissão, tente executar como root");
    }

    // Coloca nome da interface usada em ifr.ifr_name
    memset(&ifr, 0, sizeof(struct ifreq));
    memcpy(ifr.ifr_name, interface, strlen(interface));

    // Pega id da interface e coloca em if_idx.ifr_ifindex
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
        perror("ioctl(): setar id da interface falhou.\n");

    //if (DEBUG)
    //printf("ID: %d\n", ifr.ifr_ifindex);

    // Define configurações para o bind
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sll_family = AF_PACKET;
    sock_addr.sll_ifindex = ifr.ifr_ifindex;
    sock_addr.sll_protocol = htons(ETH_P_ALL);

    // Associa socket à interface de rede, com as configurações definidas
    // e verifica se funcionou.
    if (bind(sockfd,(struct sockaddr* ) &sock_addr, sizeof(sock_addr)) < 0)
        fprintf(stderr, "Erro ao tentar associar o socket à interface de rede \"%s\".\n", interface);

    // Permite receber todos os pacotes do meio compartilhado (vulgo modo promíscuo)
    memset(&pack, 0, sizeof(pack));
    pack.mr_ifindex = ifr.ifr_ifindex;
    pack.mr_type = PACKET_MR_PROMISC;

    if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &pack, sizeof(pack)) < 0) {
        perror("setsockopt(): set opt falhou.\n");
        return -1;
    }

    return sockfd;
}

char espera_resposta(int sockfd)
{
    struct pollfd fds[1];
    char buffer[TAM_MAX_PACOTE];

    // Tentativas quando dá timeout
    int tentativas = 0;
    while (tentativas < TENTATIVAS)
    {
        fds[0].fd = sockfd;
        fds[0].revents = 0;
        fds[0].events = POLLIN;    // pronto para ler

        /*if (poll(fds, 1, 1000) < 0) {
          fprintf(stderr, "Erro: %s\n", strerror(errno));
          return -1;
          }*/
        // guarda eventos prontos em revents
        poll(fds, 1, 1000);

        if (fds[0].revents == POLLHUP)
            printf("POLLHUP\n");
        else if (fds[0].revents == POLLERR)
            printf("POLLERR\n");
        else if (fds[0].revents == POLLNVAL)
            printf("POLLNVAL\n");

        //if ((fds[0].revents & POLLIN) != 0) // Tá pronto pra ler.
        if (fds[0].revents) // Tá pronto pra ler.
        {
            //printf("espera resposta %d\n", escutando);

            if (recv(sockfd, buffer, TAM_MAX_PACOTE, 0) < 0)
                perror("recv: erro");

            if (buffer[0] == 126) {
                int i;
                for (i=0; i<TAM_MAX_PACOTE; i++) {
                    pkt.dados[i] = buffer[i];
                }

                // Retorna o tipo do pacote interpretado ou -1, caso não seja
                // algo válido.
                return interpreta_resposta(pkt, sockfd);
            }
            else {
                if (DEBUG)
                    printf("Leu lixo, timeout=%d++\n", tentativas);
            }
        }
        else {
            if (DEBUG)
                printf("Não tá escutando, timeout=%d++\n", tentativas);
            tentativas++;
        }
    }

    // Desiste
    return -1;
}

char interpreta_resposta(pacote_t pacote, int sockfd) {
    char tipo = tipo_pacote(pacote);
    char tam = tam_pacote(pacote);
    pacote_t resposta;
    int i;

    switch (tipo) {
        case ERRO:
            // switch tipo de erro
            switch (pacote.dados[3]) {
                case INEXISTENTE:
                    printf("ERRO: Arquivo ou diretório não encontrado.\n");
                    break;
                case PERM_NEGADA:
                    printf("ERRO: Permissão negada.\n");
                    break;
                case ESP_INSUFICIENTE:
                    printf("ERRO: Espaço insuficiente.\n");
                    break;
                case DESCONHECIDO:
                    printf("ERRO: Desconhecido.\n");
                    break;
            }
            break;
        case SUCESSO:
            if (DEBUG)
                printf("Comando executado com sucesso.\n");
            break;
        case TAM_ARQUIVO:
            printf("TAM_ARQUIVO\n");
            unsigned long int size;
            size = ((pacote.dados[3] << 24)| (pacote.dados[4] << 16));
            size |= ( (pacote.dados[5] << 8 ) | (pacote.dados[6]));

            // Estou assumindo que tem espaço disponível.
            // TODO responde um nack se não tiver espaço disponivel, integrar com a função da nicolly.
            monta_pacote(NULL, 0, 0, SUCESSO, &resposta);
            envia_pacote(sockfd, resposta, 1);
            break;
        case DADOS:
            fwrite(pacote.dados+3, 1, tam, fp);
            monta_pacote(NULL, 0, 0, ACK, &resposta);
            envia_pacote(sockfd, resposta, 1);
            break;
        case FIM_DADOS:
            fclose(fp);
            break;
        case ACK:
            break;
        case MOSTRA_NA_TELA:
            // Caso o usuário envie algum arquivo com terminador de string, o
            // melhor é imprimir num laço
            for (i=0; i<tam; i++)
                printf("%c", pacote.dados[3+i]);
            monta_pacote(NULL, 0, 0, ACK, &resposta);
            envia_pacote(sockfd, resposta, 1);
            break;
        case FIM_TEXTO:
            // Finaliza o mostra na tela.
            printf("\n");
            break;
        case CRIA_ARQ:
            // Esse é o pacote que envio antes dos de tipo dados
            // Para criar um arquivo com o nome especificado.
            fp = fopen(pacote.dados+3, "w");
            monta_pacote(NULL, 0, 0, ACK, &resposta);
            envia_pacote(sockfd, resposta, 1);
            break;
    }
    return tipo;
}

void erros_stat(int err_cod, char *erros, int sockfd) {
    pacote_t resposta;
    switch (err_cod) {
        case EBADF:
        case EFAULT:
        case ENOENT:
        case ENOTDIR:
            if (DEBUG)
                fprintf(stdout, "ERRO: Arquivo ou diretório não encontrado.\n");
            monta_pacote(erros+INEXISTENTE, 1, 0, ERRO, &resposta);
            envia_pacote(sockfd, resposta, 0);
            break;
        case EACCES:
            if (DEBUG)
                fprintf(stdout, "ERRO: Permissão negada.\n");
            monta_pacote(erros+PERM_NEGADA, 1, 0, ERRO, &resposta);
            envia_pacote(sockfd, resposta, 0);
            break;
        case ELOOP:
        default:
            if (DEBUG)
                fprintf(stdout, "ERRO: Desconhecido.\n");
            monta_pacote(erros+DESCONHECIDO, 1, 0, ERRO, &resposta);
            envia_pacote(sockfd, resposta, 0);
            break;
    }
}


void monta_pacote(char *dados, char tam_dados, char seq, char tipo, pacote_t *pacote)
{
    // Bytes especificados mais 3 de cabeçalho e preambulo
    pacote->tam = tam_dados;
    pacote->dados[0] = PREAMBULO;
    pacote->dados[1] = (tam_dados << 4) | (seq & 0xF);
    pacote->dados[2] = (tipo << 4) | (paridade(dados) & 0xF);
    if (tam_dados > 0) {
        int i;
        for (i=0; i<tam_dados; i++)
            pacote->dados[3+i] = dados[i];
    }
}

void le_pacote(char *s, pacote_t pacote) {
    printf("%s tam=%d ", s, (int) ((pacote.dados[1]>>4) & 0xF));
    printf("seq=%d ", (int) ((pacote.dados[1] & 0xF)));
    printf("tipo=%d ", (int) ((pacote.dados[2]>>4) & 0xF));
    printf("paridade=%d ", (int)(pacote.dados[2] & 0xF));
    printf("dados=%s\n", pacote.dados+3);
}

int envia_pacote(int sockfd, pacote_t pacote, int resp) {
    int tam_pacote = pacote.tam + 3;
    if (send(sockfd, pacote.dados, tam_pacote, 0) < 0) {
        fprintf(stderr, "Erro: %s\n", strerror(errno));
        return -1;
    }

    if (resp) {
        char ret = espera_resposta(sockfd);
        if (ret < 0) {
            printf("Sem confirmação de recebimento.\n");
            return -1;
        }
        else {
            //printf("retorno do espera_resposta em envia pacote %d\n", ret);
            return ret;
        }
    }
    return 0;
}

char tipo_pacote(pacote_t pacote) {
    return ((pacote.dados[2]>>4) & 0xF);
}

char tam_pacote(pacote_t pacote) {
    return ((pacote.dados[1]>>4) & 0xF);
}

// TODO Paridade ímpar
char paridade(char *dados)
{
    return 0;
}

void monta_tam_arq(struct stat buffer, char *tam_arq) {
    unsigned long size = (unsigned long) buffer.st_size;

    tam_arq[3] = (size & 0xFF);
    tam_arq[2] = (size & 0xFF00) >> 8;
    tam_arq[1] = (size & 0xFF0000) >> 16;
    tam_arq[0] = (size >> 24);
}
