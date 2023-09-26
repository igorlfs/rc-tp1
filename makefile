all: init client server

BIN=bin
CLN=$(BIN)/client
SRV=$(BIN)/server

init:
	@ test -d $(BIN) && true || mkdir $(BIN)

client: client.c common.c common.h
	gcc client.c common.c -o $(CLN) -Wall -g

server: server.c common.c common.h
	gcc server.c common.c -o $(SRV) -Wall -g

clean:
	test ! -d $(BIN) && true || rm -rf $(BIN) 

.PHONY: all init client server clean
