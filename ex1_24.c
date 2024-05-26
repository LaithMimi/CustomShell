#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_CMD_LEN 1024  // Maximum length of a command
#define MAX_ARGS 4        // Maximum number of arguments
#define MAX_ALIASES 100   // Maximum number of aliases

typedef struct {
    char *name;
    char *cmdLine;
} Alias;

Alias aliasArr[MAX_ALIASES]; // Storage for up to 100 aliased commands
int aliasCount = 0;          // Counter for the number of aliases

void defAlias(char *name, char *cmdLine) {
    // Check if the alias already exists
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(aliasArr[i].name, name) == 0) {
            perror("This command already exists");
            return;
        }
    }

    // Add the new alias to the array
    if (aliasCount < MAX_ALIASES) {
        aliasArr[aliasCount].name = strdup(name);
        aliasArr[aliasCount].cmdLine = strdup(cmdLine);
        aliasCount++;
    } else {
        perror("Alias limit reached");
    }
}

void deleteAlias(char *name) {
    bool found = false;
    for (int i = 0; i < aliasCount; i++) {
        if (found) {
            aliasArr[i - 1] = aliasArr[i];
        } else if (strcmp(aliasArr[i].name, name) == 0) {
            free(aliasArr[i].name);
            free(aliasArr[i].cmdLine);
            found = true;
        }
    }
    if (found) {
        aliasCount--;
    } else {
        perror("Alias doesn't exist");
    }
}

void printAliases() {// Function to print all aliases
    printf("Current aliases:\n");
    for (int i = 0; i < aliasCount; i++) {
        printf("alias %s='%s'\n", aliasArr[i].name, aliasArr[i].cmdLine);
    }
}

int executeWithAliases(char* argv[]) {
    // Check if the command matches any alias and replace it
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(argv[0], aliasArr[i].name) == 0) {
            printf("Alias match: %s -> %s\n", aliasArr[i].name, aliasArr[i].cmdLine);
            argv[0] = aliasArr[i].cmdLine;
            break;
        }
    }
    return  execvp(argv[0], argv); // Return -1 if execvp fails
}

void displayPrompt(int numOfCmd, int ActiveAliases, int ScriptLines) {
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, ActiveAliases, ScriptLines);
}

void executeScriptFile(const char *fileName, int *numOfCmd, int *scriptLines, int *activeAliases) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        perror("Error opening script file");
        return;
    }

    char line[MAX_CMD_LEN];
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline character if any
        if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        char *argv[MAX_ARGS + 1]; // +1 for the NULL terminator
        const char *delim = " \t\n";
        char *token = strtok(line, delim);
        int argCount = 0;

        while (token != NULL) {
            if (argCount <= MAX_ARGS) {
                argv[argCount] = token;
                argCount++;
            } else {
                fprintf(stderr, "Illegal Command: Too Many Arguments\n");
                argCount = 0; // Reset to avoid partial commands
                break;
            }
            token = strtok(NULL, delim);
        }
        argv[argCount] = NULL;
        if (argCount == 0)
            continue;

        (*scriptLines)++;

        if (argCount > MAX_ARGS) {
            perror("Illegal Command: Too Many Arguments\n");
            continue;
        }
        if (strlen(line) > MAX_CMD_LEN) {
            perror("Illegal Command: Too Many Characters\n");
            continue;
        }

        if (strcmp(argv[0], "alias") == 0) {
            if (argCount == 3) {
                defAlias(argv[1], argv[2]);
                *activeAliases = aliasCount;
            } else if (argCount == 1) {
                printAliases();
            } else {
                perror("Usage: alias name command\n");
            }
            continue;
        } else if (strcmp(argv[0], "unalias") == 0) {
            if (argCount == 2) {
                deleteAlias(argv[1]);
                *activeAliases = aliasCount;
            } else {
                perror("Usage: unalias name\n");
            }
            continue;
        }

        pid_t PID = fork();

        if (PID == -1) {
            perror("Error: fork failed");
            exit(EXIT_FAILURE);
        } else if (PID == 0) { // Child process
            if (executeWithAliases(argv) == -1) {
                fprintf(stderr, "\nError executing command %s\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        } else { // Parent process
            int status;
            waitpid(PID, &status, 0); // Wait for the child process to complete

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                (*numOfCmd)++; // Increment only if the command executed successfully
            }
        }
    }

    fclose(file);
}

int main() {
    char *cmd = (char *) malloc(MAX_CMD_LEN * sizeof(char));
    char *argv[MAX_ARGS + 1]; // +1 for the NULL terminator

    const char *delim = " \t\n";
    int numOfCmd = 0;
    int activeAliases = 0;
    int scriptLines = 0;
    pid_t PID;

    if (cmd == NULL) {
        perror("Error: Memory Allocation failed");
        return 1;
    }

    while (1) {
        displayPrompt(numOfCmd, activeAliases, scriptLines);

        if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
            perror("Error reading input");
            continue;
        }

        // Remove trailing newline character if any
        if (strlen(cmd) > 0 && cmd[strlen(cmd) - 1] == '\n') {
            cmd[strlen(cmd) - 1] = '\0';
        }

        if (strcmp(cmd, "exit") == 0) {
            break;
        }

        char *token = strtok(cmd, delim);
        int argCount = 0;

        while (token != NULL) {
            if (argCount <= MAX_ARGS) {
                argv[argCount] = token;
                argCount++;
            } else {
                fprintf(stderr, "Illegal Command: Too Many Arguments\n");
                argCount = 0; // Reset to avoid partial commands
                break;
            }token = strtok(NULL, delim);
        }
        argv[argCount] = NULL;
        if (argCount == 0)
            continue;

        //script settings : here scriptLines++;

        if (argCount > MAX_ARGS) {
            perror("Illegal Command: Too Many Arguments\n");
            continue;
        }
        if (strlen(cmd) > MAX_CMD_LEN) {
            perror("Illegal Command: Too Many Characters\n");
            continue;
        }

        if (strcmp(argv[0], "alias") == 0) {
            if (argCount == 3) {
                defAlias(argv[1], argv[2]);
                activeAliases = aliasCount;
            } else if (argCount == 1) {
                printAliases();
            } else {
                perror("Usage: alias name command\n");
            }
            continue;
        }
        else if (strcmp(argv[0], "unalias") == 0) {
            if (argCount == 2) {
                deleteAlias(argv[1]);
                activeAliases = aliasCount;
            } else {
                perror("Usage: unalias name\n");
            }
            continue;
        }
        else if (strcmp(argv[0], "source") == 0) {
            if (argCount == 2) {
                executeScriptFile(argv[1], &numOfCmd, &scriptLines,&activeAliases);
            } else {
                perror("Usage: source <script_file>\n");
            }
            continue;
        }

        PID = fork();
        if (PID == -1) {
            perror("Error: fork failed");
            free(cmd);
            exit(EXIT_FAILURE);
        } else if (PID == 0) { // Child process
            if (executeWithAliases(argv) == -1) {
                fprintf(stderr, "\nError executing command %s\n", argv[0]);
                exit(EXIT_FAILURE);
            }

        } else { // Parent process
            int status;
            waitpid(PID, &status, 0); // Wait for the child process to complete

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                numOfCmd++; // Increment only if the command executed successfully
            }
        }
    }

    free(cmd);
    for (int i = 0; i < aliasCount; i++) {
        free(aliasArr[i].name);
        free(aliasArr[i].cmdLine);
    }
    return 0;
}