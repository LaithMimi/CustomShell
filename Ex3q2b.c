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
} shmData;

typedef struct {
    int rows;
    int cols;
    complex double data[1];
} ResultMatrix;

// Function declarations
shmData* readMatrixData(void *shm_addr, int mat_counter, size_t total_size);
int matrixType(complex double *matrix, int rows, int cols);
void freeMatrix(ResultMatrix *matrix);
void printMatrix(ResultMatrix* matrix);

ResultMatrix *createMatrix(int rows, int cols);
ResultMatrix *ADDMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
ResultMatrix *SUBMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
ResultMatrix *MULMatrices(complex double *firMatrix, complex double *secMatrix, int rows1, int cols1, int rows2, int cols2);
ResultMatrix *TRANSPOSEMatrices(complex double *matrix, int rows, int cols);
ResultMatrix *logANDmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
ResultMatrix *logORmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols);
ResultMatrix *logNOTmatrices(complex double *matrix, int rows, int cols);

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

    size_t total_size = sizeof(shmData) + (MAX_SIZE - 1) * sizeof(complex double);

    while (1) {
       // printf("Waiting for matrix operations...\n");

/*there is problems*/

        sem_wait(sem);

        if (*mat_counter == 0) {
            sem_post(sem);
            sleep(1);
            continue;
        }

        // Read all matrix operations from shared memory
        for (int i = 0; i < *mat_counter; i++) {
            shmData *firMatrix = readMatrixData(matrix_storage_start, i, total_size);
            if (firMatrix == NULL) continue;

            ResultMatrix *result = NULL;
            if (strcmp(firMatrix->operation, "NOT") == 0) {
                result = (ResultMatrix *) logNOTmatrices(firMatrix->matrix.data, firMatrix->matrix.rows,
                                                         firMatrix->matrix.cols);
            }
            else if (strcmp(firMatrix->operation, "TRANSPOSE") == 0) {
                result = (ResultMatrix *) TRANSPOSEMatrices(firMatrix->matrix.data, firMatrix->matrix.rows,
                                                            firMatrix->matrix.cols);
            }
            else if (i + 1 < *mat_counter) {
                shmData *secMatrix = readMatrixData(matrix_storage_start, i + 1, total_size);
                if (secMatrix == NULL) continue;
                if (strcmp(firMatrix->operation, "ADD") == 0) {
                    result = (ResultMatrix *) ADDMatrices(firMatrix->matrix.data, secMatrix->matrix.data,
                                                          firMatrix->matrix.rows, firMatrix->matrix.cols);
                } else if (strcmp(firMatrix->operation, "SUB") == 0) {
                    result = (ResultMatrix *) SUBMatrices(firMatrix->matrix.data, secMatrix->matrix.data,
                                                          firMatrix->matrix.rows, firMatrix->matrix.cols);
                } else if (strcmp(firMatrix->operation, "MUL") == 0) {
                    result = (ResultMatrix *) MULMatrices(firMatrix->matrix.data, secMatrix->matrix.data,
                                                          firMatrix->matrix.rows, firMatrix->matrix.cols,
                                                          secMatrix->matrix.rows, secMatrix->matrix.cols);
                } else if (strcmp(firMatrix->operation, "AND") == 0) {
                    result = (ResultMatrix *) logANDmatrices(firMatrix->matrix.data, secMatrix->matrix.data,
                                                             firMatrix->matrix.rows, firMatrix->matrix.cols);
                } else if (strcmp(firMatrix->operation, "OR") == 0) {
                    result = (ResultMatrix *) logORmatrices(firMatrix->matrix.data, secMatrix->matrix.data,
                                                            firMatrix->matrix.rows, firMatrix->matrix.cols);
                }
                i++; // Skip the next matrix as we've used it in this operation
            }
            if (result != NULL) {
                printf("Result: ");
                printMatrix(result);
                freeMatrix( result);
            } else {
                printf("Operation not performed or invalid.\n");
            }

        }
        *mat_counter = 0;

        sem_post(sem);

        printf("Enter 'REFRESH' to read again or 'END' to exit: ");
        char command[MAX_SIZE];
        if (fgets(command, MAX_SIZE, stdin) == NULL) {
            perror("fgets");
            break;
        }
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "END") == 0) {
            break;
        }else if (strcmp(command, "REFRESH") != 0) {
            printf("Invalid command. Please enter 'REFRESH' or 'END'.\n");
        }
    }

    if (shmdt(shm_addr) == -1) {
        perror("shmdt");
    }

    return 0;
}

shmData* readMatrixData(void *shm_addr, int mat_counter, size_t total_size) {
    void *target_addr = shm_addr + mat_counter * total_size;
    return (shmData *)target_addr;
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
void freeMatrix(ResultMatrix *matrix) {
    free(matrix);
}void printMatrix(ResultMatrix* matrix) {
    printf("(%d,%d:", matrix->rows, matrix->cols);
    int type = matrixType(matrix->data, matrix->rows, matrix->cols);
    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            double real = creal(matrix->data[i * matrix->cols + j]);
            double imag = cimag(matrix->data[i * matrix->cols + j]);

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
            if (i != matrix->rows - 1 || j != matrix->cols - 1) {
                printf(",");
            }
        }
    }
    printf(")\n");
}

ResultMatrix *logNOTmatrices(complex double *matrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j]  = (cabs(matrix[i * cols + j]) == 0) ? 1.0 : 0.0;
        }
    }
    return result;
}
ResultMatrix *logORmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j] = (cabs(firMatrix[i * cols + j]) != 0 || cabs(secMatrix[i * cols + j]) != 0) ? 1.0 : 0.0;
        }
    }
    return result;
}
ResultMatrix *logANDmatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
       for (int j = 0; j < cols; j++) {
           result->data[i * cols + j]  = (cabs(firMatrix[i * cols + j]) != 0 && cabs(secMatrix[i * cols + j]) != 0) ? 1.0 : 0.0;
        }
    }
    return result;
}
ResultMatrix *TRANSPOSEMatrices(complex double *matrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(cols, rows);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j]  = matrix[i * cols + j];
        }
    }
    return result;
}
ResultMatrix *MULMatrices(complex double *firMatrix, complex double *secMatrix, int rows1, int cols1, int rows2, int cols2) {
    ResultMatrix *result = createMatrix(rows1, cols2);
    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols2; j++) {
            result->data[i * cols2 + j] = 0 + 0 * I;
            for (int k = 0; k < cols1; k++) {
                result->data[i * cols1 + j]  += firMatrix[i * cols1 + k] * secMatrix[k * cols2 + j];
            }
        }
    }
    return result;
}
ResultMatrix *SUBMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j] = firMatrix[i * cols + j] - secMatrix[i * cols + j];
        }
    }
    return result;
}
ResultMatrix *ADDMatrices(complex double *firMatrix, complex double *secMatrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j] = firMatrix[i * cols + j] + secMatrix[i * cols + j];
        }
    }
    return result;
}
ResultMatrix *createMatrix(int rows, int cols) {
    size_t size = sizeof(ResultMatrix) + (rows * cols - 1) * sizeof(complex double);
    ResultMatrix* result = (ResultMatrix*)malloc(size);
    if (result == NULL) {
        perror("Failed to allocate memory for ResultMatrix");
        exit(1);
    }
    result->rows = rows;
    result->cols = cols;
    return result;
}

