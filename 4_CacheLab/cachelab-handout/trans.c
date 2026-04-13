/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_32(int M, int N, int A[N][M], int B[M][N]);
void transpose_64(int M, int N, int A[N][M], int B[M][N]);
void transpose_other(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch(N) {
        case 32:    //32x32 matrix
            transpose_32(M, N, A, B);
            break;
        case 64:    //64x64 matrix
            transpose_64(M, N, A, B);
            break;
        default:    // all matrices not 32x32 or 64x64
            transpose_other(M, N, A, B);
            break;

    }
}

char transpose_32_desc[] = "Transpose a 32x32 matrix";
void transpose_32(int M, int N, int A[N][M], int B[M][N]) {
    int i;
    int col_block, row_block;
    int t0;
    int t1;
    int t2;
    int t3;
    int t4;
    int t5;
    int t6;
    int t7;
    // sub-blocking into 8x8
    // cache holds 8 rows of a 32x32 matrix 
    // sub-block row in 8s
    for (row_block = 0 ; row_block < N; row_block +=8){
        // sub-block column in 8s
        for (col_block = 0; col_block < M; col_block +=8){
            // Iterate over row in 8s
            for (i = row_block; i < row_block + 8; i++) {
                // save Matrix A reads 0-7 in variables
                t0 = A[i][col_block];
                t1 = A[i][col_block +1];
                t2 = A[i][col_block +2];
                t3 = A[i][col_block +3];
                t4 = A[i][col_block +4];
                t5 = A[i][col_block +5];
                t6 = A[i][col_block +6];
                t7 = A[i][col_block +7];
                // assign to Matrix B locations
                B[col_block][i] = t0;
                B[col_block +1][i] = t1;
                B[col_block +2][i] = t2;
                B[col_block +3][i] = t3;
                B[col_block +4][i] = t4;
                B[col_block +5][i] = t5; 
                B[col_block +6][i] = t6;
                B[col_block +7][i] = t7;
            }
        }
    }
}

char transpose_64_desc[] = "Transpose a 64x64 matrix";
void transpose_64(int M, int N, int A[N][M], int B[M][N]){
    int i;
    int col_block, row_block;
    int t0;
    int t1;
    int t2;
    int t3;
    int t4;
    int t5;
    int t6;
    int t7;
    // sub-blocking into 4x8
    // only 4 rows fit in cache at once for 64x64
    // rows into groups of 4
    for (row_block = 0 ; row_block < N; row_block +=4){
        // cols into groups of 8
        for (col_block = 0; col_block < M; col_block +=8){
            // iterate over row in 4s
            for (i = row_block; i < row_block + 4; i++) {
                // Save matrix A reads 0-3 in variables
                t0 = A[i][col_block];
                t1 = A[i][col_block +1];
                t2 = A[i][col_block +2];
                t3 = A[i][col_block +3];
                // assign vars to matrix B locations
                B[col_block][i] = t0;
                B[col_block +1][i] = t1;
                B[col_block +2][i] = t2;
                B[col_block +3][i] = t3;
            }
            // Save matrix A read 4-7 in variables
            for (i = row_block; i < row_block + 4; i++){
                t4 = A[i][col_block +4];
                t5 = A[i][col_block +5];
                t6 = A[i][col_block +6];
                t7 = A[i][col_block +7];
                // assign vars to matrix B locations
                B[col_block +4][i] = t4;
                B[col_block +5][i] = t5; 
                B[col_block +6][i] = t6;
                B[col_block +7][i] = t7;
            }
        }
    }
}

char transpose_other_desc[] = "Transpose any matrix that isn't 32x32 or 64x64";
void transpose_other(int M, int N, int A[N][M], int B[M][N]){
    int i, j;
    int col_block, row_block;
    // sub-blocking into 16x16
    // row = 16 height
    for (row_block = 0 ; row_block < N; row_block +=16){
        // col = 16 width
        for (col_block = 0; col_block < M; col_block +=16){
            // iterating over row ( i < N means don't go past the number of rows )
            for (i = row_block; i < row_block + 16 && i < N; i++) {
                // iterating down col (i < M means don't go past the number of cols)
                for (j = col_block; j < col_block + 16 && j < M; j++) {
                    B[j][i] = A[i][j];
                }
            }
        }
    } 
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

