
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MaxCmdLen 1024
#define MaxArg 4

static int numOfCmd = 0, quotesNum = 0,ErrorFlag =0,
        background=0, jobsCounter = 0;


void displayPrompt();
void handleCmd(char *cmd, char ch);
int pairsOfQuotes(char *token, char ch);
void cmdExecution(char **argv);
void processOperators(char *cmd);
void handle_background(char *cmd);
void sigchld_handler(int sig);


int main() {
    char *cmd = (char*)malloc(MaxCmdLen * sizeof(char));
    char ch = '"';

    if (cmd == NULL) {
        perror("Error: Memory Allocation failed");
        return 1;
    }
    // Set up signal handler for SIGCHLD
    struct sigaction sa;
    sa.sa_handler = &sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        free(cmd);
        exit(EXIT_FAILURE);
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
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, 0, 0);
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

    if (strstr(cmd, "&&") || strstr(cmd, "||")) {
        processOperators(cmd);
        return;
    }
    if (strstr(cmd, "&")){
        handle_background(cmd);
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

    if (strcmp(argv[0], "exit_shell") == 0) {
        free(argv);
        return;
    }

    cmdExecution(argv);
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

void processOperators(char *cmd) {
    char *token;
    char *commands[MaxArg + 1];
    int cmdCount = 0;

    // Handle '&&' operators first
    token = strtok(cmd, "&&");
    while (token != NULL) {
        commands[cmdCount++] = token;
        token = strtok(NULL, "&&");
        if (cmdCount > MaxArg) {
            perror("ERR: Too many commands");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < cmdCount; i++) {
        char *subToken;
        char *subCommands[MaxArg + 1];
        int subCmdCount = 0;

        // Check if the current command contains '||'
        if (strstr(commands[i], "||")) {
            subToken = strtok(commands[i], "||");
            while (subToken != NULL) {
                subCommands[subCmdCount++] = subToken;
                subToken = strtok(NULL, "||");
                if (subCmdCount > MaxArg) {
                    perror("ERR: Too many sub-commands");
                    exit(EXIT_FAILURE);
                }
            }

            for (int j = 0; j < subCmdCount; j++) {
                handleCmd(subCommands[j], '"');
            }
        }

        else {
            handleCmd(commands[i], '"');
            if (ErrorFlag) {  // Stop if a command failed in '&&'
                return;
            }
        }
    }

    ErrorFlag = 0; // Reset the error flag after processing
}

void cmdExecution(char **argv) {
    pid_t PID = fork();
    if (PID == -1) {
        perror("ERR");
        ErrorFlag = 1;
        exit(EXIT_FAILURE);
    }
    else if (PID == 0) { // Child process
            execvp(argv[0], argv);
            perror("ERR");
            usleep(100000);
            exit(EXIT_FAILURE);
    }
    else { // Parent process
        if(!background) {
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                (numOfCmd)++; // Increment only if the command executed successfully
                ErrorFlag = 0;
            } else {
                ErrorFlag = 1;
            }

        }else{
            jobsCounter++;
            printf("[%d] %d\n", jobsCounter, PID);
        }
    }
}

void sigchld_handler(int sig) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child process %d terminated\n", pid);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Increment the number of commands executed successfully
            numOfCmd++;
        } else {
            // Set the error flag if the child process terminated with an error
            ErrorFlag = 1;
        }
    }
}

void handle_background(char *cmd) {
    // Remove the '&' character from the command
    cmd[strlen(cmd) - 1] = '\0';

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
        quotesNum += pairsOfQuotes(token, '"');
        token = strtok(NULL, delim);
    }
    argv[argCount] = NULL;

    if (argCount == 0) {
        free(argv);
        return;
    }

    if (strcmp(argv[0], "exit_shell") == 0) {
        free(argv);
        return;
    }

    background = 1;
    cmdExecution(argv);
    background = 0; // Reset background flag
    free(argv);
}