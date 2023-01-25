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
#include <stdlib.h>

typedef struct {
    int row;
    int col;
    int psize;
    int **grid;
    pthread_t *ids;
} params;

pthread_t *ids;


void *checkCol(void *args) {

}

void *checkRow(void *args) {

}

void *checkBox(void *args) {

}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  // YOUR CODE GOES HERE and in HELPER FUNCTIONS
  //threads for columns: n
  //thread for rows: n
  //threads for boxes: n 
  pthread_t ids[psize * 3];
  int currId = 0;
  int boxRow = 0;
  int boxCol = 1;

  for (int i = 1; i <= psize; i++) {
    //row
    //data setting
    printf("row[%d]...", i);
    fflush(stdout);
    params *data = (params *) malloc(sizeof(params));
    data->row = i;
    data->col = 1;
    data->psize = psize;
    data->grid = grid;
    //thread creation
    pthread_create(&ids[currId++], NULL, checkRow, (void *)data);

    //col
    //data setting
    printf("col[%d]...", i);
    fflush(stdout);
    data = (params *) malloc(sizeof(params));
    data->row = 1;
    data->col = i;
    data->psize = psize;
    data->grid = grid;
    //thread creation
    pthread_create(&ids[currId++], NULL, checkCol, (void *)data);

    //box
    //data setting
    printf("box[%d]...\n", i);
    fflush(stdout);
    data = (params *) malloc(sizeof(params));
    data->row = boxRow;
    data->col = boxCol;
    data->psize = psize;
    data->grid = grid;
    //thread creation
    pthread_create(&ids[currId++], NULL, checkCol, (void *)data);
    if (boxCol == (sqrt(psize) * 2) + 1) {
      boxCol = 0;
      boxRow + 3;
    } else {
      boxCol + 3;
    }
  }
  for (int i = 0; i < (psize * 3); i++) {
    printf("[%d]", ids[i]);
    pthread_join(ids[i], NULL);
  }
  printf("\n");
  *valid = true;
  *complete = true;
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
