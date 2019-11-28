all: server cliente

debug: server_g cliente_g

server: servidor.c
	gcc -o server servidor.c -lpthread

server_g: servidor.c
	gcc -g -o server servidor.c -lpthread

cliente: cliente.c
	gcc -o cliente cliente.c

cliente_g: cliente.c
	gcc -g -o cliente cliente.c

clean:
	rm server cliente
