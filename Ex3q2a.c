#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>

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
    printf("DEBUG: Checking if '%s' is complex\n", str);
    return strstr(str, "i") != NULL;
}

int is_double(const char* str) {
    printf("DEBUG: Checking if '%s' is double\n", str);
    return strchr(str, '.') != NULL;
}

void parseMatrix(Matrix *mat, const char *buffer) {
    printf("DEBUG: Parsing matrix: %s\n", buffer);
    if (sscanf(buffer, "(%d,%d:", &mat->rows, &mat->cols) != 2) {
        fprintf(stderr, "Error: Invalid matrix format\n");
        exit(1);
    }
    printf("DEBUG: Matrix dimensions: %d x %d\n", mat->rows, mat->cols);

    int n = mat->rows * mat->cols;
    const char *start = strchr(buffer, ':');
    if (!start) {
        fprintf(stderr, "Error: Invalid matrix format\n");
        exit(1);
    }
    start++;

    printf("DEBUG: Matrix values: %s\n", start);

    char *temp = strdup(start);
    if (!temp) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }

    char *token = strtok(temp, ",)");
    int count = 0;
    int is_any_complex = 0;
    int is_any_double = 0;

    while (token && count < n) {
        printf("DEBUG: Parsing token: %s\n", token);
        if (is_complex(token)) {
            is_any_complex = 1;
        } else if (is_double(token)) {
            is_any_double = 1;
        }
        token = strtok(NULL, ",)");
        count++;
    }

    printf("DEBUG: Parsed %d values\n", count);

    if (count != n) {
        fprintf(stderr, "Error: Mismatch in matrix dimensions and provided values\n");
        free(temp);
        exit(1);
    }

    free(temp);
    temp = strdup(start);  // Reset temp for actual parsing

    if (is_any_complex) {
        printf("DEBUG: Parsing as complex matrix\n");
        mat->type = 2;
        mat->complex_data = malloc(n * sizeof(Complex));
        if (!mat->complex_data) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            free(temp);
            exit(1);
        }
        for (int i = 0; i < n; i++) {
            token = (i == 0) ? strtok(temp, ",)") : strtok(NULL, ",)");
            if (!token || sscanf(token, "%lf+%lfi", &mat->complex_data[i].real, &mat->complex_data[i].imag) != 2) {
                fprintf(stderr, "Error: Invalid complex number format at position %d\n", i);
                free(temp);
                free(mat->complex_data);
                exit(1);
            }
            printf("DEBUG: Parsed complex value: %f + %fi\n", mat->complex_data[i].real, mat->complex_data[i].imag);
        }
    } else if (is_any_double) {
        printf("DEBUG: Parsing as double matrix\n");
        mat->type = 1;
        mat->double_data = malloc(n * sizeof(double));
        if (!mat->double_data) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            free(temp);
            exit(1);
        }
        for (int i = 0; i < n; i++) {
            token = (i == 0) ? strtok(temp, ",)") : strtok(NULL, ",)");
            if (!token || sscanf(token, "%lf", &mat->double_data[i]) != 1) {
                fprintf(stderr, "Error: Invalid double format at position %d\n", i);
                free(temp);
                free(mat->double_data);
                exit(1);
            }
            printf("DEBUG: Parsed double value: %f\n", mat->double_data[i]);
        }
    } else {
        printf("DEBUG: Parsing as integer matrix\n");
        mat->type = 0;
        mat->int_data = malloc(n * sizeof(int));
        if (!mat->int_data) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            free(temp);
            exit(1);
        }
        for (int i = 0; i < n; i++) {
            token = (i == 0) ? strtok(temp, ",)") : strtok(NULL, ",)");
            if (!token || sscanf(token, "%d", &mat->int_data[i]) != 1) {
                fprintf(stderr, "Error: Invalid integer format at position %d\n", i);
                free(temp);
                free(mat->int_data);
                exit(1);
            }
            printf("DEBUG: Parsed integer value: %d\n", mat->int_data[i]);
        }
    }

    free(temp);
    printf("DEBUG: Matrix parsing complete\n");
}

void writeToShm(void* shm_addr, Matrix *matrix, char *operation) {
    printf("DEBUG: Writing to shared memory\n");
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

    printf("DEBUG: Total size to write: %zu\n", total_size);

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

    printf("DEBUG: Writing matrix data\n");
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
    printf("DEBUG: Wrote operation: %s\n", operation);

    int flag = 1;
    memcpy(shm_addr + SHM_SIZE - sizeof(int), &flag, sizeof(int));

    printf("DEBUG: Set shared memory flag to 1\n");
}

void freeMatrix(Matrix *matrix) {
    printf("DEBUG: Freeing matrix memory\n");
    if (matrix->type == 0) {
        free(matrix->int_data);
    } else if (matrix->type == 1) {
        free(matrix->double_data);
    } else if (matrix->type == 2) {
        free(matrix->complex_data);
    }
}

int main() {
    printf("DEBUG: Starting writer program\n");
    int shm_id;
    void* shm_addr;
    Matrix matrix1, matrix2;
    char operation[MAX_OP_LEN];
    char input[MAX_SIZE];
    sem_t* sem;
    int *mat_counter;


    key_t key = ftok("/tmp", 'l');
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0600);
    if (shm_id == -1) {
        perror("writer: shmget");
        exit(1);
    }
    printf("DEBUG: Created shared memory segment\n");

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void*)-1) {
        perror("writer: shmat");
        exit(1);
    }
    printf("DEBUG: Attached to shared memory segment\n");

    mat_counter = (int*) shm_addr;
    *mat_counter = 0;

    sem = sem_open("/shm_sem", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("writer: sem_open");
        exit(1);
    }
    printf("DEBUG: Opened semaphore\n");

    while(1) {
//        printf("DEBUG: Waiting for semaphore\n");
//        if (sem_wait(sem) == -1) {
//            perror("writer: sem_wait");
//            exit(1);
//        }
//        printf("DEBUG: Acquired semaphore\n");

        printf("Enter matrix 1 in format (rows,cols:val1,val2,...,valN): ");
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;
        input[strcspn(input, "\n")] = '\0';
        printf("DEBUG: Received input: %s\n", input);

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
        input[strcspn(input,"\n")]='\0';
        printf("DEBUG: Received input: %s\n", input);

        if(strcmp(input,"END")==0) {
            freeMatrix(&matrix1);
            if (sem_post(sem) == -1) {
                perror("writer: sem_post");
                exit(1);
            }
            break;
        }
        if(strcmp(input,"TRANSPOSE")==0 || strcmp(input,"NOT")==0) {
            printf("DEBUG: Single matrix operation: %s\n", input);
            writeToShm(shm_addr, &matrix1, input);
            freeMatrix(&matrix1);
            continue;
        } else {
            parseMatrix(&matrix2, input);
            printf("Enter the operation for the matrix (e.g., 'ADD', 'SUB'):\n");
            scanf("%s", operation);
            getchar(); // Consume newline
            printf("DEBUG: Operation: %s\n", operation);

            writeToShm(shm_addr, &matrix1, operation);
            writeToShm(shm_addr + SHM_SIZE/2, &matrix2, operation);

        }
        printf("DEBUG: Releasing semaphore\n");
        if (sem_post(sem) == -1) {
            perror("writer: sem_post");
            exit(1);
        }
        freeMatrix(&matrix1);
        freeMatrix(&matrix2);
    }
    printf("DEBUG: Exiting main loop\n");

    if (sem_close(sem) == -1) {
        perror("writer: sem_close");
    }
    printf("DEBUG: Closed semaphore\n");

    if (shmdt(shm_addr) == -1) {
        perror("writer: shmdt");
    }
    printf("DEBUG: Detached from shared memory\n");
    shmctl(shm_id, IPC_RMID, NULL);
    printf("DEBUG: Writer program complete\n");
    return 0;
}
