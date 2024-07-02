
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <complex.h>
#include <semaphore.h>

#define MAX_SIZE 128
#define MAX_SHM 2048
#define MAX_OP_LEN 16

typedef struct {
    int rows;
    int cols;
    complex double data[1];
} Matrix;

typedef struct {
    Matrix matrix;
    char operation[MAX_OP_LEN];
} MatrixData;

typedef struct {
    int rows;
    int cols;
    complex double data[1];
} ResultMatrix;

// Function declarations
MatrixData* readMatrixData(void *shm_addr, int mat_counter, size_t total_size);
int matrixType(complex double *matrix, int rows, int cols);
complex double *createMatrix(int rows, int cols);
/*void freeMatrix(complex double *matrix);
//complex double *ADDMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
//complex double *SUBMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
//complex double *MULMatrices(complex double *firMatrix, complex double *secMatrix, int rows1, int cols1, int rows2, int cols2);
//complex double *TRANSPOSEMatrices(complex double *matrix, int rows, int cols);
//complex double *logANDmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
//complex double *logORmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
//complex double *logNOTmatrices(complex double *matrix, int rows, int cols);
 */
void printMatrix(complex double *matrix, int rows, int cols);

int main() {
    key_t key = ftok("/tmp", 'r');
    int shm_id = shmget(key, MAX_SHM, 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    void *shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    int *mat_counter = (int *)shm_addr;
    sem_t *sem = (sem_t *)(shm_addr + sizeof(int));
    void *matrix_storage_start = shm_addr + sizeof(int) + sizeof(sem_t);

    size_t total_size = sizeof(MatrixData) + (MAX_SIZE - 1) * sizeof(complex double);

    while (1) {
        printf("Waiting for matrix operations...\n");

        sem_wait(sem);

        if (*mat_counter == 0) {
            sem_post(sem);
            sleep(1);
            continue;
        }

        // Read all matrix operations from shared memory
        for (int i = 0; i < *mat_counter; i++) {
            MatrixData *matrix_data = readMatrixData(matrix_storage_start, i, total_size);
            if (matrix_data == NULL) continue;

            printf("Matrix %d: ", i + 1);
            printMatrix(matrix_data->matrix.data,matrix_data->matrix.rows,matrix_data->matrix.cols);
            printf("Operation: %s\n", matrix_data->operation);
        }

        *mat_counter = 0;

        sem_post(sem);

        printf("Enter 'REFRESH' to read again or 'EXIT' to exit: ");
        char command[MAX_SIZE];
        scanf("%s", command);

        if (strcmp(command, "EXIT") == 0) {
            break;
        }
    }

    shmdt(shm_addr);
    return 0;
}

MatrixData* readMatrixData(void *shm_addr, int mat_counter, size_t total_size) {
    void *target_addr = shm_addr + mat_counter * total_size;
    return (MatrixData *)target_addr;
}

int matrixType(complex double *matrix, int rows, int cols) {
    int is_integer = 1, is_real = 1;
    for (int i = 0; i < rows * cols; i++) {
        double real = creal(matrix[i]);
        double imag = cimag(matrix[i]);

        if (imag != 0.0) {
            return 2; // Complex matrix
        }
        if (real != (int)real) {
            is_integer = 0;
        }
        if (imag != 0.0 || real != (int)real) {
            is_real = 0;
        }
    }
    return is_integer ? 0 : 1;
}

/*complex double *logNOTmatrices(complex double *matrix, int rows, int cols) {
//    complex double *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result[i * cols + j] = (cabs(matrix[i * cols + j]) == 0) ? 1.0 : 0.0;
//        }
//    }
//    return result;
//}
//void freeMatrix(complex double *matrix) {
//    free(matrix);
//}
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
//}*/

complex double *createMatrix(int rows, int cols) {
    complex double *matrix = (complex double *) malloc(rows * cols * sizeof(complex double));
    return matrix;
}

void printMatrix(complex double *matrix, int rows, int cols) {
    printf("(%d,%d:", rows, cols);
    int type = matrixType(matrix, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double real = creal(matrix[i * cols + j]);
            double imag = cimag(matrix[i * cols + j]);

            if (type == 0) {
                printf("%d", (int)real);
            } else if (type == 1) {
                printf("%.1f", real);
            } else {
                if (imag == 0.0) {
                    printf("%d", (int)real);
                } else if (real == 0.0) {
                    printf("%di", (int)imag);
                } else {
                    printf("%d%+di", (int)real, (int)imag);
                }
            }

            if (i != rows - 1 || j != cols - 1) {
                printf(",");
            }
        }
    }
    printf(")\n");
}
