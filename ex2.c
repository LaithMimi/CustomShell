/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MaxCmdLen 1024
#define MaxArg 4

static int numOfCmd = 0, scriptLines = 0, quotesNum = 0;

void displayPrompt();
void handleCmd(char *cmd, char ch);
int pairsOfQuotes(char *token, char ch);
void checkFunctions(char **argv, int argCount);
void processes(char **argv);
void andOperator(char *cmd);

int main() {
    char *cmd = (char*)malloc(MaxCmdLen * sizeof(char));
    char ch = '"';

    if (cmd == NULL) {
        perror("Error: Memory Allocation failed");
        return 1;
    }

    while (1) {
        displayPrompt();

        if (fgets(cmd, MaxCmdLen, stdin) == NULL) {
            perror("ERR");
            free(cmd);
            return 1;
        }

        if (strcmp(cmd, "exit_shell\n") == 0) {
            printf("The number of quotes is: %d\n", quotesNum);
            break;
        }

        handleCmd(cmd, ch);
    }

    free(cmd);
    return 0;
}

void displayPrompt() {
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, 0, scriptLines);
}

void handleCmd(char *cmd, char ch) {
    // Remove trailing newline character
    if (strlen(cmd) > 0 && cmd[strlen(cmd) - 1] == '\n') {
        cmd[strlen(cmd) - 1] = '\0';
    }

    if (strlen(cmd) > MaxCmdLen) {
        perror("ERR");
        return;
    }

    if (strstr(cmd, "&&")) {
        andOperator(cmd);
        return;
    }

    char *delim = " =";
    char *token = strtok(cmd, delim);
    char **argv = (char **)malloc((MaxArg + 1) * sizeof(char *));

    if (argv == NULL) {
        perror("ERR");
        return;
    }

    int argCount = 0;
    while (token != NULL) {
        if (argCount <= MaxArg) {
            argv[argCount] = token;
            argCount++;
        } else {
            perror("ERR");
            break;
        }
        quotesNum += pairsOfQuotes(token, ch);
        token = strtok(NULL, delim);
    }
    argv[argCount] = NULL;

    if (argCount == 0) {
        free(argv);
        return;
    }

    checkFunctions(argv, argCount);
    free(argv);
}

int pairsOfQuotes(char *token, char ch) {
    int count = 0;
    while (*token) {
        if (*token == ch) {
            count++;
        }
        token++;
    }
    return count / 2;
}

void andOperator(char *cmd) {
    char *delim = "&&";
    char *token;
    char *commands[MaxArg + 1];
    int cmdCount = 0;

    token = strtok(cmd, delim);
    while (token != NULL ) {

        commands[cmdCount] = token;
        cmdCount++;
        token = strtok(NULL, delim);
        if(cmdCount > 3){
            perror("ERR");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < cmdCount; i++) {
        handleCmd(commands[i], '"');
    }
}

void checkFunctions(char **argv, int argCount) {
    if (strcmp(argv[0], "source") == 0) {
        if (argCount < 2) {
            fprintf(stderr, "source: too few arguments\n");
        } else {
            if (strlen(argv[1]) < 3 || strcmp(argv[1] + strlen(argv[1]) - 3, ".sh") != 0) {
                perror("ERR: the file doesn't end with .sh\n");
            } else {
                numOfCmd++;
                // executeScriptFile(argv[1]);  // Assuming this function exists
            }
        }
    } else if (strcmp(argv[0], "exit_shell") == 0) {
        return;
    } else {
        processes(argv);
    }
}

void processes(char **argv) {
    pid_t PID = fork();
    if (PID == -1) {
        perror("ERR");
        exit(EXIT_FAILURE);
    } else if (PID == 0) { // Child process
        execvp(argv[0], argv);
        perror("ERR");
        exit(EXIT_FAILURE);
    } else { // Parent process
        int status;
        wait(&status); // Wait for the child process to complete
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            numOfCmd++; // Increment only if the command executed successfully
        }
    }
}*/
