#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <complex.h>
#include <semaphore.h>

#define MAX_OP_LEN 16

typedef struct {
    int rows;
    int cols;
    complex double data[1]; // Flexible array member
} Matrix;
typedef struct {
    Matrix matrix;
    char operation[MAX_OP_LEN];
} MatrixData;

Matrix *inputMatrix(char input[128]);
Matrix *createMatrix(int rows, int cols);
void freeMatrix(Matrix *matrix);
void saveMatrixToSharedMemory(Matrix *matrix,const char *operation, void *shm_addr, int *mat_counter, sem_t *sem);


int main() {
    int shm_id;
    void* shm_addr;
    int *mat_counter;
    sem_t *sem;

    char input[128], op[MAX_OP_LEN];
    Matrix *matrix;

    key_t key = ftok("/tmp", 'x');

    // Allocate a shared memory segment
    shm_id = shmget(key, 2048, IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id < 0) {
        perror("main: shmget:");
        exit(1);
    }

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1) {
        perror("main: shmat:");
        exit(1);
    }

    // Initialize the matrix index
    mat_counter = (int*)shm_addr;
    *mat_counter = 0;

    // Initialize the semaphore in shared memory
    sem = (sem_t *)(shm_addr + sizeof(int));
    if (sem_init(sem, 1, 1) == -1) { // 1 means the semaphore is shared between processes
        perror("main: sem_init:");
        exit(1);
    }

    void *matrix_storage_start = shm_addr + sizeof(int);

    // Do the writing job
    while (1) {
        printf("Enter matrix (rows,cols:val1,val2,...,valN) or 'END' to exit:\n");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        if (strncmp(input, "END", 3) == 0) {
            break;
        }

        matrix = inputMatrix(input);
        if (matrix == NULL) {
            printf("Invalid format. Please use 'rows,cols:val1,val2,...,valN'.\n");
            continue;
        }
        printf("Enter the operation for the matrix (e.g., 'ADD', 'SUBTRACT'):\n");
        if (fgets(op, sizeof(op), stdin) == NULL) {
            freeMatrix(matrix);
            break;
        }
        // Remove newline character from operation input
        op[strcspn(op, "\n")] = '\0';

        saveMatrixToSharedMemory(matrix,op, matrix_storage_start, mat_counter,sem);

        freeMatrix(matrix);
    }

    sem_destroy(sem);

    // Detach and remove shared memory
    shmdt(shm_addr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}

Matrix *inputMatrix(char input[128]) {
    int rows, cols;
    sscanf(input, "(%d,%d:", rows, cols);

    Matrix *matrix = createMatrix(rows, cols);
    char *token = strtok(input, ":");
    token = strtok(NULL, ",");

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {

            if (token == NULL) {
                freeMatrix(matrix);
                return NULL;
            }
            double real = 0, imag = 0;
            char *plus = strchr(token, '+');
            char *minus = strchr(token, '-');
            char *i_char = strchr(token, 'i');

            if ((plus || minus) && i_char) {
                // Complex number
                sscanf(token, "%lf%lf", &real, &imag);
            } else if (i_char) {
                // Imaginary number
                if (token == i_char)
                    imag = 1.0;
                else if (token + 1 == i_char) {
                    imag = -1.0;
                } else {
                    sscanf(token, "%lf", &imag);
                }
            } else {
                // Real number (integer or double)
                real = strtod(token, &token);
            }
            matrix->data[i * cols + j] = real + imag * I;
            token = strtok(NULL, ",)");
        }
    }
    return matrix;
}

Matrix *createMatrix(int rows, int cols) {
    size_t struct_size = sizeof(Matrix) + rows * cols * sizeof(complex double) - sizeof(complex double);
    Matrix *matrix = (Matrix *) malloc(struct_size);
    if (matrix == NULL) {
        perror("createMatrix: malloc:");
        exit(1);
    }
    matrix->rows = rows;
    matrix->cols = cols;
    return matrix;
}

void freeMatrix(Matrix *matrix) {
    free(matrix);
}

void saveMatrixToSharedMemory(Matrix *matrix,const char *operation, void *shm_addr, int *mat_counter, sem_t *sem) {
    // Calculate the size required to store this matrix operation
    size_t matrix_size = sizeof(Matrix) + matrix->rows * matrix->cols * sizeof(complex double) - sizeof(complex double);
    size_t total_size = sizeof(MatrixData) + (matrix->rows * matrix->cols - 1) * sizeof(complex double);

    // Calculate the offset for storing the new matrix operation
    void *target_addr = shm_addr + (*mat_counter * total_size);

    // Lock the semaphore before accessing shared memory
    sem_wait(sem);

    // Copy the matrix data to the shared memory
    MatrixData *matrix_op = (MatrixData *)target_addr;

    memcpy(&(matrix_op->matrix), matrix, matrix_size);

    strncpy(matrix_op->operation, operation, MAX_OP_LEN - 1);

    matrix_op->operation[MAX_OP_LEN - 1] = '\0';

    // Increment the matrix counter
    (*mat_counter)++;

    // Unlock the semaphore after modifying shared memory
    sem_post(sem);
}