#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define ROWS 10
#define COLS 10
#define MAX_BOXES 100

enum base {NONE, WALL, STORAGE};

struct pos {
  int row;
  int col;
};

struct reader {
  enum base board[ROWS][COLS];
  int n_boxes;
  struct pos links[MAX_BOXES][MAX_BOXES];
  int link_sizes[MAX_BOXES];
};

struct state {
  struct pos player_pos;
  int boxes[ROWS][COLS];
  int move_counter;
};

void add_wall(struct reader *reader, struct state *state, int row, int col);
void add_walls(struct reader *reader, struct state *state, int r1, int c1, int r2, int c2);
void add_box(struct reader *reader, struct state *state, int row, int col);
void add_storage(struct reader *reader, struct state *state, int row, int col);
void link_boxes(struct reader *reader, struct state *state, int r1, int c1, int r2, int c2);
void print_board(struct reader *reader, struct state *state);
void gameplay(struct reader *reader, struct state *state);

int main(void) {
  struct reader reader = {{}, 0, {}, {}};
  struct state state = {{-1, -1}, {}, 0};

  printf("=== Level Setup ===\n");
  char c;
  while (scanf(" %c", &c) == 1 && c != 'q') {
    if (c == 'w') {
      int row, col;
      scanf("%d %d", &row, &col);
      add_wall(&reader, &state, row, col);
    } else if (c == 'b') {
      int row, col;
      scanf("%d %d", &row, &col);
      add_box(&reader, &state, row, col);
    } else if (c == 's') {
      int row, col;
      scanf("%d %d", &row, &col);
      add_storage(&reader, &state, row, col);
    } else if (c == 'W') {
      int r1, c1, r2, c2;
      scanf("%d %d %d %d", &r1, &c1, &r2, &c2);
      add_walls(&reader, &state, r1, c1, r2, c2);
    } else if (c == 'l') {
      int r1, r2, c1, c2;
      scanf("%d %d %d %d", &r1, &c1, &r2, &c2);
      link_boxes(&reader, &state, r1, c1, r2, c2);
    }
    print_board(&reader, &state);
  }

  printf("Enter player starting position: ");
  scanf("%d %d", &state.player_pos.row, &state.player_pos.col);

  printf("\n=== Starting Sokoban! ===\n");
  print_board(&reader, &state);

  gameplay(&reader, &state);
}

int add_single_wall(struct reader *reader, struct state *state, int row, int col) {
    reader->board[row][col] = WALL;
}

void add_wall(struct reader *reader, struct state *state, int row, int col) {
  add_single_wall(reader, state, row, col);
}

void add_walls(struct reader *reader, struct state *state, int r1, int c1, int r2, int c2) {
  for (int row = r1; row <= r2; row++) {
    for (int col = c1; col <= c2; col++) {
      add_single_wall(reader, state, row, col);
    }
  }
}

void add_storage(struct reader *reader, struct state *state, int row, int col) {
  reader->board[row][col] = STORAGE;
}

void add_box(struct reader *reader, struct state *state, int r, int c) {
  reader->n_boxes++;
  int n = reader->n_boxes;
  state->boxes[r][c] = n;
  struct pos b = {r, c};
  reader->links[n][0] = b;
  reader->link_sizes[n] = 1;
}

void link_boxes(struct reader *reader, struct state *state, int r1, int c1, int r2, int c2) {
  struct pos b1 = {r1, c1};
  struct pos b2 = {r2, c2};

  int l1 = state->boxes[r1][c1];
  int l2 = state->boxes[r2][c2];

  if (l1 != l2) {
    int m = reader->link_sizes[l1];
    int n = reader->link_sizes[l2];
    for (int i = 0; i < n; i++) {
      struct pos b = reader->links[l2][i];
      state->boxes[b.row][b.col] = l1;
      reader->links[l1][m + i] = b;
    } 
    reader->link_sizes[l1] += n;
  }
}

bool pos_eq(struct pos p1, struct pos p2) {
  return p1.row == p2.row && p1.col == p2.col;
}

void move_box(struct reader *reader, struct state *state, struct pos box, int dr, int dc, int seen[ROWS][COLS]) {
  int r = box.row;
  int c = box.col;
  if (seen[r][c]) return;
  seen[r][c] = 1;
  int b = state->boxes[r][c];
  if (b == 0) {
    if (reader->board[r][c] == WALL) {
      seen[r][c] = 2;
    } 
    return;
  }
  int rr = (r + dr + ROWS) % ROWS;
  int cc = (c + dc + COLS) % COLS;
  struct pos pp = {rr, cc};
  state->boxes[r][c] = 0;
  move_box(reader, state, pp, dr, dc, seen);
  if (seen[rr][cc] == 1) {
    state->boxes[rr][cc] = b;
    for (int i = 0; i < reader->link_sizes[b]; i++) {
      if (pos_eq(reader->links[b][i], box)) {
        reader->links[b][i] = pp;
      }
    }
  } else {
    state->boxes[r][c] = b;
    seen[r][c] = 2;
  }
  for (int i = 0; i < reader->link_sizes[b]; i++) {
    move_box(reader, state, reader->links[b][i], dr, dc, seen);
  }
}

void move_player(struct reader *reader, struct state *state, int dr, int dc) {
  int r = state->player_pos.row;
  int c = state->player_pos.col;
  struct pos new_pos = {(r + dr + ROWS) % ROWS, (c + dc + COLS) % COLS};
  int seen[ROWS][COLS] = {};
  move_box(reader, state, new_pos, dr, dc, seen);
  if (seen[new_pos.row][new_pos.col] == 1) {
    state->player_pos = new_pos;
  }
}

void print_counter(struct reader *reader, struct state *state) {
  printf("Number of moves so far: %d\n", state->move_counter);
}

int is_won(struct reader *reader, struct state *state) {
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      enum base base = reader->board[r][c];
      int box = state->boxes[r][c];
      if (box != 0 && base != STORAGE) {
        return false;
      }
    }
  }
  return true;
}

void undo(struct reader *reader, struct state *state) {
  return;
}

void gameplay(struct reader *reader, struct state *state) {
  char c;
  while (scanf(" %c", &c) == 1) {
    int dr, dc;
    switch (c) {
    case 'w':
      dr = -1;
      dc = 0;
      move_player(reader, state, dr, dc);
      break;
    case 'a':
      dr = 0;
      dc = -1;
      move_player(reader, state, dr, dc);
      break;
    case 's':
      dr = 1;
      dc = 0;
      move_player(reader, state, dr, dc);
      break;
    case 'd':
      dr = 0;
      dc = 1;
      move_player(reader, state, dr, dc);
      break;
    case 'c':
      print_counter(reader, state);
      break;
    case 'r':
      printf("=== Resetting Game ===\n");
      break;
    case 'u':
      break;
    }
    print_board(reader, state);
    for (int r = 0; r < 10; r++) {
      for (int c = 0; c < 10; c++) {
        printf("(%d", reader->links[r][c].row);
        printf(",%d)", reader->links[r][c].col);
      }
      printf("\n");
    }
    if (is_won(reader, state)) {
      if (state->move_counter == 1) {
        printf("=== Level Solved in 1 Move! ===\n");
      } else {
        printf("=== Level Solved in %d Moves! ===\n", state->move_counter);
      }
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////                           PROVIDED FUNCTIONS                           ////
////////////////////////////////////////////////////////////////////////////////

// NOTE: You can edit these if you wish.

// prints a line the width of the sokoban board.
void print_line() {
  for (int i = 0; i < COLS * 4 + 1; i++) {
    printf("-");
  }
  printf("\n");
}

// prints out the title for above the sokoban board.
void print_title() {
  print_line();
  char *title = "S O K O B A N";
  int len = COLS * 4 + 1;
  int n_white = len - strlen(title) - 2;
  printf("|");
  for (int i = 0; i < n_white / 2; i++) {
    printf(" ");
  }
  printf("%s", title);
  for (int i = 0; i < (n_white + 1) / 2; i++) {
    printf(" ");
  }
  printf("|\n");
}

// Prints out the current state of the sokoban board.
// will place the player on the board at position p_row, p_col.
// If player position is out of bounds, it won't place a player anywhere.
void print_board(struct reader *reader, struct state *state) {
  print_title();
  for (int r  = 0; r < ROWS; r++) {
    print_line();
    for (int c = 0; c < COLS; c++) {
      printf("|");
      enum base base = reader->board[r][c];
      int box = state->boxes[r][c];
      if (r == state->player_pos.row && c == state->player_pos.col) {
        printf("^_^");
      } else if (base == WALL) {
        printf("===");
      } else if (box == 0 && base == STORAGE) {
        printf(" . ");
      } else if (base == STORAGE) {
        printf("[.]");
      } else if (box == 0) {
        printf("   ");
      } else {
        printf("[%d]", box);
      }
    }
    printf("|\n");
  }
  print_line();
  printf("\n");
}
