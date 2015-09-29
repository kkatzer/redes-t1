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

const char *device = "eth0";

#endif
