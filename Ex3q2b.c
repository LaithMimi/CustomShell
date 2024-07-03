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
typedef complex double cmpdouble;

typedef struct {
    int rows;
    int cols;
    cmpdouble data[MAX_SIZE];
} Matrix;

typedef struct {
    Matrix matrix;
    char operation[MAX_OP_LEN];
} shmData;

typedef struct {
    int rows;
    int cols;
    cmpdouble data[MAX_SIZE];
} ResultMatrix;

// Function declarations
shmData* readMatrixData(void *shm_addr, int mat_counter, size_t total_size);
int matrixType(cmpdouble *matrix, int rows, int cols);
void freeMatrix(ResultMatrix *matrix);
void printMatrix(ResultMatrix* matrix);

ResultMatrix *createMatrix(int rows, int cols);
ResultMatrix *ADD(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols);
ResultMatrix *SUB(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols);
ResultMatrix *MUL(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows1, int cols1, int rows2, int cols2);
ResultMatrix *TRANSPOSE(cmpdouble *matrix, int rows, int cols);
ResultMatrix *AND(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols);
ResultMatrix *OR(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols);
ResultMatrix *NOT(cmpdouble *matrix, int rows, int cols);

int main() {
    key_t key = ftok("/tmp", 'Y');
    int shm_id = shmget(key, MAX_SHM, 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    if (shmat(shm_id, NULL, 0) == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    int *mat_counter = (int *) shmat(shm_id, NULL, 0);
    sem_t *sem = (sem_t *) (shmat(shm_id, NULL, 0) + sizeof(int));
    void *storageStart = shmat(shm_id, NULL, 0) + sizeof(int) + sizeof(sem_t);
    size_t total_size = sizeof(shmData) + (MAX_SIZE - 1) * sizeof(cmpdouble);

    while (1) {
        sem_wait(sem); //wait for the sem to be available

        if (*mat_counter == 0) { //if no matrices
            sem_post(sem);//release the sem
            sleep(1);
            continue;
        }

        // Read all matrix operations from shared memory
        for (int i = 0; i < *mat_counter; i++) {
            shmData *firMatrix = readMatrixData(storageStart, i, total_size);
            if (firMatrix == NULL) continue;
            if (strcmp(firMatrix->operation, "END") == 0) {
                sem_post(sem);
                sem_destroy(sem);
                shmdt(mat_counter);
                shmctl(shm_id, IPC_RMID, NULL);
                exit(0);
            }

            ResultMatrix *result = NULL;
            if (strcmp(firMatrix->operation, "NOT\n") == 0) {
                result = (ResultMatrix *) NOT(firMatrix->matrix.data, firMatrix->matrix.rows,
                                              firMatrix->matrix.cols);
            }
            else if (strcmp(firMatrix->operation, "TRANSPOSE\n") == 0) {
                result = (ResultMatrix *) TRANSPOSE(firMatrix->matrix.data, firMatrix->matrix.rows,
                                                    firMatrix->matrix.cols);
            }
            else if (i + 1 < *mat_counter) {
                shmData *secMatrix = readMatrixData(storageStart, i + 1, total_size);
                if (secMatrix == NULL) continue;
                if (strcmp(firMatrix->operation, "ADD\n") == 0) {
                    result = (ResultMatrix *) ADD(firMatrix->matrix.data, secMatrix->matrix.data,
                                                  firMatrix->matrix.rows, firMatrix->matrix.cols);
                }
                else if (strcmp(firMatrix->operation, "SUB\n") == 0) {
                    result = (ResultMatrix *) SUB(firMatrix->matrix.data, secMatrix->matrix.data,
                                                  firMatrix->matrix.rows, firMatrix->matrix.cols);
                }
                else if (strcmp(firMatrix->operation, "MUL\n") == 0) {
                    result = (ResultMatrix *) MUL(firMatrix->matrix.data, secMatrix->matrix.data,
                                                  firMatrix->matrix.rows, firMatrix->matrix.cols,
                                                  secMatrix->matrix.rows, secMatrix->matrix.cols);
                }
                else if (strcmp(firMatrix->operation, "AND\n") == 0) {
                    result = (ResultMatrix *) AND(firMatrix->matrix.data, secMatrix->matrix.data,
                                                  firMatrix->matrix.rows, firMatrix->matrix.cols);
                }
                else if (strcmp(firMatrix->operation, "OR\n") == 0) {
                    result = (ResultMatrix *) OR(firMatrix->matrix.data, secMatrix->matrix.data,
                                                 firMatrix->matrix.rows, firMatrix->matrix.cols);
                }
                i++; //skip the next matrix as we've used it in this operation
            }
            if (result != NULL) {
                printf("Result: ");
                printMatrix(result);
                freeMatrix( result);
            } else {
                printf("Operation not performed or invalid.\n");
            }

        }
        *mat_counter = 0; //reset the counter to indicate all matrices have been processed
        sem_post(sem); //release the semaphore
    }

    shmdt(shmat(shm_id, NULL, 0));

    return 0;
}

shmData* readMatrixData(void *shm_addr, int mat_counter, size_t total_size) {
    void *target_addr = shm_addr + mat_counter * total_size;
    return (shmData *)target_addr;
}

int matrixType(cmpdouble *matrix, int rows, int cols) {
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
}
void printMatrix(ResultMatrix* matrix) {
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

ResultMatrix *NOT(cmpdouble *matrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j]  = (cabs(matrix[i * cols + j]) == 0) ? 1.0 : 0.0;
        }
    }
    return result;
}
ResultMatrix *OR(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j] = (cabs(firMatrix[i * cols + j]) != 0 || cabs(secMatrix[i * cols + j]) != 0) ? 1.0 : 0.0;
        }
    }
    return result;
}
ResultMatrix *AND(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
       for (int j = 0; j < cols; j++) {
           result->data[i * cols + j]  = (cabs(firMatrix[i * cols + j]) != 0 && cabs(secMatrix[i * cols + j]) != 0) ? 1.0 : 0.0;
        }
    }
    return result;
}
ResultMatrix *TRANSPOSE(cmpdouble *matrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(cols, rows);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j]  = matrix[i * cols + j];
        }
    }
    return result;
}
ResultMatrix *MUL(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows1, int cols1, int rows2, int cols2) {
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
ResultMatrix *SUB(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols) {
    ResultMatrix *result = createMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result->data[i * cols + j] = firMatrix[i * cols + j] - secMatrix[i * cols + j];
        }
    }
    return result;
}
ResultMatrix *ADD(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols) {
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

