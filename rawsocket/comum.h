#ifndef _COMUM_H_
#define _COMUM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
/* close() - chdir() */
#include <unistd.h>
/* errno */
#include <errno.h>
/* socket() - setsockopt() - bind() - recv() - send() */
#include <sys/socket.h>
/* Tipos de packets */
#include <netpacket/packet.h>
/* ioctl() */
#include <sys/ioctl.h>
/* IFNAMSIZ - IFF_PROMISC */
#include <net/if.h>
/* select() */
#include <sys/select.h>
/* htons() */
#include <arpa/inet.h>
/* ETH_P_ALL */
#include <net/ethernet.h>

#include <poll.h>
#include <errno.h>


// Tipos pacotes
#define PREAMBULO 0x7E
#define CD 0
#define LS 1
#define PUT 2
#define GET 3
#define CAT 4
#define PWD 5
#define FIM_TEXTO 6
#define DADOS 7
#define TAM_ARQUIVO 8
#define MOSTRA_NA_TELA 9
#define ACK 10
#define FIM_DADOS 11
#define CRIA_ARQ 12 // enviado antes de dados
#define SUCESSO 13
#define ERRO 14
#define NACK 15

#define PWD 5

// Tipos erros
#define INEXISTENTE 0
#define PERM_NEGADA 1
#define ESP_INSUFICIENTE 2
#define DESCONHECIDO 3

#define TAM_MAX_PACOTE 18
#define TAM_MAX_DADOS 15

#define TENTATIVAS 16
#define SEQUENCIA 16

#define DEBUG 0

/* Estruturas */
typedef struct pacote {
    // Preambulo, tamanho, sequencia, tipo, detecção/paridade e dados
    char dados[TAM_MAX_PACOTE];
    char tam;
} pacote_t;

/* Funções comuns entre o servidor e o cliente */
int abre_socket(char * interface);
char espera_resposta(int sockfd);

void monta_pacote(char *dados, char tam_dados, char seq, char tipo, pacote_t *pacote);
void le_pacote(char *s, pacote_t pacote);
int envia_pacote(int sockfd, pacote_t pacote, int resp);
char tipo_pacote(pacote_t pacote);
char tam_pacote(pacote_t pacote);
char interpreta_resposta(pacote_t pacote, int sockfd);

char paridade(char *dados);

void erros_stat(int ret, char *erros, int sockfd);
void monta_tam_arq(struct stat buffer, char *tam_arq);

#endif
