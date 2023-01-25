// Sudoku puzzle verifier and solver
// notes:
// - join: waits for pthread to finish
// - kill: kills (assuming means stops) a thread, must be done before join
// - 
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int row;
    int col;
    int psize;
    int **grid;
    pthread_t *ids;
} params;
bool glbValid = true;
bool glbComplete = true;
pthread_t *ids;


void *checkCol(void *args) {
  params* colArgs = (params *) args;
  bool seen[colArgs->psize + 1];
  memset(seen, 0, (colArgs->psize + 1) * sizeof(bool));
  for (int i = 1; i <= colArgs->psize; i++) {
    if (colArgs->grid[i][colArgs->col] == 0) {
      glbComplete = false;
      continue;
    }
    if (seen[colArgs->grid[i][colArgs->col]]) {
      glbValid = false;
      return;
    }
    seen[colArgs->grid[i][colArgs->col]] = true;
  }
}

void *checkRow(void *args) {
  params* rowArgs = (params *) args;
  bool seen[rowArgs->psize + 1];
  memset(seen, 0, (rowArgs->psize + 1) * sizeof(bool));
  for (int i = 1; i <= rowArgs->psize; i++) {
    if (rowArgs->grid[rowArgs->row][i] == 0) {
      glbComplete = false;
      continue;
    }
    if (seen[rowArgs->grid[rowArgs->row][i]]) {
      glbValid = false;
      return;
    }
    seen[rowArgs->grid[rowArgs->row][i]] = true;
  }
}

void *checkBox(void *args) {
  params* boxArgs = (params *) args;
  bool seen[boxArgs->psize + 1];
  memset(seen, 0, (boxArgs->psize + 1) * sizeof(bool));
  int boxRow = boxArgs->row;
  int boxCol = boxArgs->col;
  for (int i = 1; i <= boxArgs->psize; i++) {
    if (boxArgs->grid[boxRow][boxCol] == 0) {
      glbComplete = false;
      continue;
    }
    if (seen[boxArgs->grid[boxRow][boxCol]]) {
      glbValid = false;
      return;
    }
    seen[boxArgs->grid[boxRow][boxCol]] = true;

    if (boxCol == boxArgs->col + (sqrt(boxArgs->psize) - 1)) {
      boxCol = boxArgs->col;
      boxRow++;
    } else {
      boxCol++;
    }
  }
}

void *finishPuzzle() {

}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  pthread_t ids[psize * 3];
  int currId = 0;
  int boxRow = 1;
  int boxCol = 1;

  for (int i = 1; i <= psize; i++) {
    //row
    //data setting
    params *data = (params *) malloc(sizeof(params));
    data->row = i;
    data->col = 1;
    data->psize = psize;
    data->grid = grid;
    //thread creation
    pthread_create(&ids[currId++], NULL, checkRow, (void *)data);

    //col
    //data setting
    data = (params *) malloc(sizeof(params));
    data->row = 1;
    data->col = i;
    data->psize = psize;
    data->grid = grid;
    //thread creation
    pthread_create(&ids[currId++], NULL, checkCol, (void *)data);

    //box
    //data setting
    data = (params *) malloc(sizeof(params));
    data->row = boxRow;
    data->col = boxCol;
    data->psize = psize;
    data->grid = grid;
    //thread creation
    pthread_create(&ids[currId++], NULL, checkBox, (void *)data);
    if (boxCol == psize - (sqrt(psize) - 1)) {
      boxCol = 1;
      boxRow += sqrt(psize);
    } else {
      boxCol += sqrt(psize);
    }
  }
  // for (int i = 0; i < (psize * 3); i++) {
  //   printf("[%ld]", ids[i]);
  //   pthread_join(ids[i], NULL);
  // }
  // printf("\n");
  *valid = glbValid;
  *complete = glbComplete;
  if (!complete) finishPuzzle();

  //if not complete, run backtracing complete method
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
