#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>

int** create2DArray(int rows, int cols);
void free2DArray(int** array, int rows);
int isInteger(double num);
void checkMatrixType(double complex* matrix, int n);
void parseInput(char* input, int* rows, int* cols, int** matrix);
void fill2DArray(int** array, int* matrix, int rows, int cols);

// Function to create a dynamically allocated 2D array
int** create2DArray(int rows, int cols) {
    int** array = (int**)malloc(rows * sizeof(int*));
    if (array == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < rows; i++) {
        array[i] = (int*)malloc(cols * sizeof(int));
        if (array[i] == NULL) {
            printf("Memory allocation failed\n");
            exit(1);
        }
    }

    return array;
}

// Function to free the dynamically allocated 2D array
void free2DArray(int** array, int rows) {
    for (int i = 0; i < rows; i++) {
        free(array[i]);
    }
    free(array);
}

// Function to check if a double is an integer
int isInteger(double num) {
    return fmod(num, 1.0) == 0.0;
}

// Function to check the type of numbers in a 2D matrix
void checkMatrixType(double complex* matrix, int n) {
    int allIntegers = 1;  // Assume all integers initially
    int hasComplex = 0;   // Flag to indicate if there are complex numbers

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double complex element = matrix[i * n + j];

            if (cimag(element) != 0.0) {
                hasComplex = 1;
                allIntegers = 0;
                break;
            } else if (!isInteger(creal(element))) {
                allIntegers = 0;
            }
        }
        if (hasComplex) break;
    }

    if (hasComplex) {
        printf("The matrix contains complex numbers.\n");
    } else if (allIntegers) {
        printf("The matrix contains only integers.\n");
    } else {
        printf("The matrix contains doubles.\n");
    }
}

// Function to parse the input string to extract matrix dimensions and elements
void parseInput(char* input, int* rows, int* cols, int** matrix) {
    sscanf(input, "(%d,%d:", rows, cols);

    // Allocate memory for the matrix elements
    *matrix = (int*)malloc((*rows) * (*cols) * sizeof(int));
    if (*matrix == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    // Move to the position of the first matrix element in the input string
    char* matrixElements = strchr(input, ':') + 1;

    // Read matrix elements
    for (int i = 0; i < (*rows) * (*cols); i++) {
        sscanf(matrixElements, "%d", (*matrix) + i);
        // Move to the next element
        matrixElements = strchr(matrixElements, ',') + 1;
    }
}

// Function to fill the dynamically allocated 2D array with matrix elements
void fill2DArray(int** array, int* matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            array[i][j] = matrix[i * cols + j];
        }
    }
}
void printMatrix(int** matrix, int rows, int cols) {
    printf("(%d,%d:",rows,cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d,", matrix[i][j]);
        }
    }
    printf(")");
}
int main() {
    char input1[128], input2[128], operation[10];

    // Read inputs for the first matrix
    printf("Enter the first matrix in the format (rows,cols:elements): ");
    fgets(input1, sizeof(input1), stdin);

    int rows1 = 0, cols1 = 0;
    int *matrixElements1;
    parseInput(input1, &rows1, &cols1, &matrixElements1);

    int **matrix1 = create2DArray(rows1, cols1);
    fill2DArray(matrix1, matrixElements1, rows1, cols1);

    // Read inputs for the second matrix
    printf("Enter the second matrix in the format (rows,cols:elements): ");
    fgets(input2, sizeof(input2), stdin);

    int rows2 = 0, cols2 = 0;
    int *matrixElements2;
    parseInput(input2, &rows2, &cols2, &matrixElements2);

    int **matrix2 = create2DArray(rows2, cols2);
    fill2DArray(matrix2, matrixElements2, rows2, cols2);

    // Read the operation
    printf("Enter operation (ADD, SUB, MUL, TRANSPOSE): ");
    scanf("%s", operation);

    int **result = NULL;
    if (strcmp(operation, "ADD") == 0) {
        // Add matrices
        result = create2DArray(rows1, cols1);
        for (int i = 0; i < rows1; i++) {
            for (int j = 0; j < cols1; j++) {
                result[i][j] = matrix1[i][j] + matrix2[i][j];
            }
        }
    }
    else if (strcmp(operation, "SUB") == 0) {
        // Subtract matrices
        result = create2DArray(rows1, cols1);
        for (int i = 0; i < rows1; i++) {
            for (int j = 0; j < cols1; j++) {
                result[i][j] = matrix1[i][j] - matrix2[i][j];
            }
        }
    }
    else if (strcmp(operation, "MUL") == 0) {
        // Multiply matrices
        result = create2DArray(rows1, cols2);
        for (int i = 0; i < rows1; i++) {
            for (int j = 0; j < cols2; j++) {
                result[i][j] = 0;
                for (int k = 0; k < cols1; k++) {
                    result[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
        }
    } else if (strcmp(operation, "TRANSPOSE") == 0) {
        // Transpose the first matrix
        result = create2DArray(cols1, rows1);
        for (int i = 0; i < rows1; i++) {
            for (int j = 0; j < cols1; j++) {
                result[j][i] = matrix1[i][j];
            }
        }
    } else {
        printf("Invalid operation\n");
        return 1;
    }

    // Print the result matrix
    if (result != NULL) {
        printf("Result:\n");
        printMatrix(result, (strcmp(operation, "TRANSPOSE") == 0) ? cols1 : rows1,
                    (strcmp(operation, "TRANSPOSE") == 0) ? rows1 : cols1);
        free2DArray(result, (strcmp(operation, "TRANSPOSE") == 0) ? cols1 : rows1);
    }

    // Free the allocated memory
    free(matrixElements1);
    free(matrixElements2);
    free2DArray(matrix1, rows1);
    free2DArray(matrix2, rows2);

    return 0;

}