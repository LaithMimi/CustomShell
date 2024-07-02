#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <complex.h>
#include <semaphore.h>

#define MAX_OP_LEN 16
#define MAX_SHM 2048

typedef struct {
    int rows;
    int cols;
    complex double data[1];
} Matrix;

typedef struct {
    Matrix matrix;
    char operation[MAX_OP_LEN];
} MatrixData;

Matrix *inputMatrix(char input[128],int *rows, int *cols);
Matrix *createMatrix(int rows, int cols);
void freeMatrix(Matrix *matrix);
void saveToShm(Matrix *matrix, const char *operation, void *shm_addr, int *mat_counter, sem_t *sem);

int main() {
    int shm_id;
    void* shm_addr;
    int *mat_counter;
    sem_t *sem;

    char input[128], op[MAX_OP_LEN];
    Matrix *firMatrix, *secMatrix;
    int rows1, cols1, rows2, cols2;


    key_t key = ftok("/tmp", 'r');

    // Allocate a shared memory segment
    shm_id = shmget(key, MAX_SHM, IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id < 0) {
        perror("Exq2a: shmget:");
        exit(1);
    }

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1) {
        perror("Exq2a: shmat:");
        exit(1);
    }

    // Initialize the matrix index
    mat_counter = (int*)shm_addr;
    *mat_counter = 0;

    //initialize the semaphore in shm
    sem = (sem_t *)(shm_addr + sizeof(int));
    if (sem_init(sem, 1, 1) == -1) { //1: sem is shared between processes
        perror("Exq2a: sem_init:");
        exit(1);
    }

    void *matrix_storage_start = shm_addr + sizeof(int)+sizeof(sem_t);

    //do the writing job
    while (1) {
        printf("Enter matrix (rows,cols:val1,val2,...,valN) or 'END' to exit:\n");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, "END") == 0) break;

        firMatrix = inputMatrix(input,&rows1,&cols1);
        if (!firMatrix) {
            printf("Invalid format. Please use 'rows,cols:val1,val2,...,valN'.\n");
            continue;
        }


        printf("Enter 2nd matrix (rows,cols:val1,val2,...,valN) or 'END' to exit:\n");
        if (fgets(input,sizeof(input),stdin)==NULL) {
            freeMatrix(firMatrix);
            break;
        }
        input[strcspn(input,"\n")]='\n';
        if(strcmp(input,"END")==0) {
            freeMatrix(firMatrix);
            break;
        }
        if(strcmp(input,"TRANSPOSE\n")==0 || strcmp(input,"NOT\n")==0) {
            printf("Here 1\n");
            saveToShm(firMatrix,input,matrix_storage_start,mat_counter,sem);
            freeMatrix(firMatrix);
        }
        else {
            secMatrix=inputMatrix(input,&rows2,&cols2);
            if (!secMatrix) {
                printf("Error allocating memory for second matrix.\n");
                freeMatrix(firMatrix);
                continue;
            }
            printf("Enter the operation for the matrix (e.g., 'ADD', 'SUBTRACT'):\n");
            scanf("%s", op);
            getchar();

            saveToShm(firMatrix,op, matrix_storage_start, mat_counter, sem);
            saveToShm(secMatrix,op, matrix_storage_start, mat_counter, sem);

            freeMatrix(firMatrix);
            freeMatrix(secMatrix);
        }
    }

    sem_destroy(sem);

    //detach and remove shared memory
    shmdt(shm_addr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}

Matrix *inputMatrix(char input[128],int *rows, int *cols) {
    if(sscanf(input, "(%d,%d:",rows,cols)!=2) {
        perror("Invalid matrix format");
        return NULL;
    }

    Matrix *matrix = createMatrix(*rows,*cols);

    char *token = strtok(input, ":");
    if (token == NULL) {
        freeMatrix(matrix);
        perror("Invalid matrix data format");
        return NULL;
    }

    token = strtok(NULL, ",");

    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *cols; j++) {
            if (token == NULL) {
                freeMatrix(matrix);
                perror("Insufficient matrix data");
                return NULL;
            }

            double real = 0, imag = 0;
            char *i_char = strchr(token, 'i');

            if (i_char) {
                // Parse imaginary part
                if (token == i_char) {
                    imag = 1.0;
                } else if (token + 1 == i_char && *token == '-') {
                    imag = -1.0;
                } else {
                    sscanf(token, "%lf", &imag);
                }
            } else {
                // Parse real part
                sscanf(token, "%lf", &real);
            }

            matrix->data[i * (*cols) + j] = real + imag * I;
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

void saveToShm(Matrix *matrix, const char *operation, void *shm_addr, int *mat_counter, sem_t *sem) {
    // Calculate the size required to store this matrix operation
    size_t matrix_size = sizeof(Matrix) + matrix->rows * matrix->cols * sizeof(complex double) - sizeof(complex double);
    size_t total_size = sizeof(MatrixData) + (matrix->rows * matrix->cols - 1) * sizeof(complex double);

    // Calculate the offset for storing the new matrix operation
    void *target_addr = shm_addr + (*mat_counter * total_size);

    //lock the semaphore before accessing shared memory
    sem_wait(sem);

    // Copy the matrix data to the shared memory
    MatrixData *matrix_op = (MatrixData *)target_addr;

    memcpy(&(matrix_op->matrix), matrix, matrix_size);

    strncpy(matrix_op->operation, operation, MAX_OP_LEN - 1);

    matrix_op->operation[MAX_OP_LEN - 1] = '\0';

    // Increment the matrix counter
    (*mat_counter)++;

    //unlock the semaphore after modifying shared memory
    sem_post(sem);
}