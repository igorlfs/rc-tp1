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

bool is_connected = true;

/// Modifica o tipo de ação de `action` com base no comando de `line`.
void set_action_type(char line[MAX_CMD_SIZE], Action *action) {
  char *command = strtok(line, " ,");
  if (strcmp(command, "start") == 0) {
    action->type = START;
  } else if (strcmp(command, "reveal") == 0) {
    action->type = REVEAL;
  } else if (strcmp(command, "flag") == 0) {
    action->type = FLAG;
  } else if (strcmp(command, "remove_flag") == 0) {
    action->type = REMOVE_FLAG;
  } else if (strcmp(command, "reset") == 0) {
    action->type = RESET;
  } else if (strcmp(command, "exit") == 0) {
    action->type = EXIT;
  }
}

/// Trate os erros de entrada locais.
bool handle_local_errors(Action *action) {
  int row;
  int col;

  if (action->type == UNKNOWN) {
    printf("error: command not found\n");
    return true;
  }

  if (action->type == REVEAL || action->type == FLAG ||
      action->type == REMOVE_FLAG) {
    action->coordinates[0] = atoi(strtok(NULL, ","));
    action->coordinates[1] = atoi(strtok(NULL, ","));

    row = action->coordinates[0];
    col = action->coordinates[1];

    if (row < 0 || col < 0 || row >= BOARD_SIZE || col >= BOARD_SIZE) {
      printf("error: invalid cell\n");
      return true;
    }
  }

  if (action->type == REVEAL || action->type == FLAG) {
    if (action->board[row][col] != HIDDEN_CELL && action->type == REVEAL) {
      printf("error: cell already revealed\n");
      return true;
    }
    if (action->board[row][col] == FLAGGED_CELL && action->type == FLAG) {
      printf("error: cell already has a flag\n");
      return true;
    }
    if (action->board[row][col] != HIDDEN_CELL && action->type == FLAG) {
      printf("error: cannot insert flag in revealed cell\n");
      return true;
    }
  }

  return false;
}

/// Lê instruções do usuário, faz o tratamento da entrada, envia comandos ao
/// servidor, e imprime mensagens ou o campo, se necessário (em loop).
void game_loop(int client_socket) {
  while (is_connected) {
    Action action;
    char line[MAX_CMD_SIZE];

    action.type = UNKNOWN;

    fgets(line, MAX_CMD_SIZE, stdin);

    // Come a quebra de linha que vem com o fgets
    line[strcspn(line, "\n")] = 0;

    set_action_type(line, &action);

    if (handle_local_errors(&action)) {
      continue;
    }

    if (send(client_socket, &action, sizeof(Action), 0) == -1) {
      exit(EXIT_FAILURE);
    }

    ssize_t bytes_received = recv(client_socket, &action, sizeof(Action), 0);
    if (bytes_received == -1) {
      exit(EXIT_FAILURE);
    }

    if (action.type == WIN || action.type == GAME_OVER) {
      is_connected = false;

      if (action.type == WIN) {
        printf("YOU WIN!\n");
      } else {
        printf("GAME OVER!\n");
      }
    }

    if (action.type != EXIT) {
      print_board(action.board);
    } else {
      is_connected = false;
    }
  }
}

int main(int argc, char *argv[]) {
  // Garante a quantidade de argumentos para o programa funcionar
  if (argc - 1 != ARG_PORT) {
    exit(EXIT_FAILURE);
  }

  int port = atoi(argv[ARG_PORT]);
  char *ip_address = argv[ARG_IP];

  // Define qual protocolo está sendo usado
  struct in_addr inaddr4;
  struct in6_addr inaddr6;
  int protocol = AF_INET;
  if (inet_pton(AF_INET, ip_address, &inaddr4) != 0) {
    protocol = AF_INET;
  } else if (inet_pton(AF_INET6, ip_address, &inaddr6) != 0) {
    protocol = AF_INET6;
  } else {
    exit(EXIT_FAILURE);
  }

  // Criando a socket
  int client_socket = socket(protocol, SOCK_STREAM, 0);
  if (client_socket == -1) {
    exit(EXIT_FAILURE);
  }

  // Definindo servidor e porta, com base no protocolo
  struct sockaddr_storage server_addr;
  if (protocol == AF_INET) {
    struct sockaddr_in *ipv4_addr = (struct sockaddr_in *)&server_addr;

    ipv4_addr->sin_family = AF_INET;
    ipv4_addr->sin_port = htons(port);
    ipv4_addr->sin_addr = inaddr4;
  } else if (protocol == AF_INET6) {
    struct sockaddr_in6 *ipv6_addr = (struct sockaddr_in6 *)&server_addr;

    ipv6_addr->sin6_family = AF_INET6;
    ipv6_addr->sin6_port = htons(port);
    ipv6_addr->sin6_addr = inaddr6;
  } else {
    exit(EXIT_FAILURE);
  }

  // Conectando ao servidor
  if (connect(client_socket, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) == -1) {
    exit(EXIT_FAILURE);
  }

  game_loop(client_socket);

  close(client_socket);
}
