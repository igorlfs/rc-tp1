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

#define ARG_PROTOCOL_VERSION 1
#define ARG_PORT 2
#define ARG_INPUT_FILE 4

#define MAX_LINE_SIZE 100

#define MAX_PENDING 5

bool is_game_over = false;

void read_input_file(char *file_path, int board[BOARD_SIZE][BOARD_SIZE]) {
  FILE *file_handler = fopen(file_path, "r");

  if (file_handler == NULL) {
    exit(EXIT_FAILURE);
  }

  char line[MAX_LINE_SIZE];
  char token[MAX_LINE_SIZE];

  for (int i = 0; i < BOARD_SIZE; ++i) {
    fgets(line, MAX_LINE_SIZE, file_handler);
    strcpy(token, strtok(line, ","));

    board[i][0] = atoi(token);

    for (int j = 1; j < BOARD_SIZE; ++j) {
      strcpy(token, strtok(NULL, ","));

      board[i][j] = atoi(token);
    }
  }

  fclose(file_handler);
}

int get_protocol(char *string) {
  if (strcmp(string, "v4") == 0) {
    return AF_INET;
  }
  if (strcmp(string, "v6") == 0) {
    return AF_INET6;
  }
  exit(EXIT_FAILURE);
}

void start_action(Action *action) {
  action->type = STATE;
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      action->board[i][j] = HIDDEN_CELL;
    }
  }
}

void reveal_all(Action *action, int board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      action->board[i][j] = board[i][j];
    }
  }
}

void reveal_action(Action *action, int board[BOARD_SIZE][BOARD_SIZE]) {
  action->type = STATE;
  int row = action->coordinates[0];
  int col = action->coordinates[1];
  if (board[row][col] == BOMB_CELL) {
    is_game_over = true;
    reveal_all(action, board);
  } else {
    action->board[row][col] = board[row][col];
    int not_bombs_counter = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) {
      for (int j = 0; j < BOARD_SIZE; ++j) {
        if (action->board[i][j] >= 0) {
          not_bombs_counter++;
        }
      }
    }
    if (not_bombs_counter == NOT_BOMBS) {
      action->type = WIN;
      is_game_over = false;
      reveal_all(action, board);
    }
  }
}

void flag_action(Action *action) {
  action->type = STATE;
  int row = action->coordinates[0];
  int col = action->coordinates[1];
  action->board[row][col] = FLAGGED_CELL;
}

void remove_flag_action(Action *action) {
  // Tecnicamente a gente deveria indicar que altera o estado só se ele
  // realmente for alterado
  action->type = STATE;
  int row = action->coordinates[0];
  int col = action->coordinates[1];
  if (action->board[row][col] == FLAGGED_CELL) {
    action->board[row][col] = HIDDEN_CELL;
  }
}

int main(int argc, char *argv[]) {
  int port = atoi(argv[ARG_PORT]);
  int protocol = get_protocol(argv[ARG_PROTOCOL_VERSION]);

  if (argc - 1 != ARG_INPUT_FILE) {
    exit(EXIT_FAILURE);
  }

  int initial_board[BOARD_SIZE][BOARD_SIZE];
  read_input_file(argv[ARG_INPUT_FILE], initial_board);
  print_board(initial_board);

  int server_socket = socket(protocol, SOCK_STREAM, 0);
  if (server_socket == -1) {
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = protocol;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == -1) {
    exit(EXIT_FAILURE);
  }

  int client_socket;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

reconnect:
  if (listen(server_socket, MAX_PENDING) == -1) {
    exit(EXIT_FAILURE);
  }

  client_socket =
      accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_socket == -1) {
    exit(EXIT_FAILURE);
  }
  printf("client connected\n");

  while (true) {
    Action action;
    ssize_t bytes_received = recv(client_socket, &action, sizeof(Action), 0);
    if (bytes_received == -1) {
      exit(EXIT_FAILURE);
    }

    switch (action.type) {
    case START:
      start_action(&action);
      break;
    case REVEAL:
      reveal_action(&action, initial_board);
      break;
    case FLAG:
      flag_action(&action);
      break;
    case REMOVE_FLAG:
      remove_flag_action(&action);
      break;
    case RESET:
      start_action(&action);
      break;
    case EXIT:
      printf("client disconnected\n");
    }

    if (send(client_socket, &action, sizeof(Action), 0) == -1) {
      exit(EXIT_FAILURE);
    }

    if (action.type == EXIT) {
      close(client_socket);
      goto reconnect;
    }
  }

  close(client_socket);
  close(server_socket);
}
