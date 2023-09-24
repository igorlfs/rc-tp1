all: client server

client: client.c common.c common.h
	chmod +x client.sh
	./client.sh

server: server.c common.c common.h
	chmod +x server.sh
	./server.sh

clean:
	rm client
	rm server
