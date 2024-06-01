/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 5
#define MAX_ALIASES 100   // Maximum number of aliases

typedef struct {
    char *name;
    char *cmdLine;
} AliasNode;

AliasNode aliasList[MAX_ALIASES]; // Storage for up to 100 aliased commands
int aliasCount = 0;          // Counter for the number of aliases

// Function prototypes
void displayPrompt(int numOfCmd, int activeAliases, int scriptLines);
int pairsOfQuotes(const char *cmd, char ch);
void printAliases();
void executeScriptFile(const char *filename, int *numOfCmd, int *scriptLines, int *activeAliases);
int executeWithAliases(char *argv[]);
void handleCommand(char *cmd, int *numOfCmd, int *activeAliases, int *scriptLines, int *result, char ch);
void executeBuiltInCommands(char *argv[], int argCount, int *activeAliases, int *numOfCmd, int *scriptLines);
void processes(char *argv[], int *numOfCmd);

//Functions implementations
void displayPrompt(int numOfCmd, int ActiveAliases, int ScriptLines) {
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, ActiveAliases, ScriptLines);
}

int pairsOfQuotes(const char *cmd, char ch) {
    int count = 0;
    while (*cmd) {
        if (*cmd == ch) {
            count++;
        }
        cmd++;
    }

    return count/2;
}

void printAliases() {// Function to print all aliases
    printf("Current aliases:\n");
    for (int i = 0; i < aliasCount; i++) {
        printf("alias %s='%s'\n", aliasList[i].name, aliasList[i].cmdLine);
    }
}

void defAlias(char *name, char *cmdLine) {
    // Check if the alias already exists
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(aliasList[i].name, name) == 0) {
            perror("This command already exists");
            return;
        }
    }

    // Add the new alias to the array
    if (aliasCount < MAX_ALIASES) {
        aliasList[aliasCount].name = strdup(name);
        aliasList[aliasCount].cmdLine = strdup(cmdLine);
        aliasCount++;
    } else {
        perror("AliasNode limit reached");
    }
}

void deleteAlias(char *name) {
    bool found = false;
    for (int i = 0; i < aliasCount; i++) {
        if (found) {
            aliasList[i - 1] = aliasList[i];
        } else if (strcmp(aliasList[i].name, name) == 0) {
            free(aliasList[i].name);
            free(aliasList[i].cmdLine);
            found = true;
        }
    }
    if (found) {
        aliasCount--;
    } else {
        perror("AliasNode doesn't exist");
    }
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

int executeWithAliases(char* argv[]) {
    // Check if the command matches any alias and replace it
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(argv[0], aliasList[i].name) == 0) {
            printf("AliasNode match: %s -> %s\n", aliasList[i].name, aliasList[i].cmdLine);
            char *token = strtok(aliasList[i].cmdLine, " "); // Tokenize the command line
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
                token = strtok(NULL, " ");
            }
            argv[argCount] = NULL;
            if (argCount == 0)
                continue;

            return execvp(argv[0], argv); // Return -1 if execvp fails
        }
    }
    return execvp(argv[0], argv); // Return -1 if no alias is found
}

void handleCommand(char *cmd, int *numOfCmd, int *activeAliases, int *scriptLines, int *result, char ch) {
    size_t len = strlen(cmd);
    if (len > 0 && cmd[len - 1] == '\n') {
        cmd[len - 1] = '\0'; // Remove trailing newline character
    }

    *result += pairsOfQuotes(cmd, ch);

    char *argv[MAX_ARGS + 1]; // +1 for the NULL terminator
    const char *delim = " ";
    char *token = strtok(cmd, delim);
    int argCount = 0;

    while (token != NULL) {
        if (argCount <= MAX_ARGS) {
            argv[argCount] = token;
            argCount++;
        } else {
            perror("Illegal Command: Too Many Arguments\n");
            return;
        }
        token = strtok(NULL, delim);
    }
    argv[argCount] = NULL;

    if (argCount == 0) return;

    if (strlen(cmd) > MAX_CMD_LEN) {
        perror("Illegal Command: Too Many Characters\n");
        return;
    }

    executeBuiltInCommands(argv, argCount, activeAliases, numOfCmd, scriptLines);
}

void executeBuiltInCommands(char *argv[], int argCount, int *activeAliases, int *numOfCmd, int *scriptLines) {
    if (strcmp(argv[0], "alias") == 0) {
        if (argCount == 3) {
            defAlias(argv[1], argv[2]);
            *activeAliases = aliasCount;
        }
        else if (argCount == 1) {
            printAliases();
        }
        else {
            perror("Usage: alias name 'command'\n");
        }
    }
    else if (strcmp(argv[0], "unalias") == 0) {
        deleteAlias(argv[1]);
        *activeAliases = aliasCount;
    }
    else if (strcmp(argv[0], "source") == 0) {
        if (argCount == 2) {
            executeScriptFile(argv[1], numOfCmd, scriptLines, activeAliases);
        }
        else {
            perror("Usage: source <script_file>\n");
        }
    }
    else {
        processes(argv, numOfCmd);
    }
}

void processes(char *argv[], int *numOfCmd) {
    pid_t PID = fork();
    if (PID == -1) {
        perror("Error: fork failed");
        exit(EXIT_FAILURE);
    } else if (PID == 0) { // Child process
        if (executeWithAliases(argv) == -1) {
            execvp(argv[0], argv);
            perror("Error executing command\n");
            exit(EXIT_FAILURE);
        }
    } else { // Parent process
        int status;
        wait(&status); // Wait for the child process to complete

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            (*numOfCmd)++; // Increment only if the command executed successfully
        }
    }
}

int main() {
    char *cmd = (char *) malloc(MAX_CMD_LEN * sizeof(char));
    int numOfCmd = 0;
    int activeAliases = 0;
    int scriptLines = 0;
    int result = 0;
    char ch = '"';

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

        handleCommand(cmd, &numOfCmd, &activeAliases, &scriptLines, &result, ch);

        if (strcmp(cmd, "exit_shell") == 0) {
            printf("The number of quotes is: %d\n", result);
            break;
        }
    }

    free(cmd);
    // Free any allocated memory for aliases
    for (int i = 0; i < aliasCount; i++) {
        free(aliasList[i].name);
        free(aliasList[i].cmdLine);
    }

    return 0;
}*/
/*
 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 5
#define MAX_ALIASES 100   // Maximum number of aliases

typedef struct {
    char *name;
    char *cmdLine;
} AliasNode;

AliasNode *aliasList;
int aliasCount = 0;          // Counter for the number of aliases

// Function prototypes
void displayPrompt(int numOfCmd, int activeAliases, int scriptLines);
int pairsOfQuotes(const char *cmd, char ch);
void printAliases();
void executeScriptFile(const char *filename, int *numOfCmd, int *scriptLines, int *activeAliases);
int executeWithAliases(char *argv[]);
void handleCommand(char *cmd, int *numOfCmd, int *activeAliases, int *scriptLines, int *result, char ch);
void executeBuiltInCommands(char *argv[], int argCount, int *activeAliases, int *numOfCmd, int *scriptLines);
void processes(char *argv[], int *numOfCmd);

//Functions implementations
void displayPrompt(int numOfCmd, int ActiveAliases, int ScriptLines) {
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, ActiveAliases, ScriptLines);
}

int pairsOfQuotes(const char *cmd, char ch) {
    int count = 0;
    while (*cmd) {
        if (*cmd == ch) {
            count++;
        }
        cmd++;
    }

    return count/2;
}

void printAliases() {// Function to print all aliases
    printf("Current aliases:\n");
    for (int i = 0; i < aliasCount; i++) {
        printf("alias %s='%s'\n", aliasList[i].name, aliasList[i].cmdLine);
    }
}

void defAlias(char *name, char *cmd) {
    if (aliasCount < MAX_ALIASES) {
        aliasList[aliasCount].name = malloc(strlen(name) + 1);
        strcpy(aliasList[aliasCount].name, name);
        aliasList[aliasCount].cmdLine = malloc(strlen(cmd) + 1);
        strcpy(aliasList[aliasCount].cmdLine, cmd);
        aliasCount++;
    } else {
        perror("Error: Maximum aliases reached");
    }
}

void deleteAlias(char *name) {
    bool found = false;
    for (int i = 0; i < aliasCount; i++) {
        if (found) {
            // Shift the remaining aliases down one position
            aliasList[i - 1] = aliasList[i];
        } else if (strcmp(aliasList[i].name, name) == 0) {
            // Free the memory of the alias to be deleted
            free(aliasList[i].name);
            free(aliasList[i].cmdLine);
            found = true;
        }
    }

    if (found) {
        aliasCount--;
        // Resize the alias array to release unused memory
        aliasList = realloc(aliasList, aliasCount * sizeof(AliasNode));
        if (aliasList == NULL && aliasCount > 0) {
            perror("Failed to resize alias array");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "AliasNode doesn't exist\n");
    }
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

int executeWithAliases(char* argv[]) {
    // Check if the command matches any alias and replace it
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(argv[0], aliasList[i].name) == 0) {
            printf("AliasNode match: %s -> %s\n", aliasList[i].name, aliasList[i].cmdLine);

            // Create a copy of the alias command line to tokenize
            char *aliasCmdLine = strdup(aliasList[i].cmdLine);
            if (!aliasCmdLine) {
                perror("Failed to duplicate alias command line");
                return -1;
            }

            // Tokenize the alias command line
            char *token = strtok(aliasCmdLine, " ");
            char *newArgv[MAX_ARGS + 1]; // +1 for NULL terminator
            int argCount = 0;

            while (token != NULL) {
                if (argCount < MAX_ARGS) {
                    newArgv[argCount] = token;
                    argCount++;
                } else {
                    fprintf(stderr, "Illegal Command: Too Many Arguments\n");
                    free(aliasCmdLine); // Free the duplicated command line
                    return -1;
                }
                token = strtok(NULL, " ");
            }
            newArgv[argCount] = NULL;

            // Execute the alias command
            int result = execvp(newArgv[0], newArgv);
            free(aliasCmdLine); // Free the duplicated command line
            return result; // Return the result of execvp (will be -1 on failure)
        }
    }

    // No alias match, execute the original command
    return execvp(argv[0], argv); // Return the result of execvp (will be -1 on failure)
}
/*void parseAlias(char *aliasDefinition) {
//    // Find the position of the '=' character
//    char *equalPos = strchr(aliasDefinition, '=');
//    if (equalPos == NULL) {
//        printf("Invalid alias definition.\n");
//        return;
//    }
//
//    // Extract the alias name
//    int aliasNameLength = equalPos - aliasDefinition;
//    char aliasName[aliasNameLength + 1];
//    strncpy(aliasName, aliasDefinition, aliasNameLength);
//    aliasName[aliasNameLength] = '\0'; // Null-terminate the string
//
//    // Extract the command part
//    char *commandPart = equalPos + 1; // Skip the '=' character
//
//    // Remove surrounding single quotes if present
//    if (commandPart[0] == '\'' && commandPart[strlen(commandPart) - 1] == '\'') {
//        commandPart[strlen(commandPart) - 1] = '\0'; // Remove the trailing quote
//        commandPart++; // Skip the leading quote
//    }
//
//    // Add the alias to the alias array
//    defAlias(aliasName, commandPart);
//}
void handleCommand(char *cmd, int *numOfCmd, int *activeAliases, int *scriptLines, int *result, char ch) {
    size_t len = strlen(cmd);
    if (len > 0 && cmd[len - 1] == '\n') {
        cmd[len - 1] = '\0'; // Remove trailing newline character
    }

    *result += pairsOfQuotes(cmd, ch);

    char *equalPos = strchr(cmd, '=');
    char *argv[MAX_ARGS + 1]; // +1 for the NULL terminator
    const char *delim = " ";
    char *token = strtok(cmd, delim);
    int argCount = 0;


    while (token != NULL) {
        if (strcmp(argv[0], "alias") == 0 ) {
            if (equalPos == NULL) {
                printf("Invalid alias definition.\n");
                return;
            }
            int aliasNameLength = equalPos - cmd;

            char aliasName[aliasNameLength + 1];
            strncpy(aliasName, cmd, aliasNameLength);
            aliasName[aliasNameLength] = '\0'; // Null-terminate the string

            // Extract the command part
            char *commandPart = equalPos + 1; // Skip the '=' character

            // Remove surrounding single quotes if present
            if (commandPart[0] == '\'' && commandPart[strlen(commandPart) - 1] == '\'') {
                commandPart[strlen(commandPart) - 1] = '\0'; // Remove the trailing quote
                commandPart++; // Skip the leading quote
            }

            // Add the alias to the alias array
            defAlias(aliasName, commandPart);
        }
        if (argCount <= MAX_ARGS) {
            argv[argCount] = token;
            argCount++;
        } else {
            perror("Illegal Command: Too Many Arguments\n");
            return;
        }
        token = strtok(NULL, delim);
    }

    argv[argCount] = NULL;

    if (argCount == 0) return;

    if (strlen(cmd) > MAX_CMD_LEN) {
        perror("Illegal Command: Too Many Characters\n");
        return;
    }

    executeBuiltInCommands(argv, argCount, activeAliases, numOfCmd, scriptLines);
}

void executeBuiltInCommands(char *argv[], int argCount, int *activeAliases, int *numOfCmd, int *scriptLines) {
    if (strcmp(argv[0], "alias") == 0) {
        for(int i=0;i<MAX_ALIASES;i++){
            printf("%s",argv[i]);
        }
        if (argCount == 3) {
            defAlias(argv[1], argv[2]);
            *activeAliases = aliasCount;
        }
        else if (argCount == 1) {
            printAliases();
        }
        else {
            perror("Usage: alias name 'command'\n");
        }
    }
    else if (strcmp(argv[0], "unalias") == 0) {
        deleteAlias(argv[1]);
        *activeAliases = aliasCount;
    }
    else if (strcmp(argv[0], "source") == 0) {
        if (argCount == 2) {
            executeScriptFile(argv[1], numOfCmd, scriptLines, activeAliases);
        }
        else {
            perror("Usage: source <script_file>\n");
        }
    }
    else {
        processes(argv, numOfCmd);
    }
}

void processes(char *argv[], int *numOfCmd) {
    pid_t PID = fork();
    if (PID == -1) {
        perror("Error: fork failed");
        exit(EXIT_FAILURE);
    } else if (PID == 0) { // Child process
        if (executeWithAliases(argv) == -1) {
            execvp(argv[0], argv);
            perror("Err");
            exit(EXIT_FAILURE);
        }
    } else { // Parent process
        usleep(200000);
        int status;
        wait(&status); // Wait for the child process to complete

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            (*numOfCmd)++; // Increment only if the command executed successfully
        }
    }
}

int main() {
    aliasList =(AliasNode *)malloc(MAX_ALIASES * sizeof(AliasNode));
    char *cmd = (char *) malloc(MAX_CMD_LEN * sizeof(char));
    int numOfCmd = 0;
    int activeAliases = 0;
    int scriptLines = 0;
    int result = 0;
    char ch = '"';

    if (cmd == NULL) {
        perror("Error: Memory Allocation failed");
        return 1;
    }
    if(aliasList==NULL){
        perror("Eroor: Memory ALlocation failed");
        free(cmd);
        return 1;
    }
    for (int i=0;i<MAX_ALIASES;i++){
        aliasList[i].name = (char *)malloc(MAX_CMD_LEN * sizeof(char));
        if (aliasList[i].name == NULL) {
            fprintf(stderr, "Memory allocation failed for aliasList[%d].name\n", i);
            // Free previously allocated memory before exiting
            for (int j = 0; j < i; j++) {
                free(aliasList[j].name);
            }
            free(aliasList);
            free(cmd);
            return 1;
        }
        aliasList[i].cmdLine=(char*) malloc (MAX_CMD_LEN*sizeof(char));
        if(aliasList[i].cmdLine==NULL){
            fprintf(stderr, "Memory allocation failed for aliasList[%d].cmdLine\n", i);
            // Free previously allocated memory before exiting
            free(aliasList[i].name);
            for (int j = 0; j < i; j++) {
                free(aliasList[j].name);
                free(aliasList[j].cmdLine);
            }
            free(aliasList);
            free(cmd);
            return 1;
        }
    }
    while (1) {
        displayPrompt(numOfCmd, activeAliases, scriptLines);

        if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
            perror("Error reading input");
            continue;
        }

        handleCommand(cmd, &numOfCmd, &activeAliases, &scriptLines, &result, ch);

        if (strcmp(cmd, "exit_shell") == 0) {
            printf("The number of quotes is: %d\n", result);
            break;
        }
    }

    for (int i = 0; i < MAX_ALIASES; i++) {
        free(aliasList[i].name);
        free(aliasList[i].cmdLine);
    }
    free(aliasList);
    free(cmd);

    return 0;
}*/
