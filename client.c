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

bool is_game_over = false;

bool handle_out_of_bounds(int *row, int *col) {
  *row = atoi(strtok(NULL, ","));
  *col = atoi(strtok(NULL, ","));
  return row[0] < 0 || row[1] < 0 || row[0] >= BOARD_SIZE ||
         row[1] >= BOARD_SIZE;
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

  while (!is_game_over) {
    Action action;
    char buffer[MAX_CMD_SIZE];

  reread:
    fgets(buffer, MAX_CMD_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    char *command = strtok(buffer, " ,");
    if (strcmp(command, "start") == 0) {
      action.type = START;
      action.coordinates[0] = 0;
      action.coordinates[1] = 0;
    } else if (strcmp(command, "reveal") == 0) {
      action.type = REVEAL;
      if (handle_out_of_bounds(&action.coordinates[0],
                               &action.coordinates[1])) {
        printf("error: invalid cell\n");
        goto reread;
      }
      int row = action.coordinates[0];
      int col = action.coordinates[1];
      if (action.board[row][col] != HIDDEN_CELL) {
        printf("error: cell already revealed\n");
        goto reread;
      }
    } else if (strcmp(command, "flag") == 0) {
      action.type = FLAG;
      if (handle_out_of_bounds(&action.coordinates[0],
                               &action.coordinates[1])) {
        printf("error: invalid cell\n");
        goto reread;
      }
      int row = action.coordinates[0];
      int col = action.coordinates[1];
      if (action.board[row][col] == FLAGGED_CELL) {
        printf("error: cell already has a flag\n");
        goto reread;
      }
      if (action.board[row][col] != HIDDEN_CELL) {
        printf("error: cannot insert flag in revealed cell\n");
        goto reread;
      }
    } else if (strcmp(command, "remove_flag") == 0) {
      action.type = REMOVE_FLAG;
      if (handle_out_of_bounds(&action.coordinates[0],
                               &action.coordinates[1])) {
        printf("error: invalid cell\n");
        goto reread;
      }
    } else if (strcmp(command, "reset") == 0) {
      action.type = RESET;
      printf("starting new game\n");
    }
    if (send(client_socket, &action, sizeof(Action), 0) == -1) {
      exit(EXIT_FAILURE);
    }
    ssize_t bytes_received = recv(client_socket, &action, sizeof(Action), 0);
    if (bytes_received == -1) {
      exit(EXIT_FAILURE);
    }
    if (action.type == WIN) {
      is_game_over = true;
      printf("YOU WIN!\n");
    } else if (action.board[action.coordinates[0]][action.coordinates[1]] ==
               BOMB_CELL) {
      is_game_over = true;
      printf("GAME OVER!\n");
    }
    print_board(action.board);
  }

  close(client_socket);
}
