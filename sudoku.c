/*
created by Ali Ibrahim

General notes and assumptions...
  - program validates whether a puzzle of N x N size is valid and/or complete

  - if incomplete, the program will attempt to complete given this tautology...
    - during the solving process, at least one empty spot of the puzzle
      can be filled by looking at that puzzle spot's column, row, and box.
    - Every complete iteration through each spot of the puzzle, will result
      in at least one spot of the puzzle being filled in.
    
  - a complete iteration resulting in no spots being filled in is a invalid
    and incomplete puzzle given the solving tautology.
  
To run the program...
  - compile: gcc -Wall -Wextra -pthread -lm -std=c99 sudoku.c -o sudoku
  - run (verify): 
    - ./sudoku puzzle2-invalid.txt
    - ./sudoku puzzle2-valid.txt
    - ./sudoku puzzle9-valid.txt
  - run (complete): 
    - ./sudoku puzzle2-fill-valid.txt
    - completing puzzles, tested with hard puzzle listed at the end of the assignment page
    - accounts for both completing puzzles and completing difficult puzzles extra credit
*/

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// struct defining the params for all methods used in threads
typedef struct {
    int row;
    int col;
    int boxRow;
    int boxCol;
    int psize;
    int **grid;
    bool *seen;
} params;

// bool var keeping track if puzzle is valid or not during thread runs
bool glbValid = true;
// bool var keeping track if puzzle is complete or not during thread runs
bool glbComplete = true;
pthread_t *ids;
 
// checks if the given column makes the puzzle invalid or incomplete
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

// checks if the given row makes the puzzle invalid or incomplete
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

// checks if the given box makes the puzzle invalid or incomplete
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

// finds and records all values found in the row of given puzzle spot
void *slvChkRow(void *args) {
  params* chkRow = (params *) args;
  for (int i = 1; i <= chkRow->psize; i++) {
    if (chkRow->grid[chkRow->row][i] != 0) {
      chkRow->seen[chkRow->grid[chkRow->row][i]] = true;
    }
  }
  return 0;
}

// finds and records all values found in the column of given puzzle spot
void *slvChkCol(void *args) {
  params* chkCol = (params *) args;
  for (int i = 1; i <= chkCol->psize; i++) {
    if (chkCol->grid[i][chkCol->col] != 0) {
      chkCol->seen[chkCol->grid[i][chkCol->col]] = true;
    }
  }
  return 0;
}

// finds and records all values found in the box of given puzzle spot
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
  // all thread ids stored to later join
  pthread_t ids[psize + 2];
  // current thread id index to use in thread creation
  int currId = 0;
  // the top left square of the current puzzle square
  int boxRow = 1;
  int boxCol = 1;

  // parameters to be sent to thread validation methods
  params *data = (params *) malloc(sizeof(params));
  data->psize = psize;
  data->grid = grid;
  // validating all rows and cols
  pthread_create(&ids[currId++], NULL, checkRow, (void *)data);
  pthread_create(&ids[currId++], NULL, checkCol, (void *)data);
  free(data);
  // iterating through each square pos in the puzzle
  for (int i = 1; i <= psize; i++) {
    // box
    // data setting
    data = (params *) malloc(sizeof(params));
    data->row = boxRow;
    data->col = boxCol;
    data->psize = psize;
    data->grid = grid;
    // thread creation
    pthread_create(&ids[currId++], NULL, checkBox, (void *)data);
    if (boxCol == psize - (sqrt(psize) - 1)) {
      boxCol = 1;
      boxRow += sqrt(psize);
    } else {
      boxCol += sqrt(psize);
    }
  }
  // joining all threads from validation
  for (int i = 0; i < (psize + 2); i++) {
    pthread_join(ids[i], NULL);
  }
  // setting the results of the validation
  *valid = glbValid;
  *complete = glbComplete;
  // if not complete, try to solve
  if (glbComplete) return;
  // if the puzzle can be solved (given the tautologies that must exist for every puzzle given to the program)
  bool canSolve;
  // array to store what values have been seen during solve checks
  bool *seen;
  bool seenArr[psize + 1];
  seen = seenArr;
  // if the puzzle hasn't been solved yet
  bool unSolved = true;
  while (unSolved) {
    // current puzzle square to be checked
    int boxRow = 1;
    int boxCol = 1;
    // assume puzzle is to be completed this iteration
    unSolved = false;
    // assume puzzle cannot be solved unless a puzzle value is set this iteration
    canSolve = false;
    // iterate through whole puzzle
    for (int i = 1; i <= psize; i++) {
      for (int j = 1; j <= psize; j++) {
        // adjust current square pos if needed
        if (j != 1 && (j - 1)  % (int)sqrt(psize) == 0) {
          boxCol += sqrt(psize);  
        }
        // if puzzle spot needs to be filled
        if (grid[i][j] == 0) {
          // clear seen array
          memset(seen, 0, (psize + 1) * sizeof(bool));
          // create threads to check for possible values to insert into puzzle spot
          // checking for possible values in the current row, col, and box of puzzle spot
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
          // searching through seen array for numbers that weren't found in same row, col, or box
          int num = -1;
          for (int k = 1; k <= psize; k++) {
            // havent seen the current number
            if (!seen[k]) {
              if (num == -1) {
                // haven't found a number to slot into puzzle spot
                // assumes this number is the one to use
                num = k;
              } else {
                // another possible number has been found
                unSolved = true;
                continue;
              }
            }
          }
          grid[i][j] = num;
          // because value was inserted into puzzle spot, assuming puzzle is solvable again
          canSolve = true;
          free(data);
        }
      }
      // adjusting box row value if needed
      if (i != 1 && (i - 1)  % (int)sqrt(psize) == 0) {
          boxRow += sqrt(psize);
          boxCol = 1;
      }
    }
    if (!canSolve && unSolved) {
      // no value was inserted this iteration, but the puzzle isn't solved yet
      // meaning the puzzle doesn't follow given tautologies
      // not solvable given current conditions, so valid and complete are false
      *valid = false;
      *complete = false;
      return;
    }
  }
  // if program reaches this point, puzzle has been solved and is valid
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
