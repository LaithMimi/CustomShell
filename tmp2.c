//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
//#include <complex.h>
//#include <semaphore.h>
//
//#define MAX_SIZE 128
//#define MAX_SHM 2048
//#define MAX_OP_LEN 16
//
//typedef complex double cmpdouble;
//
//typedef struct {
//    int rows;
//    int cols;
//    cmpdouble data[MAX_SIZE];
//} Matrix;
//
//typedef struct {
//    Matrix matrix;
//    char operation[MAX_OP_LEN];
//} shmData;
//
//typedef struct {
//    int rows;
//    int cols;
//    cmpdouble data[MAX_SIZE];
//} ResultMatrix;
//
//// Function declarations
//int matrixType(cmpdouble *matrix, int rows, int cols);
//void freeMatrix(ResultMatrix *matrix);
//void printMatrix(ResultMatrix* matrix);
//
//ResultMatrix *createMatrix(int rows, int cols);
//ResultMatrix *ADD(cmpdouble *firMatrix, cmpdouble *secMatrix, int rows, int cols);
//ResultMatrix *SUB(cmpdouble *firMatrix, cmpdouble *secMatrix, int rows, int cols);
//ResultMatrix *MUL(cmpdouble *firMatrix, cmpdouble *secMatrix, int rows1, int cols1, int rows2, int cols2);
//ResultMatrix *TRANSPOSE(cmpdouble *matrix, int rows, int cols);
//ResultMatrix *AND(cmpdouble *firMatrix, cmpdouble *secMatrix, int rows, int cols);
//ResultMatrix *OR(cmpdouble *firMatrix, cmpdouble *secMatrix, int rows, int cols);
//ResultMatrix *NOT(cmpdouble *matrix, int rows, int cols);
//shmData* readMatrixData(void *shm_addr, int mat_counter, size_t total_size);
//
//int main() {
//    key_t key = ftok("/tmp", 'L');
//    int shm_id = shmget(key, MAX_SHM, 0666);
//    if (shm_id == -1) {
//        perror("shmget");
//        exit(1);
//    }
//
//
//    if (shmat(shm_id, NULL, 0) == (void *)-1) {
//        perror("shmat");
//        exit(1);
//    }
//
//    int *mat_counter = (int *)shmat(shm_id, NULL, 0);
//    sem_t *sem = (sem_t *)(shmat(shm_id, NULL, 0) + sizeof(int));
//    void *storageStart = shmat(shm_id, NULL, 0) + sizeof(int) + sizeof(sem_t);
//    size_t total_size = sizeof(shmData);
//
//    while (1) {
//        if (sem_wait(sem) == -1) {
//            perror("sem_wait failed");
//            exit(1);
//        }
//
//        if (*mat_counter == 0) {
//            if (sem_post(sem) == -1) {
//                perror("sem_post failed");
//                exit(1);
//            }
//            sleep(1);
//            continue;
//        }
//
//        for (int i = 0; i < *mat_counter; i++) {
//            shmData *firMatrix = readMatrixData(storageStart, i, total_size);
//
//            printf("Processing operation: %s\n", firMatrix->operation);
//
//            if (strcmp(firMatrix->operation, "END") == 0) {
//                if (sem_post(sem) == -1) {
//                    perror("sem_post failed");
//                }
//                if (sem_destroy(sem) == -1) {
//                    perror("sem_destroy failed");
//                }
//                if (shmdt(shmat(shm_id, NULL, 0)) == -1) {
//                    perror("shmdt failed");
//                }
//                if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
//                    perror("shmctl failed");
//                }
//                exit(0);
//            }
//
//            ResultMatrix *result = NULL;
//            if (strcmp(firMatrix->operation, "NOT") == 0) {
//                result = NOT(firMatrix->matrix.data, firMatrix->matrix.rows, firMatrix->matrix.cols);
//            } else if (strcmp(firMatrix->operation, "TRANSPOSE") == 0) {
//                result = TRANSPOSE(firMatrix->matrix.data, firMatrix->matrix.rows, firMatrix->matrix.cols);
//            } else {
//                shmData *secMatrix = readMatrixData(storageStart, i + 1, total_size);
//
//                if (strcmp(firMatrix->operation, "ADD") == 0) {
//                    result = ADD(firMatrix->matrix.data, secMatrix->matrix.data,
//                                 firMatrix->matrix.rows, firMatrix->matrix.cols);
//                }
//                else if (strcmp(firMatrix->operation, "SUB") == 0) {
//                    result = SUB(firMatrix->matrix.data, secMatrix->matrix.data,
//                                 firMatrix->matrix.rows, firMatrix->matrix.cols);
//                }
//                else if (strcmp(firMatrix->operation, "MUL") == 0) {
//                    result = MUL(firMatrix->matrix.data, secMatrix->matrix.data,
//                                 firMatrix->matrix.rows, firMatrix->matrix.cols,
//                                 secMatrix->matrix.rows, secMatrix->matrix.cols);
//                }
//                else if (strcmp(firMatrix->operation, "AND") == 0) {
//                    result = AND(firMatrix->matrix.data, secMatrix->matrix.data,
//                                 firMatrix->matrix.rows, firMatrix->matrix.cols);
//                }
//                else if (strcmp(firMatrix->operation, "OR") == 0) {
//                    result = OR(firMatrix->matrix.data, secMatrix->matrix.data,
//                                firMatrix->matrix.rows, firMatrix->matrix.cols);
//                }
//                i++;
//            }
//            if (result != NULL) {
//                printf("Result: ");
//                printMatrix(result);
//                freeMatrix(result);
//            } else {
//                fprintf(stderr, "Operation %s not performed or invalid.\n", firMatrix->operation);
//            }
//        }
//        *mat_counter = 0;
//        if (sem_post(sem) == -1) {
//            perror("sem_post failed");
//            exit(1);
//        }
//    }
//
//    if (shmdt(shmat(shm_id, NULL, 0)) == -1) {
//        perror("shmdt failed");
//        exit(1);
//    }
//
//    return 0;
//}
//
//shmData* readMatrixData(void *shm_addr, int mat_counter, size_t total_size) {
//    if (shm_addr == NULL) {
//        fprintf(stderr, "Error: shm_addr is NULL in readMatrixData\n");
//        return NULL;
//    }
//
//    // Calculate the target address
//    char *base = (char *)shm_addr;
//    char *target_addr = base + mat_counter * total_size;
//
//    // Print debug information
//    printf("Base address: %p\n", (void*)base);
//    printf("Target address: %p\n", (void*)target_addr);
//    printf("End address: %p\n", (void*)(base + MAX_SHM));
//
//
//    // Access the data
//    shmData *data = (shmData *)target_addr;
//
////    // Validate matrix dimensions
////    if (data->matrix.rows <= 0 || data->matrix.rows > MAX_SIZE ||
////        data->matrix.cols <= 0 || data->matrix.cols > MAX_SIZE) {
////        fprintf(stderr, "Error: Invalid matrix dimensions read: %d x %d\n",
////                data->matrix.rows, data->matrix.cols);
////        return NULL;
////    }
//
//    // Validate operation string
//    if (strlen(data->operation) == 0 || strlen(data->operation) >= MAX_OP_LEN) {
//        fprintf(stderr, "Error: Invalid operation string length: %zu\n",
//                strlen(data->operation));
//        return NULL;
//    }
//
//    // Print debug information
//    printf("Read matrix: rows=%d, cols=%d, operation=%s\n",
//           data->matrix.rows, data->matrix.cols, data->operation);
//
//    printf("First few elements: ");
//    for (int i = 0; i < 5 && i < data->matrix.rows * data->matrix.cols; i++) {
//        printf("%.2f+%.2fi ", creal(data->matrix.data[i]), cimag(data->matrix.data[i]));
//    }
//    printf("\n");
//
//    return data;
//}
//
//
//
//int matrixType(cmpdouble *matrix, int rows, int cols) {
//    int is_integer = 1, is_real = 1;
//    for (int i = 0; i < rows * cols; i++) {
//        double real = creal(matrix[i]);
//        double imag = cimag(matrix[i]);
//
//        if (imag != 0.0) {
//            return 2; // Complex matrix
//        }
//        if (real != (int)real) {
//            is_integer = 0;
//        }
//        if (imag != 0.0 || real != (int)real) {
//            is_real = 0;
//        }
//    }
//    return is_integer ? 0 : 1;
//}
//
//void freeMatrix(ResultMatrix *matrix) {
//    free(matrix);
//}
//
//void printMatrix(ResultMatrix* matrix) {
//    printf("(%d,%d:", matrix->rows, matrix->cols);
//    int type = matrixType(matrix->data, matrix->rows, matrix->cols);
//    for (int i = 0; i < matrix->rows; i++) {
//        for (int j = 0; j < matrix->cols; j++) {
//            double real = creal(matrix->data[i * matrix->cols + j]);
//            double imag = cimag(matrix->data[i * matrix->cols + j]);
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
//            if (i != matrix->rows-1 || j != matrix->cols-1) {
//                printf(",");
//            }
//        }
//    }
//    printf(")\n");
//}
//
//ResultMatrix *NOT(cmpdouble *matrix, int rows, int cols) {
//    ResultMatrix *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result->data[i * cols + j]  = (cabs(matrix[i * cols + j]) == 0) ? 1.0 : 0.0;
//        }
//    }
//    return result;
//}
//ResultMatrix *OR(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols) {
//    ResultMatrix *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result->data[i * cols + j] = (cabs(firMatrix[i * cols + j]) != 0 || cabs(secMatrix[i * cols + j]) != 0) ? 1.0 : 0.0;
//        }
//    }
//    return result;
//}
//ResultMatrix *AND(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols) {
//    printf("in ADD\n");
//    ResultMatrix *result = createMatrix(rows, cols);
//    printf("in ADD 1\n");
//    for (int i = 0; i < rows; i++) {
//       for (int j = 0; j < cols; j++) {
//           printf("in ADD 2\n");
//           result->data[i * cols + j]  = (cabs(firMatrix[i * cols + j]) != 0 && cabs(secMatrix[i * cols + j]) != 0) ? 1.0 : 0.0;
//        }
//    }
//    return result;
//}
//ResultMatrix *TRANSPOSE(cmpdouble *matrix, int rows, int cols) {
//    ResultMatrix *result = createMatrix(cols, rows);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result->data[i * cols + j]  = matrix[i * cols + j];
//        }
//    }
//    return result;
//}
//ResultMatrix *MUL(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows1, int cols1, int rows2, int cols2) {
//    ResultMatrix *result = createMatrix(rows1, cols2);
//    for (int i = 0; i < rows1; i++) {
//        for (int j = 0; j < cols2; j++) {
//            result->data[i * cols2 + j] = 0 + 0 * I;
//            for (int k = 0; k < cols1; k++) {
//                result->data[i * cols1 + j]  += firMatrix[i * cols1 + k] * secMatrix[k * cols2 + j];
//            }
//        }
//    }
//    return result;
//}
//ResultMatrix *SUB(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols) {
//    ResultMatrix *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result->data[i * cols + j] = firMatrix[i * cols + j] - secMatrix[i * cols + j];
//        }
//    }
//    return result;
//}
//ResultMatrix *ADD(cmpdouble *firMatrix,cmpdouble *secMatrix, int rows, int cols) {
//    ResultMatrix *result = createMatrix(rows, cols);
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            result->data[i * cols + j] = firMatrix[i * cols + j] + secMatrix[i * cols + j];
//        }
//    }
//    return result;
//}
//
//ResultMatrix *createMatrix(int rows, int cols) {
//    ResultMatrix* result = (ResultMatrix*)malloc(sizeof(ResultMatrix) + (rows * cols - 1) * sizeof(cmpdouble));
//    if (result == NULL) {
//        perror("Failed to allocate memory for ResultMatrix");
//        exit(1);
//    }
//    result->rows = rows;
//    result->cols = cols;
//    return result;
//}
//
