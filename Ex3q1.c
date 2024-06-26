#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include "math.h"
#define MAX_SIZE 128

int matrixType(complex double **matrix,const int *rows, const int *cols);
complex double **inputMatrix(char input[MAX_SIZE], int *rows, int *cols);
complex double **createMatrix(int rows, int cols);
void freeMatrix(complex double **matrix, int rows);
complex double **ADDMatrices(complex double** firMatrix,complex double** secMatrix, int rows, int cols);
complex double **SUBMatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols);
complex double **MULMatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols);
complex double **TRANSPOSEMatrices(complex double **firMatrix, int rows, int cols);
complex double **logANDmatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols);
complex double **logORmatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols);
complex double **logNOTmatrices(complex double **matrix, int rows, int cols);
void printMatrix(complex double **matrix, int rows, int cols);


int main() {
    double complex **firMatrix, **secMatrix, **resMatrix;
    char op[10], input[MAX_SIZE];
    int rows1, cols1, rows2, cols2;
    while (1) {
        printf("Enter the first matrix in format (rows,columns:val1,val2,...,valN): ");
        scanf(" %[^\n]", input);
        firMatrix = inputMatrix(input, &rows1, &cols1);

        printf("Enter the second matrix in format (rows,columns:val1,val2,...,valN): ");
        scanf(" %[^\n]", input);
        secMatrix = inputMatrix(input, &rows2, &cols2);

        printf("Enter the operation (ADD,SUB,MUL,AND,OR,NOT,TRANSPOSE): ");
        scanf("%s", op);


        // Perform the operation
        if (strcmp(op, "ADD") == 0) {
            resMatrix = ADDMatrices(firMatrix, secMatrix, rows1, cols1);
        } else if (strcmp(op, "SUB") == 0) {
            resMatrix = SUBMatrices(firMatrix, secMatrix, rows1, cols1);
        } else if (strcmp(op, "MUL") == 0) {
            resMatrix = MULMatrices(firMatrix, secMatrix, rows1, cols1);
        } else if (strcmp(op, "TRANSPOSE") == 0) {
            resMatrix = TRANSPOSEMatrices(firMatrix, rows1, cols1);
        } else if (strcmp(op, "AND") == 0) {
            resMatrix = logANDmatrices(firMatrix, secMatrix, rows1, cols1);
        } else if (strcmp(op, "OR") == 0) {
            resMatrix = logORmatrices(firMatrix, secMatrix, rows1, cols1);
        } else if (strcmp(op, "NOT") == 0) {
            resMatrix = logNOTmatrices(firMatrix, rows1, cols1);
        } else {
            printf("NOT VALID OP\n");
        }

        printf("Output: ");
        printMatrix(resMatrix, rows1, cols1);

        // Free memory
        freeMatrix(firMatrix, rows1);
        freeMatrix(secMatrix, rows2);
        if (strcmp(op, "TRANSPOSE") == 0) {
            freeMatrix(resMatrix, cols1);
        } else {
            freeMatrix(resMatrix, rows1);
        }
        if(strcmp(op,"Exit")==0)break;
    }

    return 0;
}
int matrixType(complex double **matrix,const int *rows, const int *cols){
    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *cols; j++) {
            double real = creal(matrix[i][j]);
            double imag = cimag(matrix[i][j]);

            if ((floor(real) != real) || imag != 0.0) {
                return 0; // Not an integer matrix
            }
        }
    }
    return 1; // Integer matrix
}
complex double **logNOTmatrices(complex double **matrix, int rows, int cols) {
    complex double **result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = (cabs(matrix[i][j]) == 0) ? 1.0 : 0.0;
        }
    }
    return result;
}

complex double **logORmatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols) {
    complex double **result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = (cabs(firMatrix[i][j]) != 0 || cabs(secMatrix[i][j]) != 0) ? 1.0 : 0.0;
        }
    }
    return result;
}

complex double **logANDmatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols) {
    complex double ** result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = (cabs(firMatrix[i][j]) != 0 && cabs(secMatrix[i][j]) != 0) ? 1.0 : 0.0;
        }
    }
    return result;
}

complex double **TRANSPOSEMatrices(complex double **firMatrix, int rows, int cols) {
    complex double** result = createMatrix(cols, rows);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[j][i] = firMatrix[i][j];
        }
    }
    return result;
}

complex double **MULMatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols) {
    complex double** result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = 0+0*I;
            for (int k = 0; k < cols; k++) {
                result[i][j] += firMatrix[i][k] * secMatrix[k][j];
            }
        }
    }
    return result;
}

complex double **SUBMatrices(double complex** firMatrix, double complex** secMatrix, int rows, int cols) {
    complex double** result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = firMatrix[i][j] - secMatrix[i][j];
        }
    }
    return result;}

complex double **ADDMatrices(double complex** firMatrix, double complex** secMatrix, int rows, int cols) {
    complex double **result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = firMatrix[i][j] + secMatrix[i][j];
        }
    }
    return result;
}

complex double ** createMatrix(int rows, int cols) {
    complex double **matrix = (double complex**)malloc(rows * sizeof(double complex*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (double complex*)malloc(cols * sizeof(double complex));
    }
    return matrix;
}
void freeMatrix(complex double **matrix, int rows){
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

complex double **inputMatrix(char input[MAX_SIZE], int *rows, int *cols){
    sscanf(input, "(%d,%d:", rows, cols);
    complex double** matrix = createMatrix(*rows, *cols);
    char* token = strtok(input, ":");
    token = strtok(NULL, ",");
    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *cols; j++) {
            double real = 0, imag = 0;
            char* plus = strchr(token, '+');
            char* i_char = strchr(token, 'i');
            if (plus && i_char) {
                // Complex number
                real = strtod(token, &token);
                imag = strtod(token + 1, &token); // Skip '+'
            } else if (i_char) {
                //imaginary number
                imag = strtod(token, &token); // Skip 'i'
            } else {
                // Real number (integer or double)
                real = strtod(token, &token);
            }
            matrix[i][j] = real + imag * I;
            token = strtok(NULL, ",)");
        }
    }
    return matrix;
}

void printMatrix(complex double **matrix, int rows, int cols) {
    printf("(%d,%d:", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double real = creal(matrix[i][j]);
            double imag = cimag(matrix[i][j]);

            if (matrixType(matrix, &rows,&cols) && imag == 0.0) {
                // Print as integer
                printf("%d", (int) real);
            } else {
                // Print as double or complex
                if (imag == 0.0) {
                    printf("%lf", real);
                } else if (real == 0.0) {
                    printf("%lfi", imag);
                } else {
                    printf("%lf+%lfi", real, imag);
                }
            }

            if (i != rows - 1 || j != cols - 1) {
                printf(",");
            }
        }
    }
    printf(")\n");
}