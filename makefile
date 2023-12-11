all: init client server

BIN=bin
CLN=$(BIN)/client
SRV=$(BIN)/server
PORT=51511
INPUT_FILE=input/jogo1.csv
IP=127.0.0.1

init:
	@ test -d $(BIN) && true || mkdir $(BIN)

client: client.c common.c common.h
	gcc client.c common.c -o $(CLN) -Wall -g

server: server.c common.c common.h
	gcc server.c common.c -o $(SRV) -Wall -g

clean:
	test ! -d $(BIN) && true || rm -rf $(BIN) 

run_server: init server
	./$(SRV) v4 $(PORT) -i $(INPUT_FILE)

run_client: init client
	./$(CLN) $(IP) $(PORT)

.PHONY: all init client server clean run_server run_client
