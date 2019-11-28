all: server cliente

server: Servidor.c
	gcc -o server Servidor.c -lpthread -lrt

cliente: Cliente.c
	gcc -o cliente Cliente.c -lpthread -lrt

clean:
	rm server cliente
