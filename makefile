all: client server

client: client.c common.c common.h
	clang client.c common.c -o client -Wall -g

server: server.c common.c common.h
	clang server.c common.c -o server -Wall -g

clean:
	rm client
	rm server
