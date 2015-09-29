#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INICIO 0x7E
#define ACK 0
#define OK 1
#define DADOS 2
#define CD 10
#define LS 11
#define GET 12
#define PUT 13
#define DESCRITOR 28
#define EOF 29
#define ERR 30
#define NACK 31

#define TAM_MAX_DADOS 63
#define TAM_MAX_PACOTE 67

const char *device = "eth0";

typedef struct pacote {
	//Início, tamanho, sequencia, tipo, dados e paridade
    char dados[TAM_MAX_PACOTE];
    char tam;
} pacote_t;

void cria_pacote(char *dados, char tam, char seq, char tipo, pacote_t *pacote);
void le_pacote(char *s, pacote_t pacote);

#endif
