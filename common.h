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
};

#define BOMB_CELL (-1)
#define HIDDEN_CELL (-2)
#define MARKED_CELL (-3)

#define BOARD_SIZE 4

typedef struct Action {
  int type;
  int coordinates[2];
  int board[BOARD_SIZE][BOARD_SIZE];
} Action;
