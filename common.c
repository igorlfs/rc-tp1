#include "common.h"
#include <stdio.h>

/// Imprime o campo `board` da maneira especificada.
void print_board(int board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      int current = board[i][j];
      char printable;
      switch (current) {
      case BOMB_CELL:
        printable = '*';
        break;
      case HIDDEN_CELL:
        printable = '-';
        break;
      case FLAGGED_CELL:
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
