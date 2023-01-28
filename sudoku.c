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
    int boxRow;
    int boxCol;
    int psize;
    int **grid;
    bool *seen;
} params;

bool glbValid = true;
bool glbComplete = true;
pthread_t *ids;


void *checkCol(void *args) {
  params* colArgs = (params *) args;
  bool seen[colArgs->psize + 1];
  for (int i = 1; i <= colArgs->psize; i++) {
    memset(seen, 0, (colArgs->psize + 1) * sizeof(bool));
    for (int j = 1; j <= colArgs->psize; j++) {
      if (!glbComplete || !glbValid ) return 0;
      if (colArgs->grid[j][i] == 0) {
        glbComplete = false;
        continue;
      }
      if (seen[colArgs->grid[j][i]]) {
        glbValid = false;
        return 0;
      }
      seen[colArgs->grid[j][i]] = true;
    }
  }
  return 0;
}

void *checkRow(void *args) {
  params* rowArgs = (params *) args;
  bool seen[rowArgs->psize + 1];
  for (int i = 1; i <= rowArgs->psize; i++) {
    memset(seen, 0, (rowArgs->psize + 1) * sizeof(bool));
    for (int j = 1; j <= rowArgs->psize; j++) {
      if (!glbComplete || !glbValid ) return 0;
      if (rowArgs->grid[i][j] == 0) {
        glbComplete = false;
        continue;
      }
      if (seen[rowArgs->grid[i][j]]) {
        glbValid = false;
        return 0;
      }
      seen[rowArgs->grid[i][j]] = true;
    }
  }
  return 0;
}

void *checkBox(void *args) {
  params* boxArgs = (params *) args;
  bool seen[boxArgs->psize + 1];
  memset(seen, 0, (boxArgs->psize + 1) * sizeof(bool));
  int boxRow = boxArgs->row;
  int boxCol = boxArgs->col;
  for (int i = 1; i <= boxArgs->psize; i++) {
    if (!glbComplete || !glbValid ) return 0;
    if (boxArgs->grid[boxRow][boxCol] == 0) {
      glbComplete = false;
      continue;
    }
    if (seen[boxArgs->grid[boxRow][boxCol]]) {
      glbValid = false;
      return 0;
    }
    seen[boxArgs->grid[boxRow][boxCol]] = true;

    if (boxCol == boxArgs->col + (sqrt(boxArgs->psize) - 1)) {
      boxCol = boxArgs->col;
      boxRow++;
    } else {
      boxCol++;
    }
  }
  free(args);
  return 0;
}

void *slvChkRow(void *args) {
  params* chkRow = (params *) args;
  for (int i = 1; i <= chkRow->psize; i++) {
    if (chkRow->grid[chkRow->row][i] != 0) {
      chkRow->seen[chkRow->grid[chkRow->row][i]] = true;
    }
  }
  return 0;
}

void *slvChkCol(void *args) {
  params* chkCol = (params *) args;
  for (int i = 1; i <= chkCol->psize; i++) {
    if (chkCol->grid[i][chkCol->col] != 0) {
      chkCol->seen[chkCol->grid[i][chkCol->col]] = true;
    }
  }
  return 0;
}

void *slvChkBox(void *args) {
  params* chkBox = (params *) args;
  int boxRow = chkBox->boxRow;
  int boxCol = chkBox->boxCol;
  for (int i = 1; i <= chkBox->psize; i++) {
    if (chkBox->grid[boxRow][boxCol] != 0) {
      chkBox->seen[chkBox->grid[boxRow][boxCol]] = true;
    }

    if (boxCol == chkBox->boxCol + (sqrt(chkBox->psize) - 1)) {
      boxCol = chkBox->boxCol;
      boxRow++;
    } else {
      boxCol++;
    }
  }
  return 0;
}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  pthread_t ids[psize + 2];
  int currId = 0;
  int boxRow = 1;
  int boxCol = 1;

  params *data = (params *) malloc(sizeof(params));
  data->psize = psize;
  data->grid = grid;
  pthread_create(&ids[currId++], NULL, checkRow, (void *)data);
  pthread_create(&ids[currId++], NULL, checkCol, (void *)data);
  free(data);
  for (int i = 1; i <= psize; i++) {
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
  for (int i = 0; i < (psize + 2); i++) {
    pthread_join(ids[i], NULL);
  }
  *valid = glbValid;
  *complete = glbComplete;
  if (glbComplete) return;
  bool *seen;
  bool seenArr[psize + 1];
  seen = seenArr;
  bool unSolved = true;
  while (unSolved) {
    fflush(stdout);
    int boxRow = 1;
    int boxCol = 1;
    unSolved = false;
    for (int i = 1; i <= psize; i++) {
      for (int j = 1; j <= psize; j++) {
        if (j != 1 && (j - 1)  % (int)sqrt(psize) == 0) {
          boxCol += sqrt(psize);  
        }
        if (grid[i][j] == 0) {
          memset(seen, 0, (psize + 1) * sizeof(bool));
          pthread_t rowId, colId, boxId;
          data = (params *) malloc(sizeof(params));
          data->row = i;
          data->col = j;
          data->psize = psize;
          data->grid = grid;
          data->seen = seen;
          data->boxRow = boxRow;
          data->boxCol = boxCol;
          pthread_create(&rowId, NULL, slvChkRow, (void *)data);
          pthread_create(&colId, NULL, slvChkCol, (void *)data);
          pthread_create(&boxId, NULL, slvChkBox, (void *)data);
          pthread_join(rowId, NULL);
          pthread_join(colId, NULL);
          pthread_join(boxId, NULL);
          int num = -1;
          for (int k = 1; k <= psize; k++) {
            if (!seen[k]) {
              if (num == -1) {
                num = k;
              } else {
                unSolved = true;
                continue;
              }
            }
          }
          grid[i][j] = num;
          free(data);
        }
      }
      if (i != 1 && (i - 1)  % (int)sqrt(psize) == 0) {
          boxRow += sqrt(psize);
          boxCol = 1;
      }
      
    }
  }


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
