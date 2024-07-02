//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
//#include <pthread.h>
//#include <complex.h>
//#include <math.h>
//
//#define MAX_SIZE 128
//#define SHM_SIZE 2048
//#define MAX_MATRICES 10
//#define MAX_MATRIX_SIZE 10
//
//typedef struct {
//    int count;
//    complex double *matrices[MAX_MATRICES];
//    int rows[MAX_MATRICES];
//    int cols[MAX_MATRICES];
//    pthread_mutex_t mutex;
//} SharedData;
//
//// Function declarations
//int matrixType(complex double *matrix, int rows, int cols);
//complex double *createMatrix(int rows, int cols);
//void freeMatrix(complex double *matrix);
//complex double *ADDMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
//complex double *SUBMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
//complex double *MULMatrices(complex double *firMatrix, complex double *secMatrix, int rows1, int cols1, int rows2, int cols2);
//complex double *TRANSPOSEMatrices(complex double *matrix, int rows, int cols);
//complex double *logANDmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
//complex double *logORmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
//complex double *logNOTmatrices(complex double *matrix, int rows, int cols);
//void printMatrix(complex double *matrix, int rows, int cols);
//
//
//int main() {
//    int rows=0,cols=0;
//    key_t key = ftok("server.c", 'R');
//    int shm_id = shmget(key, sizeof(SharedData), 0666);
//    if (shm_id == -1) {
//        perror("shmget");
//        exit(1);
//    }
//
//    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
//    if (shared_data == (void *)-1) {
//        perror("shmat");
//        exit(1);
//    }
//
//    while (1) {
//        pthread_mutex_lock(&shared_data->mutex);
//        int count = shared_data->count;
//        pthread_mutex_unlock(&shared_data->mutex);
//
//        if (count == 0) {
//            sleep(1);
//            continue;
//        }
//
//        printf("Matrices in shared memory:\n");
//        for (int i = 0; i < count; i++) {
//            pthread_mutex_lock(&shared_data->mutex);
//            complex double *matrix = shared_data->matrices[i];
//            rows = shared_data->rows[i];
//            cols = shared_data->cols[i];
//            pthread_mutex_unlock(&shared_data->mutex);
//
//            printf("Matrix %d: ", i + 1);
//            printMatrix(matrix, rows, cols);
//        }
//
//        printf("Enter operation (ADD, SUB, MUL, TRANSPOSE, LOGAND, LOGOR, LOGNOT) or EXIT: ");
//        char operation[MAX_SIZE];
//        scanf("%s", operation);
//
//        if (strcmp(operation, "EXIT") == 0) {
//            break;
//        }
//
//        int index1, index2;
//        complex double *result = NULL;
//
//        if (strcmp(operation, "ADD") == 0 || strcmp(operation, "SUB") == 0 || strcmp(operation, "MUL") == 0 || strcmp(operation, "LOGAND") == 0 || strcmp(operation, "LOGOR") == 0) {
//            printf("Enter indices of two matrices (1-%d): ", count);
//            scanf("%d %d", &index1, &index2);
//            index1--;
//            index2--;
//
//            pthread_mutex_lock(&shared_data->mutex);
//            complex double *matrix1 = shared_data->matrices[index1];
//            int rows1 = shared_data->rows[index1];
//            int cols1 = shared_data->cols[index1];
//            complex double *matrix2 = shared_data->matrices[index2];
//            int rows2 = shared_data->rows[index2];
//            int cols2 = shared_data->cols[index2];
//            pthread_mutex_unlock(&shared_data->mutex);
//
//            if (strcmp(operation, "ADD") == 0) {
//                if (rows1 == rows2 && cols1 == cols2) {
//                    result = ADDMatrices(matrix1, matrix2, rows1, cols1);
//                } else {
//                    printf("Matrices dimensions do not match for addition\n");
//                }
//            } else if (strcmp(operation, "SUB") == 0) {
//                if (rows1 == rows2 && cols1 == cols2) {
//                    result = SUBMatrices(matrix1, matrix2, rows1, cols1);
//                } else {
//                    printf("Matrices dimensions do not match for subtraction\n");
//                }
//            } else if (strcmp(operation, "MUL") == 0) {
//                result = MULMatrices(matrix1, matrix2, rows1, cols1, rows2, cols2);
//            } else if (strcmp(operation, "LOGAND") == 0) {
//                if (rows1 == rows2 && cols1 == cols2) {
//                    result = logANDmatrices(matrix1, matrix2, rows1, cols1);
//                } else {
//                    printf("Matrices dimensions do not match for logical AND\n");
//                }
//            } else if (strcmp(operation, "LOGOR") == 0) {
//                if (rows1 == rows2 && cols1 == cols2) {
//                    result = logORmatrices(matrix1, matrix2, rows1, cols1);
//                } else {
//                    printf("Matrices dimensions do not match for logical OR\n");
//                }
//            }
//        } else if (strcmp(operation, "TRANSPOSE") == 0 || strcmp(operation, "LOGNOT") == 0) {
//            printf("Enter index of matrix (1-%d): ", count);
//            scanf("%d", &index1);
//            index1--;
//
//            pthread_mutex_lock(&shared_data->mutex);
//            complex double *matrix = shared_data->matrices[index1];
//            rows = shared_data->rows[index1];
//            cols = shared_data->cols[index1];
//            pthread_mutex_unlock(&shared_data->mutex);
//
//            if (strcmp(operation, "TRANSPOSE") == 0) {
//                result = TRANSPOSEMatrices(matrix, rows, cols);
//            } else if (strcmp(operation, "LOGNOT") == 0) {
//                result = logNOTmatrices(matrix, rows, cols);
//            }
//        }
//
//        if (result != NULL) {
//            printf("Result: ");
//            if (strcmp(operation, "TRANSPOSE") == 0) {
//                printMatrix(result, cols, rows);
//                freeMatrix(result);
//            } else {
//                printMatrix(result, shared_data->rows[index1], shared_data->cols[index1]);
//                freeMatrix(result);
//            }
//        }
//    }
//
//    shmdt(shared_data);
//    return 0;
//}
//int matrixType(complex double *matrix, int rows, int cols) {
//    int hasComplex = 0;
//    int hasDouble = 0;
//
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            double real = creal(matrix[i * cols + j]);
//            double imag = cimag(matrix[i * cols + j]);
//
//            if (imag != 0.0) {
//                hasComplex = 1;
//            } else if (floor(real) != real) {
//                hasDouble = 1;
//            }
//        }
//    }
//
//    if (hasComplex) {
//        return 2;
//    } else if (hasDouble) {
//        return 1;
//    } else {
//        return 0;
//    }
//}
//
//complex double *logNOTmatrices(complex double *matrix, int rows, int cols) {
//    complex double *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result[i * cols + j] = (cabs(matrix[i * cols + j]) == 0) ? 1.0 : 0.0;
//        }
//    }
//    return result;
//}
//
//complex double *logORmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols) {
//    complex double *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result[i * cols + j] = (cabs(firMatrix[i * cols + j]) != 0 || cabs(secMatrix[i * cols + j]) != 0) ? 1.0 : 0.0;
//        }
//    }
//    return result;
//}
//
//complex double *logANDmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols) {
//    complex double *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result[i * cols + j] = (cabs(firMatrix[i * cols + j]) != 0 && cabs(secMatrix[i * cols + j]) != 0) ? 1.0 : 0.0;
//        }
//    }
//    return result;
//}
//
//complex double *TRANSPOSEMatrices(complex double *matrix, int rows, int cols) {
//    complex double *result = createMatrix(cols, rows);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result[j * rows + i] = matrix[i * cols + j];
//        }
//    }
//    return result;
//}
//
//complex double *MULMatrices(complex double *firMatrix, complex double *secMatrix, int rows1, int cols1, int rows2, int cols2) {
//    if (cols1 != rows2) {
//        printf("Matrix multiplication is not possible\n");
//        return NULL;
//    }
//    complex double *result = createMatrix(rows1, cols2);
//    for (int i = 0; i < rows1; i++) {
//        for (int j = 0; j < cols2; j++) {
//            result[i * cols2 + j] = 0 + 0 * I;
//            for (int k = 0; k < cols1; k++) {
//                result[i * cols2 + j] += firMatrix[i * cols1 + k] * secMatrix[k * cols2 + j];
//            }
//        }
//    }
//    return result;
//}
//
//complex double *SUBMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols) {
//    complex double *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result[i * cols + j] = firMatrix[i * cols + j] - secMatrix[i * cols + j];
//        }
//    }
//    return result;
//}
//
//complex double *ADDMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols) {
//    complex double *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result[i * cols + j] = firMatrix[i * cols + j] + secMatrix[i * cols + j];
//        }
//    }
//    return result;
//}
//
//complex double *createMatrix(int rows, int cols) {
//    complex double *matrix = (complex double *)malloc(rows * cols * sizeof(complex double));
//    return matrix;
//}
//
//void freeMatrix(complex double *matrix) {
//    free(matrix);
//}
//
//void printMatrix(complex double *matrix, int rows, int cols) {
//    printf("(%d,%d:", rows, cols);
//    int type = matrixType(matrix, rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            double real = creal(matrix[i * cols + j]);
//            double imag = cimag(matrix[i * cols + j]);
//
//            if (type == 0) {
//                printf("%d", (int)real);
//            } else if (type == 1) {
//                printf("%.1f", real);
//            } else {
//                if (imag == 0.0) {
//                    printf("%d", (int)real);
//                } else if (real == 0.0) {
//                    printf("%di", (int)imag);
//                } else {
//                    printf("%d%+di", (int)real, (int)imag);
//                }
//            }
//
//            if (i != rows - 1 || j != cols - 1) {
//                printf(",");
//            }
//        }
//    }
//    printf(")\n");
//}
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <complex.h>
#include <semaphore.h>

#define MAX_SHM 2048

typedef struct {
    int rows;
    int cols;
    complex double data[1];
} Matrix;

typedef struct {
    Matrix matrix;
    char operation[16];
} MatrixData;

// Function prototypes
void printMatrix(complex double **matrix, int rows, int cols);
int matrixType(complex double **matrix, int rows, int cols);

int main() {
    key_t key = ftok("/tmp", 'x');
    int shm_id = shmget(key, MAX_SHM, 0600);

    if (shm_id < 0) {
        perror("client: shmget:");
        exit(1);
    }

    void* shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1) {
        perror("client: shmat:");
        exit(1);
    }

    int *mat_counter = (int*)shm_addr;
    void *matrix_storage_start = shm_addr + sizeof(int) + sizeof(sem_t);

    for (int i = 0; i < *mat_counter; i++) {
        MatrixData *matrix_op = (MatrixData *)(matrix_storage_start + i * (sizeof(MatrixData) + (matrix_op->matrix.rows * matrix_op->matrix.cols - 1) * sizeof(complex double)));
        printf("%d: \n",i+1);
        printf("Operation: %s\n", matrix_op->operation);

        // Allocate memory to create a matrix representation
        complex double **matrix = malloc(matrix_op->matrix.rows * sizeof(complex double *));
        for (int r = 0; r < matrix_op->matrix.rows; r++) {
            matrix[r] = malloc(matrix_op->matrix.cols * sizeof(complex double));
            for (int c = 0; c < matrix_op->matrix.cols; c++) {
                matrix[r][c] = matrix_op->matrix.data[r * matrix_op->matrix.cols + c];
            }
        }

        // Print the matrix
        printMatrix(matrix, matrix_op->matrix.rows, matrix_op->matrix.cols);

        // Free the allocated matrix
        for (int r = 0; r < matrix_op->matrix.rows; r++) {
            free(matrix[r]);
        }
        free(matrix);
    }

    shmdt(shm_addr);
    return 0;
}

// Function to print the matrix
void printMatrix(complex double **matrix, int rows, int cols) {

    printf("(%d,%d:", rows, cols);
    int type = matrixType(matrix, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double real = creal(matrix[i][j]);
            double imag = cimag(matrix[i][j]);

            if (type == 0) {
                // Integer
                printf("%d", (int) real);
            } else if (type == 1) {
                // Double
                printf("%.1f", real);
            } else {
                // Complex
                if (imag == 0.0) {
                    printf("%d", (int) real);
                } else if (real == 0.0) {
                    printf("%di", (int) imag);
                } else {
                    if (imag > 0) {
                        printf("%d%+di", (int) real, (int) imag);
                    }
                    else if (imag < 0){
                        printf("%d%-di", (int) real, (int) imag);
                    }
                    else{
                        printf("0");
                    }
                }
            }

            if (i != rows - 1 || j != cols - 1) {
                printf(",");
            }
        }
    }
    printf(")\n");
}

// Determine the type of the matrix values
int matrixType(complex double **matrix, int rows, int cols) {
    int isInteger = 1, isDouble = 1;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double real = creal(matrix[i][j]);
            double imag = cimag(matrix[i][j]);

            if (imag != 0.0) {
                return 2; // Complex
            }

            if (real != (int)real) {
                isInteger = 0;
            }
            if (real != (double)(int)real) {
                isDouble = 0;
            }
        }
    }

    if (isInteger) return 0; // Integer
    if (isDouble) return 1;  // Double
    return 2;                // Complex
}

