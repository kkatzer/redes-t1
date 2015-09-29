CC = gcc
CFLAGS = -Wall

all:
	        $(CC) $(CFLAGS) utils.c servidor.c ConexaoRawSocket.c -o servidor
	        $(CC) $(CFLAGS) utils.c cliente.c ConexaoRawSocket.c -o cliente

install:

clean:
	        rm -rf cliente servidor