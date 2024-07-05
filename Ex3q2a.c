#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <ctype.h>

#define SHM_SIZE 2048
#define MAX_OP_LEN 16
#define MAX_SIZE 128

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

int is_complex(const char* str) {
    return strstr(str, "i") != NULL;
}

int is_double(const char* str) {
    return strchr(str, '.') != NULL;
}

void parseMatrix(Matrix *mat, const char *buffer) {
    if (sscanf(buffer, "(%d,%d:", &mat->rows, &mat->cols) != 2) {
        fprintf(stderr, "Error: Invalid matrix format\n");
        exit(1);
    }
    int n = mat->rows * mat->cols;
    const char *start = strchr(buffer, ':');
    if (!start) {
        fprintf(stderr, "Error: Invalid matrix format\n");
        exit(1);
    }
    start++;

    int is_any_complex = 0;
    int is_any_double = 0;

    char *temp = malloc(strlen(buffer) + 1);
    if (!temp) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    strcpy(temp, start);

    char *token = strtok(temp, ",)");
    int count = 0;

    while (token && count < n) {
        if (is_complex(token)) {
            is_any_complex = 1;
        } else if (is_double(token)) {
            is_any_double = 1;
        }
        token = strtok(NULL, ",)");
        count++;
    }

    if (count != n) {
        fprintf(stderr, "Error: Mismatch in matrix dimensions and provided values\n");
        free(temp);
        exit(1);
    }

    if (is_any_complex) {
        mat->type = 2;
        mat->complex_data = malloc(n * sizeof(Complex));
        if (!mat->complex_data) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            free(temp);
            exit(1);
        }
        sscanf(start, "%lf+%lfi", &mat->complex_data[0].real, &mat->complex_data[0].imag);
        for (int i = 1; i < n; i++) {
            token = strtok(NULL, ",)");
            if (!token || sscanf(token, "%lf+%lfi", &mat->complex_data[i].real, &mat->complex_data[i].imag) != 2) {
                fprintf(stderr, "Error: Invalid complex number format\n");
                free(temp);
                free(mat->complex_data);
                exit(1);
            }
        }
    } else if (is_any_double) {
        mat->type = 1;
        mat->double_data = malloc(n * sizeof(double));
        if (!mat->double_data) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            free(temp);
            exit(1);
        }
        sscanf(start, "%lf", &mat->double_data[0]);
        for (int i = 1; i < n; i++) {
            token = strtok(NULL, ",)");
            if (!token || sscanf(token, "%lf", &mat->double_data[i]) != 1) {
                fprintf(stderr, "Error: Invalid double format\n");
                free(temp);
                free(mat->double_data);
                exit(1);
            }
        }
    } else {
        mat->type = 0;
        mat->int_data = malloc(n * sizeof(int));
        if (!mat->int_data) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            free(temp);
            exit(1);
        }
        sscanf(start, "%d", &mat->int_data[0]);
        for (int i = 1; i < n; i++) {
            token = strtok(NULL, ",)");
            if (!token || sscanf(token, "%d", &mat->int_data[i]) != 1) {
                fprintf(stderr, "Error: Invalid integer format\n");
                free(temp);
                free(mat->int_data);
                exit(1);
            }
        }
    }

    free(temp);
}

void writeToShm(void* shm_addr, Matrix *matrix, char *operation) {
    size_t offset = 0;
    size_t total_size = 3 * sizeof(int);  // For rows, cols, and type

    int n = matrix->rows * matrix->cols;
    if (matrix->type == 0) {
        total_size += n * sizeof(int);
    } else if (matrix->type == 1) {
        total_size += n * sizeof(double);
    } else if (matrix->type == 2) {
        total_size += n * sizeof(Complex);
    }
    total_size += strlen(operation) + 1;  // +1 for null terminator

    if (total_size > SHM_SIZE) {
        fprintf(stderr, "Error: Data too large for shared memory\n");
        exit(1);
    }

    memcpy(shm_addr, &matrix->rows, sizeof(int));
    offset += sizeof(int);
    memcpy(shm_addr + offset, &matrix->cols, sizeof(int));
    offset += sizeof(int);
    memcpy(shm_addr + offset, &matrix->type, sizeof(int));
    offset += sizeof(int);

    if (matrix->type == 0) {
        memcpy(shm_addr + offset, matrix->int_data, n * sizeof(int));
        offset += n * sizeof(int);
    } else if (matrix->type == 1) {
        memcpy(shm_addr + offset, matrix->double_data, n * sizeof(double));
        offset += n * sizeof(double);
    } else if (matrix->type == 2) {
        memcpy(shm_addr + offset, matrix->complex_data, n * sizeof(Complex));
        offset += n * sizeof(Complex);
    }
    strcpy(shm_addr + offset, operation);
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

int main() {
    int shm_id;
    void* shm_addr;
    Matrix matrix1, matrix2;
    char operation[MAX_OP_LEN];
    char input[MAX_SIZE];
    sem_t* sem;

    key_t key = ftok("/tmp", 'm');
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0600);
    if (shm_id == -1) {
        perror("writer: shmget");
        exit(1);
    }

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void*)-1) {
        perror("writer: shmat");
        exit(1);
    }

    sem = sem_open("/shm_sem", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("writer: sem_open");
        exit(1);
    }


    while(1) {
        if (sem_wait(sem) == -1) {
            perror("writer: sem_wait");
            exit(1);
        }
        printf("Enter matrix 1 in format (rows,cols:val1,val2,...,valN): ");
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;
        input[strcspn(input, "\n")] = '\0'; //attention here
        if (strcmp(input, "END") == 0){
            if (sem_post(sem) == -1) {
                perror("writer: sem_post");
                exit(1);
            }
            break;
        }
        parseMatrix(&matrix1, input);


        printf("Enter matrix 2 in format (rows,cols:val1,val2,...,valN): ");
        if (fgets(input, sizeof(input), stdin) == NULL){
            freeMatrix(&matrix1);
            break;
        }
        input[strcspn(input,"\n")]='\n';
        if(strcmp(input,"END")==0) {
            freeMatrix(&matrix1);
            if (sem_post(sem) == -1) {
                perror("writer: sem_post");
                exit(1);
            }
            break;
        }
        if(strcmp(input,"TRANSPOSE")==0 || strcmp(input,"NOT")==0) {
            writeToShm(shm_addr, &matrix1, input);
            freeMatrix(&matrix1);
        }else {
            parseMatrix(&matrix2, input);
            printf("Enter the operation for the matrix (e.g., 'ADD', 'SUB'):\n");
            scanf(" %s", operation);
            getchar();


            writeToShm(shm_addr, &matrix1, operation);
            writeToShm(shm_addr, &matrix2, operation);
            freeMatrix(&matrix1);
            freeMatrix(&matrix2);
        }
        if (sem_post(sem) == -1) {
            perror("writer: sem_post");
            exit(1);
        }
    }
    if (sem_close(sem) == -1) {
        perror("writer: sem_close");
    }

    if (shmdt(shm_addr) == -1) {
        perror("writer: shmdt");
    }

    return 0;
}
