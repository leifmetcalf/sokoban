#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define ROWS 10
#define COLS 10
#define MAX_HIS 1000
#define MAX_LINKS 100

// Every tile on the map has to be one of the following values.
enum base {NONE, WALL, STORAGE};

// A single tile of our board.
// box should only contain the value:
// - true (1): there exists a box here
// - false (0): there doesn't exist a box here
struct tile {
  enum base base;
  int box;
  int link;
};

struct pos {
  int row;
  int col;
};

struct player_state {
  struct pos location;
  int counter;
};

struct link_info {
  int link;
  bool done;
};

void init_board(struct tile board[ROWS][COLS]);
void print_board(struct tile board[ROWS][COLS], struct pos player);
void level_setup(struct tile board[ROWS][COLS]);
struct pos set_player_location(struct tile board[ROWS][COLS]);
void gameplay(struct tile board[ROWS][COLS], struct player_state player);

int in_bounds(int row, int col);
int valid_player_pos(struct tile board[ROWS][COLS], struct pos player);
int valid_box_pos(struct tile board[ROWS][COLS], struct pos curr);
struct pos adjacent_pos(struct pos curr, char direction);
void copy_board(struct tile dest[ROWS][COLS], struct tile source[ROWS][COLS]);
int is_box(struct tile board[ROWS][COLS], struct pos c);
int is_wall(struct tile board[ROWS][COLS], struct pos c);
int eq_pos(struct pos a, struct pos b);
void init_links(struct link_info links[MAX_LINKS]);
int get_link(struct link_info links[MAX_LINKS]);
void add_link(struct link_info links[MAX_LINKS], int link);
void link_done(struct link_info links[MAX_LINKS], int link);

void test(int i) {

  printf("TEST %d\n", i);
  fflush(stdout);
}

void print_links(struct link_info links[MAX_LINKS]) {
  for (int i = 0; i < 10; i++) {

    printf("[%d] [%d]\n", links[i].link, links[i].done);
  }
}

int main(void) {

  struct tile board[ROWS][COLS];
  init_board(board);

  printf("=== Level Setup ===\n");
  level_setup(board);

  struct player_state player;
  player.location = set_player_location(board);
  player.counter = 0;

  printf("\n=== Starting Sokoban! ===\n");
  print_board(board, player.location);

  gameplay(board, player);
}

void print_invalid_loc(void) {
  printf("location out of bounds\n");
}

int add_single_wall(struct tile board[ROWS][COLS], int row, int col) {
  if (in_bounds(row, col)) {
    board[row][col].base = WALL;
    board[row][col].box = false;
    return true;
  }
  return false;
}

void add_wall(struct tile board[ROWS][COLS]) {
  int row, col;
  scanf("%d %d", &row, &col);

  if (!add_single_wall(board, row, col)) {
    print_invalid_loc();
  }
}

void add_walls(struct tile board[ROWS][COLS]) {
  int row_s, col_s, row_e, col_e;
  scanf("%d %d %d %d", &row_s, &col_s, &row_e, &col_e);

  if (!(in_bounds(row_s, col_s) || in_bounds(row_e, col_e))) {
    print_invalid_loc();
    return;
  }

  for (int i = row_s; i <= row_e; i++) {
    for (int j = col_s; j <= col_e; j++) {
      add_single_wall(board, i, j);
    }
  }
}

void add_storage(struct tile board[ROWS][COLS]) {
  int row, col;
  scanf("%d %d", &row, &col);

  if (!in_bounds(row, col)) {
    print_invalid_loc();
    return;
  }

  board[row][col].base = STORAGE;
}

void add_block(struct tile board[ROWS][COLS]) {
  int row, col;
  scanf("%d %d", &row, &col);

  if (!in_bounds(row, col)) {
    print_invalid_loc();
    return;
  }

  if (board[row][col].base == WALL) {
    board[row][col].base = NONE;
  }

  board[row][col].box = true;
  board[row][col].link = 0;
}

void relink(struct tile board[ROWS][COLS], int old, int new) {
  printf("test1");
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      printf("test1.5");
      if (board[i][j].link == old) {
        printf("Test2");
        board[i][j].link = new;
      }
    }
  }
}

int link_boxes(struct tile board[ROWS][COLS], int curr_link) {
  int r1, r2, c1, c2;
  scanf("%d %d %d %d", &r1, &c1, &r2, &c2);

  if (!in_bounds(r1, c1) || !in_bounds(r2, c2)) {
    printf("Invalid Location(s)\n");
    return curr_link;
  }
  
  struct pos b1 = {r1, c1};
  struct pos b2 = {r2, c2};
  if (!is_box(board, b1) || !is_box(board, b2)) {
    printf("Location not box(s)\n");
  }

  if (board[r1][c1].link != 0) {
    printf("test: %d\n", board[r1][c1].link);
    int old_link = board[r1][c1].link;
    relink(board, old_link, curr_link);
  }
  if (board[r2][c2].link != 0) {
    relink(board, board[r2][c2].link, curr_link);
  }

  board[r1][c1].link = curr_link;
  board[r2][c2].link = curr_link;
  return curr_link++;
}

void level_setup(struct tile board[ROWS][COLS]) {
  struct pos no_player = {-1, -1};
  int curr_link = 1;
  char c;
  while (scanf(" %c", &c) == 1 && c != 'q') {
    if (c == 'w') {
      add_wall(board);
    } else if (c == 'b') {
      add_block(board);
    } else if (c == 's') {
      add_storage(board);
    } else if (c == 'W') {
      add_walls(board);
    } else if (c == 'l') {
      curr_link = link_boxes(board, curr_link);
    }
    print_board(board, no_player);
  }
}

struct pos set_player_location(struct tile board[ROWS][COLS]) {
  printf("Enter player starting position: ");
  struct pos p;
  scanf("%d %d", &p.row, &p.col);
  while (!(valid_player_pos(board, p))) {
    printf("Position [%d][%d] is invalid\n", p.row, p.col);
    printf("Enter player starting position: ");
    scanf("%d %d", &p.row, &p.col);
  }
  return p;
}

int move_boxes(struct tile board[ROWS][COLS], struct tile new[ROWS][COLS], struct pos first, char direction, struct link_info links[MAX_LINKS]) {

  // look for the next free spot
  add_link(links, board[first.row][first.col].link);
  struct pos next = adjacent_pos(first, direction);
  while (is_box(board, next) && !eq_pos(next, first)) {
    add_link(links, board[next.row][next.col].link);
    next = adjacent_pos(next, direction);
  }
    
  // if we found a wall, do nothing
  if (is_wall(board, next)) {
    // fill in new with the same values
    return false;
  }

  // if there was no wall, we can move things along!
  struct pos curr = first;
  next = adjacent_pos(curr, direction);
  new[curr.row][curr.col].box = false;
  board[curr.row][curr.col].box = false;
  new[next.row][next.col].box = true;
  new[next.row][next.col].link = board[curr.row][curr.col].link;
  curr = next;
  next = adjacent_pos(curr, direction);

  while (!eq_pos(first, curr) && is_box(board, curr)) {
    new[next.row][next.col].box = true;
    new[next.row][next.col].link = board[curr.row][curr.col].link;
    board[curr.row][curr.col].box = false;

    curr = next;
    next = adjacent_pos(curr, direction);
  }
  return true;
}



struct player_state move_player(struct tile board[ROWS][COLS], struct player_state orig, char direction) {

  struct pos new = adjacent_pos(orig.location, direction);

  // if there is nothing in that position
  if (valid_player_pos(board, new)) {
    orig.counter++;
    orig.location = new;
    return orig;
  }

  // not a box, so we can't go there.
  if (!board[new.row][new.col].box) {
    return orig;
  }


  // MOVING BOXES

  struct pos first_box = new;
  struct tile new_board[ROWS][COLS];
  copy_board(new_board, board);
  int anything_moved = false;

  // array of links that we need to push
  struct link_info links[MAX_LINKS];
  init_links(links);
  if (move_boxes(board, new_board, first_box, direction, links)) {
    anything_moved = true;
  }
  int curr_link = get_link(links);
  while (curr_link != 0) {
    link_done(links, curr_link); 
    for (int i = 0; i < ROWS; i++) {
      for (int j = 0; j < COLS; j++) {
        if (board[i][j].box && board[i][j].link == curr_link) {
          struct pos c = {i, j};
          if (move_boxes(board, new_board, c, direction, links)) {
            anything_moved = true;
          }
        }
      }
    }
    curr_link = get_link(links);
  }
  copy_board(board, new_board);

  if (!anything_moved) {
    return orig;
  }
  int player_moved = !is_box(board, new) && !is_wall(board, new);

  orig.counter++;
  if (player_moved) {
    orig.location = new;
  }
  return orig;
}

void print_counter(int counter) {
  printf("Number of moves so far: %d\n", counter);
}

int is_won(struct tile board[ROWS][COLS]) {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      struct tile curr = board[i][j];
      if (curr.box && curr.base == NONE) {
        return false;
      }
    }
  }
  return true;
}

struct player_state undo(struct tile board[ROWS][COLS], struct tile history[MAX_HIS][ROWS][COLS], struct pos p_history[MAX_HIS], struct player_state p) {
  
  if (p.counter == 0) {
    return p;
  }

  p.counter--;
  copy_board(board, history[p.counter]);
  p.location = p_history[p.counter];
  return p;
}

void update_history(struct tile history[MAX_HIS][ROWS][COLS], struct pos p_history[MAX_HIS], struct tile board[ROWS][COLS], struct player_state p) {
  
  copy_board(history[p.counter], board);
  p_history[p.counter] = p.location;
}

void gameplay(struct tile board[ROWS][COLS], struct player_state p) {
  
  p.counter = 0;

  struct tile history[MAX_HIS][ROWS][COLS];
  struct pos p_history[MAX_HIS];
  update_history(history, p_history, board, p); 

  char c;
  while (scanf(" %c", &c) == 1) {
    switch (c) {
    case 'w':
    case 'a':
    case 's':
    case 'd':;
      //printf("test");
      int old_counter = p.counter;
      p = move_player(board, p, c);
      if (p.counter != old_counter) {
        update_history(history, p_history, board, p);
      }
      break;
    case 'c':
      print_counter(p.counter);
      break;
    case 'r':
      printf("=== Resetting Game ===\n");
      copy_board(board, history[0]);
      p.location = p_history[0];
      p.counter = 0;
      break;
    case 'u':
      p = undo(board, history, p_history, p);
    }
    print_board(board, p.location);
    if (is_won(board)) {
      if (p.counter == 1) {
        printf("=== Level Solved in 1 Move! ===\n");
      } else {
        printf("=== Level Solved in %d Moves! ===\n", p.counter);
      }
      return;
    }
  }
  return;
}

//// UTILITIES ////

struct pos adjacent_pos(struct pos curr, char direction) {
  struct pos new = curr;
  if (direction == 'w') {
    new.row--;
  } else if (direction == 'a') {
    new.col--;
  } else if (direction == 's') {
    new.row++;
  } else if (direction == 'd') {
    new.col++;
  }

  // board wrapping
  new.row = (new.row + ROWS) % ROWS;
  new.col = (new.col + COLS) % COLS;

  return new;
}

int in_bounds(int row, int col) {
  return (0 <= row && row < ROWS && 0 <= col && col < COLS);
}

int valid_player_pos(struct tile board[ROWS][COLS], struct pos player) {
  if (!in_bounds(player.row, player.col)) {
    return false;
  }
  struct tile curr = board[player.row][player.col];
  if (curr.base == WALL || curr.box) {
    return false;
  }
  return true;
}

int valid_box_pos(struct tile board[ROWS][COLS], struct pos c) {
  if (!in_bounds(c.row, c.col)) {
    return false;
  }
  struct tile ct = board[c.row][c.col];
  if (ct.base == WALL) {
    return false;
  }
  return true;
}

void copy_board(struct tile dest[ROWS][COLS], struct tile source[ROWS][COLS]) {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      dest[i][j] = source[i][j];
    }
  }
}

int is_box(struct tile board[ROWS][COLS], struct pos c) {
  return board[c.row][c.col].box;
}

int is_wall(struct tile board[ROWS][COLS], struct pos c) {
  return board[c.row][c.col].base == WALL;
}

int eq_pos(struct pos a, struct pos b) {
  if (a.row == b.row && a.col == b.col) {
    return true;
  }
  return false;
}

void init_links(struct link_info links[MAX_LINKS]) {
  for (int i = 0; i != MAX_LINKS; i++) {
    links[i].link = 0;
    links[i].done = false;
  }
}

int get_link(struct link_info links[MAX_LINKS]) {
  int i = 0;
  while (i != MAX_LINKS && links[i].done) {
    i++;
  }
  return links[i].link;
}

void add_link(struct link_info links[MAX_LINKS], int link) {
  int i = 0;
  while (i != MAX_LINKS && !(links[i].link == 0 || links[i].link == link)) {
    i++;
  }
  if (links[i].link == 0) {
    links[i].link = link;
  }
}

void link_done(struct link_info links[MAX_LINKS], int link) {
  int i = 0;
  while (i != MAX_LINKS && !(links[i].link == link || links[i].link == 0)) {
    i++;
  }
  if (links[i].link != 0) {
    links[i].done = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
////                           PROVIDED FUNCTIONS                           ////
////////////////////////////////////////////////////////////////////////////////

// NOTE: You can edit these if you wish.

// initialises the board to default values.
void init_board(struct tile board[ROWS][COLS]) {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      board[i][j].base = NONE;
      board[i][j].box = false;
      board[i][j].link = 0;
    }
  }
}

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
void print_board(struct tile board[ROWS][COLS], struct pos player) {
  print_title();
  for (int i  = 0; i < ROWS; i++) {
    print_line();
    for (int j = 0; j < COLS; j++) {
      printf("|");

      struct tile curr = board[i][j];
      if (i == player.row && j == player.col) {
        printf("^_^");
      } else if (curr.base == WALL) {
        printf("===");
      } else if (curr.box && curr.base == STORAGE) {
        printf("[.]");
      } else if (curr.box) {
        printf("[ ]");
      } else if (curr.base == STORAGE) {
        printf(" . ");
      } else {
        printf("   ");
      }

    }
    printf("|\n");
  }
  print_line();
  printf("\n");
}
