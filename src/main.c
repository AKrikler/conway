#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define SLEEP_MS 50

#define INDEX(r, c, cols) ((r * cols) + c)

volatile sig_atomic_t running = 1;

void handle_signal(int sig) {
  (void)sig;
  running = 0;
}

void init_grid(int *grid, int rows, int cols) {
  srand(time(NULL));

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      grid[INDEX(r, c, cols)] = rand() % 2;
    }
  }
}

void render(const int *grid, int rows, int cols) {
  printf("\033[H");
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      putchar(grid[INDEX(r, c, cols)] ? '@' : ' ');
    }
    if (r < rows - 1)
      putchar('\n');
  }

  fflush(stdout);
}

void tick(int *grid, int rows, int cols) {
  int *next_grid = malloc(rows * cols * sizeof(int));
  if (!next_grid)
    return;

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      int neighbors = 0;

      for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
          if (dr == 0 && dc == 0)
            continue;

          int nr = r + dr;
          int nc = c + dc;

          if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
            neighbors += grid[INDEX(nr, nc, cols)];
          }
        }
      }

      int curr = grid[INDEX(r, c, cols)];
      if (curr && (neighbors < 2 || neighbors > 3)) {
        next_grid[INDEX(r, c, cols)] = 0;
      } else if (!curr && neighbors == 3) {
        next_grid[INDEX(r, c, cols)] = 1;
      } else {
        next_grid[INDEX(r, c, cols)] = curr;
      }
    }
  }

  memcpy(grid, next_grid, rows * cols * sizeof(int));
  free(next_grid);
}

int main(void) {
  signal(SIGINT, handle_signal);

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int rows = w.ws_row;
  int cols = w.ws_col;

  int *grid = malloc(rows * cols * sizeof(int));
  if (!grid)
    return 1;

  printf("\033[?25l\033[2J");

  init_grid(grid, rows, cols);

  while (running) {
    render(grid, rows, cols);
    tick(grid, rows, cols);
    usleep(SLEEP_MS * 1000);
  }

  printf("\033[?25h");
  free(grid);
  return 0;
}
