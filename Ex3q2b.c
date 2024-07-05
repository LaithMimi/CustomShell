#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>


#define SHM_SIZE 2048
#define MAX_OP_LEN 16

typedef struct {
    double real;
    double imag;
} Complex;

typedef struct {
    int rows;
    int cols;
    int type;  // 0 for int, 1 for double, 2 for complex
    union {
        int* int_data;
        double* double_data;
        Complex* complex_data;
    };
} Matrix;

void readFromShm(void* shm_addr, Matrix *matrix, char *operation) {
    size_t offset = 0;

    memcpy(&matrix->rows, shm_addr, sizeof(int));
    offset += sizeof(int);
    memcpy(&matrix->cols, shm_addr + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&matrix->type, shm_addr + offset, sizeof(int));
    offset += sizeof(int);

    int n = matrix->rows * matrix->cols;
    if (matrix->type == 0) {
        matrix->int_data = malloc(n * sizeof(int));
        memcpy(matrix->int_data, shm_addr + offset, n * sizeof(int));
        offset += n * sizeof(int);
    } else if (matrix->type == 1) {
        matrix->double_data = malloc(n * sizeof(double));
        memcpy(matrix->double_data, shm_addr + offset, n * sizeof(double));
        offset += n * sizeof(double);
    } else if (matrix->type == 2) {
        matrix->complex_data = malloc(n * sizeof(Complex));
        memcpy(matrix->complex_data, shm_addr + offset, n * sizeof(Complex));
        offset += n * sizeof(Complex);
    }
    memcpy(operation, shm_addr + offset, sizeof(char));
}

void printMatrix(Matrix *matrix) {
    printf("(%d,%d:", matrix->rows, matrix->cols);
    int n = matrix->rows * matrix->cols;
    for (int i = 0; i < n; i++) {
        if (matrix->type == 0) {
            printf("%d", matrix->int_data[i]);
        } else if (matrix->type == 1) {
            printf("%f", matrix->double_data[i]);
        } else if (matrix->type == 2) {
            printf("%d+ddi",(int) matrix->complex_data[i].real,(int) matrix->complex_data[i].imag);
        }
        if (i < n - 1) printf(",");
    }
    printf(")\n");
}

void freeMatrix(Matrix *matrix) {
    if (matrix->type == 0) {
        free(matrix->int_data);
    } else if (matrix->type == 1) {
        free(matrix->double_data);
    } else if (matrix->type == 2) {
        free(matrix->complex_data);
    }
}

void ADD(Matrix *result, Matrix *m1, Matrix *m2) {
    result->rows = m1->rows;
    result->cols = m1->cols;
    result->type = m1->type > m2->type ? m1->type : m2->type;
    int n = result->rows * result->cols;

    if (result->type == 0) {
        result->int_data = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            result->int_data[i] = m1->int_data[i] + m2->int_data[i];
        }
    } else if (result->type == 1) {
        result->double_data = malloc(n * sizeof(double));
        for (int i = 0; i < n; i++) {
            result->double_data[i] = (m1->type == 1 ? m1->double_data[i] : (double)m1->int_data[i]) +
                                     (m2->type == 1 ? m2->double_data[i] : (double)m2->int_data[i]);
        }
    } else if (result->type == 2) {
        result->complex_data = malloc(n * sizeof(Complex));
        for (int i = 0; i < n; i++) {
            if (m1->type == 2 && m2->type == 2) {
                result->complex_data[i].real = m1->complex_data[i].real + m2->complex_data[i].real;
                result->complex_data[i].imag = m1->complex_data[i].imag + m2->complex_data[i].imag;
            } else if (m1->type == 2) {
                result->complex_data[i].real = m1->complex_data[i].real + (m2->type == 1 ? m2->double_data[i] : (double)m2->int_data[i]);
                result->complex_data[i].imag = m1->complex_data[i].imag;
            } else {
                result->complex_data[i].real = (m1->type == 1 ? m1->double_data[i] : (double)m1->int_data[i]) + m2->complex_data[i].real;
                result->complex_data[i].imag = m2->complex_data[i].imag;
            }
        }
    }
}

void SUB(Matrix *result, Matrix *m1, Matrix *m2) {
    result->rows = m1->rows;
    result->cols = m1->cols;
    result->type = m1->type > m2->type ? m1->type : m2->type;
    int n = result->rows * result->cols;

    if (result->type == 0) {
        result->int_data = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            result->int_data[i] = m1->int_data[i] - m2->int_data[i];
        }
    } else if (result->type == 1) {
        result->double_data = malloc(n * sizeof(double));
        for (int i = 0; i < n; i++) {
            result->double_data[i] = (m1->type == 1 ? m1->double_data[i] : (double)m1->int_data[i]) -
                                     (m2->type == 1 ? m2->double_data[i] : (double)m2->int_data[i]);
        }
    } else if (result->type == 2) {
        result->complex_data = malloc(n * sizeof(Complex));
        for (int i = 0; i < n; i++) {
            if (m1->type == 2 && m2->type == 2) {
                result->complex_data[i].real = m1->complex_data[i].real - m2->complex_data[i].real;
                result->complex_data[i].imag = m1->complex_data[i].imag - m2->complex_data[i].imag;
            } else if (m1->type == 2) {
                result->complex_data[i].real = m1->complex_data[i].real - (m2->type == 1 ? m2->double_data[i] : (double)m2->int_data[i]);
                result->complex_data[i].imag = m1->complex_data[i].imag;
            } else {
                result->complex_data[i].real = (m1->type == 1 ? m1->double_data[i] : (double)m1->int_data[i]) - m2->complex_data[i].real;
                result->complex_data[i].imag = -m2->complex_data[i].imag;
            }
        }
    }
}

void TRANSPOSE(Matrix *result, Matrix *m) {
    result->rows = m->cols;
    result->cols = m->rows;
    result->type = m->type;
    int n = result->rows * result->cols;

    if (result->type == 0) {
        result->int_data = malloc(n * sizeof(int));
        for (int i = 0; i < m->rows; i++) {
            for (int j = 0; j < m->cols; j++) {
                result->int_data[j * m->rows + i] = m->int_data[i * m->cols + j];
            }
        }
    } else if (result->type == 1) {
        result->double_data = malloc(n * sizeof(double));
        for (int i = 0; i < m->rows; i++) {
            for (int j = 0; j < m->cols; j++) {
                result->double_data[j * m->rows + i] = m->double_data[i * m->cols + j];
            }
        }
    } else if (result->type == 2) {
        result->complex_data = malloc(n * sizeof(Complex));
        for (int i = 0; i < m->rows; i++) {
            for (int j = 0; j < m->cols; j++) {
                result->complex_data[j * m->rows + i] = m->complex_data[i * m->cols + j];
            }
        }
    }
}

void MUL(Matrix *result, Matrix *m1, Matrix *m2) {
    if (m1->cols != m2->rows) {
        printf("Error: Incompatible matrix dimensions for multiplication\n");
        return;
    }

    result->rows = m1->rows;
    result->cols = m2->cols;
    result->type = m1->type > m2->type ? m1->type : m2->type;
    int n = result->rows * result->cols;

    if (result->type == 0) {
        result->int_data = calloc(n, sizeof(int));
        for (int i = 0; i < m1->rows; i++) {
            for (int j = 0; j < m2->cols; j++) {
                for (int k = 0; k < m1->cols; k++) {
                    result->int_data[i*m2->cols + j] += m1->int_data[i*m1->cols + k] * m2->int_data[k*m2->cols + j];
                }
            }
        }
    } else if (result->type == 1) {
        result->double_data = calloc(n, sizeof(double));
        for (int i = 0; i < m1->rows; i++) {
            for (int j = 0; j < m2->cols; j++) {
                for (int k = 0; k < m1->cols; k++) {
                    double m1_val = m1->type == 1 ? m1->double_data[i*m1->cols + k] : (double)m1->int_data[i*m1->cols + k];
                    double m2_val = m2->type == 1 ? m2->double_data[k*m2->cols + j] : (double)m2->int_data[k*m2->cols + j];
                    result->double_data[i*m2->cols + j] += m1_val * m2_val;
                }
            }
        }
    } else if (result->type == 2) {
        result->complex_data = calloc(n, sizeof(Complex));
        for (int i = 0; i < m1->rows; i++) {
            for (int j = 0; j < m2->cols; j++) {
                for (int k = 0; k < m1->cols; k++) {
                    Complex m1_val, m2_val;
                    if (m1->type == 2) {
                        m1_val = m1->complex_data[i*m1->cols + k];
                    } else if (m1->type == 1) {
                        m1_val.real = m1->double_data[i*m1->cols + k];
                        m1_val.imag = 0;
                    } else {
                        m1_val.real = m1->int_data[i*m1->cols + k];
                        m1_val.imag = 0;
                    }
                    if (m2->type == 2) {
                        m2_val = m2->complex_data[k*m2->cols + j];
                    } else if (m2->type == 1) {
                        m2_val.real = m2->double_data[k*m2->cols + j];
                        m2_val.imag = 0;
                    } else {
                        m2_val.real = m2->int_data[k*m2->cols + j];
                        m2_val.imag = 0;
                    }
                    result->complex_data[i*m2->cols + j].real += m1_val.real * m2_val.real - m1_val.imag * m2_val.imag;
                    result->complex_data[i*m2->cols + j].imag += m1_val.real * m2_val.imag + m1_val.imag * m2_val.real;
                }
            }
        }
    }
}

void NOT(Matrix *result, Matrix *m) {
    result->rows = m->rows;
    result->cols = m->cols;
    result->type = m->type;
    int n = result->rows * result->cols;

    if (result->type == 0) {
        result->int_data = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            result->int_data[i] = ~m->int_data[i];
        }
    } else if (result->type == 1) {
        result->double_data = malloc(n * sizeof(double));
        for (int i = 0; i < n; i++) {
            result->double_data[i] = -m->double_data[i];
        }
    } else if (result->type == 2) {
        result->complex_data = malloc(n * sizeof(Complex));
        for (int i = 0; i < n; i++) {
            result->complex_data[i].real = -m->complex_data[i].real;
            result->complex_data[i].imag = -m->complex_data[i].imag;
        }
    }
}

void AND(Matrix *result, Matrix *m1, Matrix *m2) {
    if (m1->type != 0 || m2->type != 0) {
        printf("Error: AND operation is only for integer (binary) matrices\n");
        return;
    }
    result->rows = m1->rows;
    result->cols = m1->cols;
    result->type = 0;
    int n = result->rows * result->cols;
    result->int_data = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        result->int_data[i] = m1->int_data[i] & m2->int_data[i];
    }
}

void OR(Matrix *result, Matrix *m1, Matrix *m2) {
    if (m1->type != 0 || m2->type != 0) {
        printf("Error: OR operation is only for integer (binary) matrices\n");
        return;
    }
    result->rows = m1->rows;
    result->cols = m1->cols;
    result->type = 0;
    int n = result->rows * result->cols;
    result->int_data = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        result->int_data[i] = m1->int_data[i] | m2->int_data[i];
    }
}

void notMatrix(Matrix *result, Matrix *m) {
    result->rows = m->rows;
    result->cols = m->cols;
    result->type = m->type;
    int n = result->rows * result->cols;

    if (result->type == 0) {
        result->int_data = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            result->int_data[i] = ~m->int_data[i];
        }
    } else {
        printf("Error: NOT operation is only for integer (binary) matrices\n");
        return;
    }
}
int main() {
    int shm_id;
    void* shm_addr;
    Matrix matrix1, matrix2, result;
    char operation[MAX_OP_LEN];
    sem_t* sem;

    key_t key = ftok("/tmp", 'm');
    shm_id = shmget(key, SHM_SIZE, 0600);
    if (shm_id == -1) {
        perror("reader: shmget");
        exit(1);
    }

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void*)-1) {
        perror("reader: shmat");
        exit(1);
    }

    sem = sem_open("/shm_sem", 0);
    if (sem == SEM_FAILED) {
        perror("reader: sem_open");
        exit(1);
    }

    while(1) {
        if (sem_wait(sem) == -1) {
            perror("reader: sem_wait");
            exit(1);
        }

        readFromShm(shm_addr, &matrix1, operation);

        if (strcmp(operation, "END") == 0) {
            if (sem_post(sem) == -1) {
                perror("reader: sem_post");
                exit(1);
            }
            break;
        }

        if (strcmp(operation, "TRANSPOSE") == 0) {
            TRANSPOSE(&result, &matrix1);
        } else if (strcmp(operation, "NOT") == 0) {
            NOT(&result, &matrix1);
        } else {
            readFromShm(shm_addr + SHM_SIZE/2, &matrix2, operation);

            if (strcmp(operation, "ADD") == 0) {
                ADD(&result, &matrix1, &matrix2);
            }
            else if (strcmp(operation, "SUB") == 0) {
                SUB(&result, &matrix1, &matrix2);
            }
            else if (strcmp(operation, "MUL") == 0) {
                MUL(&result, &matrix1, &matrix2);
            }
            else if (strcmp(operation, "AND") == 0) {
                AND(&result, &matrix1, &matrix2);
            }
            else if (strcmp(operation, "OR") == 0) {
                OR(&result, &matrix1, &matrix2);
            }else {
                printf("Unknown operation: %s\n", operation);
            }

            freeMatrix(&matrix2);
        }

        printf("Operation: %s\n", operation);
        printf("Result: ");
        printMatrix(&result);

        freeMatrix(&matrix1);
        freeMatrix(&result);

        if (sem_post(sem) == -1) {
            perror("reader: sem_post");
            exit(1);
        }
    }

    if (sem_close(sem) == -1) {
        perror("reader: sem_close");
    }

    if (shmdt(shm_addr) == -1) {
        perror("reader: shmdt");
    }

    return 0;
}