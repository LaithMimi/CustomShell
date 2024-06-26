#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#define MAX_SIZE 128

complex double **inputMatrix(char input[MAX_SIZE], int *rows, int *cols);
double complex** createMatrix(int rows, int cols);
void freeMatrix(double complex** matrix, int rows);

double complex** ADDMatrices(double complex** firMatrix, double complex** secMatrix, int rows, int cols);
complex double **SUBMatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols);

complex double **MULMatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols);

int main() {
   double complex **firMatrix,**secMatrix,**resMatrix;
   char *op, input[MAX_SIZE];
   int rows,cols;

   //the %[^\n] conversion specification,
   // which matches a string of all characters not equal to the new line character ('\n')
   // and stores it (plus a terminating '\0' character) in str.
   scanf("%[^\n]",input);
   firMatrix = inputMatrix(input,&rows,&cols);

    if (!strcmp(op, "NOT") && !strcmp(op,"TRANSPOSE")) {
        scanf("%[^\n]", input);
        secMatrix = inputMatrix(input, &rows, &cols);
    }

    if (strcmp(op, "ADD") == 0) {
        resMatrix=ADDMatrices(firMatrix,secMatrix,rows,cols);
    }
    else if (strcmp(op, "SUB") == 0) {
        resMatrix=SUBMatrices(firMatrix,secMatrix,rows,cols);

    }
    else if (strcmp(op, "MUL") == 0) {
        resMatrix=MULMatrices(firMatrix,secMatrix,rows,cols);
    }
    else if (strcmp(op, "TRANSPOSE") == 0) {
        resMatrix=TRANSPOSEMatrices(firMatrix,rows,cols);
    }
    else if (strcmp(op, "AND") == 0) {
        resMatrix=logANDmatrices(firMatrix,secMatrix,rows,cols);
    }
    else if (strcmp(op, "OR") == 0) {
        resMatrix=logORmatrices(firMatrix,secMatrix,rows,cols);
    }
    else if (strcmp(op, "NOT") == 0) {
        resMatrix=logNOTmatrices(firMatrix,rows,cols);
    }
    else {
        printf("Invalid operation\n");
        return 1;
    }

    freeMatrix(firMatrix, rows);
    if (!strcmp(op, "NOT") && !strcmp(op, "TRANSPOSE")) {
        freeMatrix(secMatrix, rows);
    }
    if (strcmp(op, "TRANSPOSE")==0) {
        freeMatrix(resMatrix, cols);
    } else if (strcmp(op, "MUL")==0) {
        freeMatrix(resMatrix, rows);
    } else {
        freeMatrix(resMatrix, rows);
    }


    return 0;

}

complex double **MULMatrices(complex double **firMatrix, complex double **secMatrix, int rows, int cols) {
    double complex** result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = 0;
            for (int k = 0; k < cols; k++) {
                result[i][j] += firMatrix[i][k] * secMatrix[k][j];
            }
        }
    }
    return result;
}

double complex** subtractMatrices(double complex** firMatrix, double complex** secMatrix, int rows, int cols) {
    double complex** result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = firMatrix[i][j] - secMatrix[i][j];
        }
    }
    return result;}

double complex** ADDMatrices(double complex** firMatrix, double complex** secMatrix, int rows, int cols) {
    double complex** result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = firMatrix[i][j] + secMatrix[i][j];
        }
    }
    return result;
}

double complex** createMatrix(int rows, int cols) {
    double complex** matrix = (double complex**)malloc(rows * sizeof(double complex*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (double complex*)malloc(cols * sizeof(double complex));
    }
    return matrix;
}
void freeMatrix(double complex** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

complex double **inputMatrix(char input[MAX_SIZE], int *rows, int *cols){
    sscanf(input,"(%d,%d:",rows,cols);
    double complex** matrix = createMatrix(*rows, *cols);
    char* token = strtok(input, ":,");
    token = strtok(NULL, ",");
    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *cols; j++) {
            double real = 0, imag = 0;
            char* plus = strchr(token, '+');
            char* i = strchr(token, 'i');
            if (plus && i) {
                // Complex number
                sscanf(token, "%lf+%lfi", &real, &imag);
            } else if (i) {
                // Pure imaginary number
                sscanf(token, "%lfi", &imag);
            } else {
                // Real number (integer or double)
                sscanf(token, "%lf", &real);
            }
            matrix[i][j] = real + imag * I;
            token = strtok(NULL, ",)");
        }
    }
    return matrix;
}