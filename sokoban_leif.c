#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ROWS 10
#define COLS 10
#define MAX_BOXES 100
#define MAX_HISTORY 1000

enum base { NONE, WALL, STORAGE };

enum seen { FRESH, LINKED, MARKED, STOPPED };

struct pos {
  int row;
  int col;
};

struct state {
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

bool in_bounds(int r, int c) {
  return (0 <= r && r < ROWS && 0 <= c && c < COLS);
}

bool add_single_wall(struct reader *reader, struct state *state, int r, int c) {
  if (in_bounds(r, c)) {
    reader->board[r][c] = WALL;
    int b = state->boxes[r][c];
    if (b > 0) {
      reader->n_boxes--;
      int l = reader->box_link[b];
      for (int i = 0; i < reader->link_sizes[l]; i++)
        if (reader->links[l][i] == b) {
          for (int j = i + 1; j < reader->link_sizes[l]; j++)
            reader->links[l][j - 1] = reader->links[l][j];
          break;
        }
      reader->link_sizes[l]--;
      state->boxes[r][c] = 0;
    }
    return true;
  }
  return false;
}

void print_invalid_loc() { printf("Location out of bounds\n"); }

void add_wall(struct reader *reader, struct state *state, int r, int c) {
  if (!add_single_wall(reader, state, r, c))
    print_invalid_loc();
}

void add_walls(struct reader *reader, struct state *state, int r1, int c1,
               int r2, int c2) {
  if (!(in_bounds(r1, c1) || in_bounds(r2, c2))) {
    print_invalid_loc();
    return;
  }
  for (int r = r1; r <= r2; r++)
    for (int c = c1; c <= c2; c++)
      add_single_wall(reader, state, r, c);
}

void add_storage(struct reader *reader, struct state *state, int r, int c) {
  if (!in_bounds(r, c)) {
    print_invalid_loc();
    return;
  }
  reader->board[r][c] = STORAGE;
}

void add_box_with_id(struct reader *reader, struct state *state, int id, int r,
                     int c) {
  reader->box_link[id] = id;
  reader->links[id][0] = id;
  reader->link_sizes[id] = 1;
  state->boxes[r][c] = id;
  struct pos p = {r, c};
  state->box_pos[id] = p;
}

void add_box(struct reader *reader, struct state *state, int r, int c) {
  if (!in_bounds(r, c)) {
    print_invalid_loc();
    return;
  }
  if (reader->board[r][c] == WALL)
    reader->board[r][c] = NONE;
  reader->n_boxes++;
  int b = reader->n_boxes;
  add_box_with_id(reader, state, b, r, c);
}

void link_boxes(struct reader *reader, struct state *state, int r1, int c1,
                int r2, int c2) {
  if (!in_bounds(r1, c1) || !in_bounds(r2, c2)) {
    printf("Invalid Location(s)\n");
    return;
  }
  int b1 = state->boxes[r1][c1];
  int b2 = state->boxes[r2][c2];
  if (b1 == 0 || b2 == 0)
    printf("Location not box(s)\n");
  int l1 = reader->box_link[b1];
  int l2 = reader->box_link[b2];
  if (l1 == l2)
    return;
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
  if (d == 'w')
    p.row--;
  else if (d == 'a')
    p.col--;
  else if (d == 's')
    p.row++;
  else if (d == 'd')
    p.col++;
  p.row = (p.row + ROWS) % ROWS;
  p.col = (p.col + COLS) % COLS;
  return p;
}

void move_box_rec(struct reader *reader, struct state *state, struct pos p,
                  char d, enum seen seen[ROWS][COLS], int links[MAX_BOXES],
                  int *links_len) {
  int r = p.row;
  int c = p.col;
  if (seen[r][c] == MARKED | seen[r][c] == STOPPED)
    return;
  seen[r][c] = MARKED;
  int b = state->boxes[r][c];
  if (b == 0) {
    if (reader->board[r][c] == WALL)
      seen[r][c] = STOPPED;
    return;
  }
  struct pos pp = pos_plus(p, d);
  int rr = pp.row;
  int cc = pp.col;
  state->boxes[r][c] = 0;
  move_box_rec(reader, state, pp, d, seen, links, links_len);
  if (seen[rr][cc] == MARKED) {
    state->boxes[rr][cc] = b;
    state->box_pos[b] = pp;
  } else {
    state->boxes[r][c] = b;
    seen[r][c] = STOPPED;
  }
  int l = reader->box_link[b];
  for (int i = 0; i < reader->link_sizes[l]; i++) {
    int b = reader->links[l][i];
    struct pos p = state->box_pos[b];
    int r = p.row;
    int c = p.col;
    if (seen[r][c] == FRESH) {
      links[(*links_len)++] = b;
      seen[r][c] = LINKED;
    }
  }
}

void move_player(struct reader *reader, struct state *state, char d) {
  enum seen seen[ROWS][COLS] = {};
  bool moved = false;
  int links[MAX_BOXES] = {};
  int links_len = 0;
  struct pos p = state->box_pos[1];
  move_box_rec(reader, state, p, d, seen, links, &links_len);
  if (seen[p.row][p.col] == MARKED)
    moved = true;
  while (links_len > 0) {
    int b = links[--links_len];
    struct pos p = state->box_pos[b];
    move_box_rec(reader, state, p, d, seen, links, &links_len);
    if (seen[p.row][p.col] == MARKED)
      moved = true;
  }
  if (moved) {
    reader->move_counter++;
    reader->history[reader->move_counter % MAX_HISTORY] = *state;
  }
}

void print_counter(struct reader *reader, struct state *state) {
  printf("Number of moves so far: %d\n", reader->move_counter);
}

int is_won(struct reader *reader, struct state *state) {
  for (int r = 0; r < ROWS; r++)
    for (int c = 0; c < COLS; c++) {
      enum base base = reader->board[r][c];
      int box = state->boxes[r][c];
      if (box != 0 && base != STORAGE)
        return false;
    }
  return true;
}

void undo(struct reader *reader, struct state *state) {
  if (reader->move_counter > 0) {
    reader->move_counter--;
    *state = reader->history[reader->move_counter % MAX_HISTORY];
  }
}

void reset_game(struct reader *reader, struct state *state) {
  printf("=== Resetting Game ===\n");
  reader->move_counter = 0;
  *state = reader->history[0];
}

void print_line() {
  for (int i = 0; i < COLS * 4 + 1; i++)
    printf("-");
  printf("\n");
}

void print_title() {
  print_line();
  char *title = "S O K O B A N";
  int len = COLS * 4 + 1;
  int n_white = len - strlen(title) - 2;
  printf("|");
  for (int i = 0; i < n_white / 2; i++)
    printf(" ");
  printf("%s", title);
  for (int i = 0; i < (n_white + 1) / 2; i++)
    printf(" ");
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
      if (box == 1)
        printf("^_^");
      else if (base == WALL)
        printf("===");
      else if (box == 0 && base == STORAGE)
        printf(" o ");
      else if (base == STORAGE)
        printf("[o]");
      else if (box == 0)
        printf("   ");
      else
        printf("[ ]");
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
      if (reader->move_counter == 1)
        printf("=== Level Solved in 1 Move! ===\n");
      else
        printf("=== Level Solved in %d Moves! ===\n", reader->move_counter);
      return;
    }
  }
}

int main(void) {
  struct reader reader = {{}, 1, {}, {}, {}, 0, {}};
  struct state state = {{}, {}};

  printf("=== Level Setup ===\n");
  char c;
  while (true) {
    if (scanf(" %c", &c) != 1)
      return 0;
    if (c == 'q')
      break;
    else if (c == 'w') {
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
  int player_row;
  int player_col;
  while (true) {
    printf("Enter player starting position: ");
    if (scanf("%d %d", &player_row, &player_col) != 2)
      return 0;
    if (in_bounds(player_row, player_col) &&
        reader.board[player_row][player_col] != WALL &&
        state.boxes[player_row][player_col] == 0)
      break;
    printf("Position [%d][%d] is invalid\n", player_row, player_col);
  }
  add_box_with_id(&reader, &state, 1, player_row, player_col);

  printf("\n=== Starting Sokoban! ===\n");
  reader.history[0] = state;
  print_board(&reader, &state);

  gameplay(&reader, &state);
}
