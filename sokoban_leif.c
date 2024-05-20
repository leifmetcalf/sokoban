#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ROWS 10
#define COLS 10
#define MAX_BOXES 100
#define MAX_HISTORY 1000

enum base { NONE, WALL, STORAGE };

struct pos {
  int row;
  int col;
};

struct state {
  struct pos player_pos;
  int boxes[ROWS][COLS];
  struct pos box_pos[MAX_BOXES];
};

struct reader {
  enum base board[ROWS][COLS];
  int n_boxes;
  int box_link[MAX_BOXES];
  int links[MAX_BOXES][MAX_BOXES];
  int link_sizes[MAX_BOXES];
  int move_counter;
  struct state history[MAX_HISTORY];
};

int in_bounds(int r, int c) {
  return (0 <= r && r < ROWS && 0 <= c && c < COLS);
}

int add_single_wall(struct reader *reader, struct state *state, int r, int c) {
  if (in_bounds(r, c)) {
    reader->board[r][c] = WALL;
    return true;
  }
  return false;
}

void print_invalid_loc() { printf("Location out of bounds\n"); }

void add_wall(struct reader *reader, struct state *state, int r, int c) {
  if (!add_single_wall(reader, state, r, c)) {
    print_invalid_loc();
  }
}

void add_walls(struct reader *reader, struct state *state, int r1, int c1,
               int r2, int c2) {
  if (!(in_bounds(r1, c1) || in_bounds(r2, c2))) {
    print_invalid_loc();
    return;
  }
  for (int r = r1; r <= r2; r++) {
    for (int c = c1; c <= c2; c++) {
      add_single_wall(reader, state, r, c);
    }
  }
}

void add_storage(struct reader *reader, struct state *state, int r, int c) {
  if (!in_bounds(r, c)) {
    print_invalid_loc();
    return;
  }
  reader->board[r][c] = STORAGE;
}

void add_box(struct reader *reader, struct state *state, int r, int c) {
  if (!in_bounds(r, c)) {
    print_invalid_loc();
    return;
  }
  if (reader->board[r][c] == WALL) {
    reader->board[r][c] = NONE;
  }
  reader->n_boxes++;
  int n = reader->n_boxes;
  reader->box_link[n] = n;
  reader->links[n][0] = n;
  reader->link_sizes[n] = 1;
  state->boxes[r][c] = n;
  struct pos p = {r, c};
  state->box_pos[n] = p;
}

void link_boxes(struct reader *reader, struct state *state, int r1, int c1,
                int r2, int c2) {
  if (!in_bounds(r1, c1) || !in_bounds(r2, c2)) {
    printf("Invalid Location(s)\n");
    return;
  }
  int b1 = state->boxes[r1][c1];
  int b2 = state->boxes[r2][c2];
  if (b1 == 0 || b2 == 0) {
    printf("Location not box(s)\n");
  }
  int l1 = reader->box_link[b1];
  int l2 = reader->box_link[b2];
  if (l1 == l2) {
    return;
  }
  int l1_len = reader->link_sizes[l1];
  int l2_len = reader->link_sizes[l2];
  for (int i = 0; i < l2_len; i++) {
    int b = reader->links[l2][i];
    reader->box_link[b] = l1;
    reader->links[l1][l1_len + i] = b;
  }
  reader->link_sizes[l1] += l2_len;
}

struct pos pos_plus(struct pos p, char d) {
  if (d == 'w') {
    p.row--;
  } else if (d == 'a') {
    p.col--;
  } else if (d == 's') {
    p.row++;
  } else if (d == 'd') {
    p.col++;
  }
  p.row = (p.row + ROWS) % ROWS;
  p.col = (p.col + COLS) % COLS;
  return p;
}

void move_box(struct reader *reader, struct state *state, struct pos p, char d,
              int seen[ROWS][COLS]) {
  int r = p.row;
  int c = p.col;
  if (seen[r][c])
    return;
  seen[r][c] = 1;
  int b = state->boxes[r][c];
  if (b == 0) {
    if (reader->board[r][c] == WALL) {
      seen[r][c] = 2;
    }
    return;
  }
  struct pos pp = pos_plus(p, d);
  int rr = pp.row;
  int cc = pp.col;
  state->boxes[r][c] = 0;
  move_box(reader, state, pp, d, seen);
  if (seen[rr][cc] == 1) {
    state->boxes[rr][cc] = b;
    state->box_pos[b] = pp;
  } else {
    state->boxes[r][c] = b;
    seen[r][c] = 2;
  }
  for (int i = 0; i < reader->link_sizes[b]; i++) {
    move_box(reader, state, state->box_pos[reader->links[b][i]], d, seen);
  }
}

void move_player(struct reader *reader, struct state *state, char d) {
  struct pos new_pos = pos_plus(state->player_pos, d);
  int seen[ROWS][COLS] = {};
  move_box(reader, state, new_pos, d, seen);
  if (seen[new_pos.row][new_pos.col] == 1) {
    state->player_pos = new_pos;
  }
  bool moved = false;
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      if (seen[r][c] == 1) {
        moved = true;
      }
    }
  }
  if (moved) {
    reader->move_counter++;
    reader->history[reader->move_counter] = *state;
  }
}

void print_counter(struct reader *reader, struct state *state) {
  printf("Number of moves so far: %d\n", reader->move_counter);
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
  reader->move_counter--;
  *state = reader->history[reader->move_counter];
}

void reset_game(struct reader *reader, struct state *state) {
  printf("=== Resetting Game ===\n");
  reader->move_counter = 0;
  *state = reader->history[0];
}

void print_line() {
  for (int i = 0; i < COLS * 4 + 1; i++) {
    printf("-");
  }
  printf("\n");
}

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

void print_board(struct reader *reader, struct state *state) {
  print_title();
  for (int r = 0; r < ROWS; r++) {
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
        printf("[ ]");
      }
    }
    printf("|\n");
  }
  print_line();
  printf("\n");
}

void gameplay(struct reader *reader, struct state *state) {
  char c;
  while (scanf(" %c", &c) == 1) {
    switch (c) {
    case 'w':
    case 'a':
    case 's':
    case 'd':
      move_player(reader, state, c);
      break;
    case 'c':
      print_counter(reader, state);
      break;
    case 'r':
      reset_game(reader, state);
      break;
    case 'u':
      undo(reader, state);
      break;
    }
    print_board(reader, state);
    if (is_won(reader, state)) {
      if (reader->move_counter == 1) {
        printf("=== Level Solved in 1 Move! ===\n");
      } else {
        printf("=== Level Solved in %d Moves! ===\n", reader->move_counter);
      }
      return;
    }
  }
}

int main(void) {
  struct reader reader = {{}, 0, {}, {}, {}, 0, {}};
  struct state state = {{-1, -1}, {}, {}};

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
  reader.history[0] = state;
  print_board(&reader, &state);

  gameplay(&reader, &state);
}
