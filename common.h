#pragma once

enum ACTION_TYPES {
  START,
  REVEAL,
  FLAG,
  STATE,
  REMOVE_FLAG,
  RESET,
  WIN,
  EXIT,
  GAME_OVER,
  UNKNOWN,
};

#define BOMB_CELL (-1)
#define HIDDEN_CELL (-2)
#define FLAGGED_CELL (-3)

#define BOARD_SIZE 4
#define NUM_BOMSS 3
#define NOT_BOMBS ((BOARD_SIZE) * (BOARD_SIZE) - (NUM_BOMSS))

typedef struct Action {
  int type;
  int coordinates[2];
  int board[BOARD_SIZE][BOARD_SIZE];
} Action;

void print_board(int board[BOARD_SIZE][BOARD_SIZE]);
