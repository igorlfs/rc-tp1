#include "common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define ARG_IP 1
#define ARG_PORT 2

#define MAX_CMD_SIZE 100

void print_board(int board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      int current = board[i][j];
      char printable;
      switch (current) {
      case -1:
        printable = '*';
        break;
      case -2:
        printable = '-';
        break;
      case -3:
        printable = '>';
        break;
      default:
        printable = (char)current + '0';
      }
      printf("%c", printable);
      if (j != BOARD_SIZE - 1) {
        printf("\t\t");
      }
    }
    putchar('\n');
  }
}

int main(int argc, char *argv[]) {
  int port = atoi(argv[ARG_PORT]);
  char *ip_address = argv[ARG_IP];

  if (argc - 1 != ARG_PORT) {
    exit(EXIT_FAILURE);
  }

  // TODO(igorlfs) usar IPV6 (AF_INET6) se for o caso
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  // TODO(igorlfs) usar IPV6 se for o caso
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip_address);

  if (connect(client_socket, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) == -1) {
    exit(EXIT_FAILURE);
  }

  while (true) {
    Action action;
    char buffer[MAX_CMD_SIZE];

    fgets(buffer, MAX_CMD_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    char *command = strtok(buffer, " ");
    if (strcmp(command, "start") == 0) {
      action.type = START;
    }
    if (send(client_socket, &action, sizeof(Action), 0) == -1) {
      exit(EXIT_FAILURE);
    }
    ssize_t bytes_received = recv(client_socket, &action, sizeof(Action), 0);
    if (bytes_received == -1) {
      exit(EXIT_FAILURE);
    }
    print_board(action.board);
  }

  close(client_socket);
}
