#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MaxCmdLen 1024
#define MaxArg 5
#define MaxAliases 100

typedef struct {
    char *name;
    char *cmdLine;
} Alias;

Alias *aliasArr=NULL;
int aliasCount = 0;

void displayPrompt(int numOfCmd, int activeAliases, int scriptLines);
int pairsOfQuotes(const char *cmd, char ch);
void deleteAlias(char *name);
void defAlias(char *name, char *cmd);
int executeWithAliases(char* argv[]);
void processes(char *argv[], int *numOfCmd);
void parseAlias(char *cmd, char **argv,int *activeAliases,int argCount);
void executeBuiltInCommands(char *argv[], int argCount, int *activeAliases, int *numOfCmd, int *scriptLines);
void handleCommand(char *cmd,int *numOfCmd, int *activeAliases, int *scriptLines,int *quotesNum,char ch);


int main(){
    char *cmd=(char*)malloc(MaxCmdLen*sizeof(char));
    int numOfCmd=0,activeAliases=0,scriptLines=0,quotesNum=0;
    char ch ='"';

    if(cmd==NULL){
        perror("Error: Memory Allocation failed");
        return 1;
    }

    aliasArr=(Alias*)malloc(MaxAliases * sizeof(Alias));
    if (aliasArr==NULL) {
        perror("Error: Memory Allocation for aliasArr failed");
        free(cmd);
        return 1;
    }

    while (1) {
        displayPrompt(numOfCmd,activeAliases,scriptLines);

        if (fgets(cmd,MaxCmdLen,stdin)==NULL){
            perror("Err");
            free(cmd);
            free(aliasArr);
            return 1;
        }

        handleCommand(cmd, &numOfCmd, &activeAliases, &scriptLines,&quotesNum, ch);

        if (strcmp(cmd, "exit_shell") == 0) {
            printf("The number of quotes is: %d\n", quotesNum);
            break;
        }
    }

    free(cmd);
    for (int i = 0; i < aliasCount; i++) {
        free(aliasArr[i].name);
        free(aliasArr[i].cmdLine);
    }
    free(aliasArr);
    return 0;
}

void displayPrompt(int numOfCmd, int activeAliases, int scriptLines){
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, activeAliases, scriptLines);
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
        // Use a temporary pointer for realloc
        Alias *temp = realloc(aliasArr, aliasCount * sizeof(Alias));
        if (temp == NULL && aliasCount > 0) {
            perror("Failed to resize alias array");
            // In case of failure, keep the original buffer and exit
            exit(EXIT_FAILURE);
        }
        aliasArr = temp;
    } else {
        fprintf(stderr, "Alias doesn't exist\n");
    }
}
void defAlias(char *name, char *cmd) {
    if (aliasCount < MaxAliases) {
        aliasArr[aliasCount].name = malloc(strlen(name) + 1);
        strcpy(aliasArr[aliasCount].name, name);

        aliasArr[aliasCount].cmdLine = malloc(strlen(cmd) + 1);
        strcpy(aliasArr[aliasCount].cmdLine, cmd);

        aliasCount++;
    } else {
        perror("Err");
    }
}
void printAliases(){
    printf("Current aliases:\n");
    for (int i = 0; i < aliasCount; i++) {
        printf("alias %s='%s'\n", aliasArr[i].name, aliasArr[i].cmdLine);
    }
}
void parseAlias(char *cmd, char **argv,int *activeAliases,int argCount) {
    char *delim = "=";
    char *token = strtok(cmd, delim);
    while (token != NULL) {
        argv[argCount] = token;
        argCount++;
        token = strtok(NULL, delim);
    }
    argv[argCount] = NULL;
    if (argCount == 1 ) {
        printAliases();
    }else if (argCount == 3) {
        defAlias(argv[1], argv[2]);
        *activeAliases = aliasCount;
    } else {
        perror("Usage: alias name 'command'\n");
    }
}

int executeWithAliases(char** argv) {
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(argv[0], aliasArr[i].name) == 0) {
            // Found the alias, replace the command
            char *aliasedCommand = aliasArr[i].cmdLine;

            // Tokenize the aliased command
            char *token;
            char *newArgv[MaxArg + 1];
            int newArgCount = 0;

            token = strtok(aliasedCommand, " ");
            while (token != NULL && newArgCount < MaxArg) {
                newArgv[newArgCount++] = token;
                token = strtok(NULL, " ");
            }
            newArgv[newArgCount] = NULL;

            // Execute the aliased command
            execvp(newArgv[0], newArgv);

            perror("Error executing aliased command");
            return -1;

        }
    }
    // No alias found, return 0 to indicate normal execution should proceed
    return 0;
}
void processes(char **argv, int *numOfCmd) {
    pid_t PID = fork();
    if (PID == -1) {
        perror("Error: fork failed");
        exit(EXIT_FAILURE);
    }
    else if (PID == 0) { // Child process
        if (executeWithAliases(argv) == 0) {
            execvp(argv[0], argv);

            perror("Error executing command\n");
            exit(EXIT_FAILURE);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    else { // Parent process
        usleep(100000);
        int status;
        wait(&status); // Wait for the child process to complete

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            (*numOfCmd)++; // Increment only if the command executed successfully
        }
    }
}
void executeBuiltInCommands(char **argv, int argCount, int *activeAliases, int *numOfCmd, int *scriptLines) {
    if (strcmp(argv[0], "source") == 0) {
        if (argCount == 2) {
            //executeScriptFile(argv[1], numOfCmd, scriptLines, activeAliases);
        }
        else {
            perror("Usage: source <script_file>\n");
        }
    }else {
        processes(argv, numOfCmd);
    }
}
void handleCommand(char *cmd,int *numOfCmd, int *activeAliases, int *scriptLines,int *quotesNum,char ch) {
    //first i should remove the \n
    if (strlen(cmd) > 0 && cmd[(strlen(cmd)) - 1] == '\n') {
        cmd[strlen(cmd) - 1] = '\0'; // Remove trailing newline character
    }

    char *delim = " =";
    char *token = strtok(cmd, delim);
    char **argv = (char **) malloc((MaxArg + 1) * sizeof(char *));

    if (argv == NULL) {
        perror("Err\n");
        return;
    }

    *quotesNum += pairsOfQuotes(cmd, ch);
    int argCount = 0;

    if (token != NULL && strcmp(token, "alias") == 0) {
        // Remove single quotes from the command
        char *command = strtok(NULL, "");

        if (command != NULL) {
            char *ptr = strchr(command, '\'');
            while (ptr != NULL) {
                memmove(ptr, ptr + 1, strlen(ptr));
                ptr = strchr(ptr, '\'');
            }
            strcpy(cmd,command);
        }
        //printf("%s",command);

        parseAlias(cmd,argv,activeAliases,argCount);
    }
    else if(token != NULL && strcmp(token, "unalias") == 0) {
        while (token != NULL) {
            argv[argCount++] = token;
            token = strtok(NULL, " ");
        }
        argv[argCount] = NULL;
        if (argCount == 2) {
            deleteAlias(argv[1]);
            *activeAliases = aliasCount;
        } else {
            perror("Err\n");
        }
    }
    else {
        while (token != NULL) {
            if (argCount <= MaxArg) {
                argv[argCount] = token;
                argCount++;
            } else {
                perror("Too many arguments\n");
                break;
            }
            token = strtok(NULL, delim);
        }
        argv[argCount] = NULL;
    }

    if (argCount == 0) return;

    if (strlen(cmd) > MaxCmdLen) {
        perror("Illegal Command: Too Many Characters\n");
        return;
    }
    executeBuiltInCommands(argv, argCount, activeAliases, numOfCmd, scriptLines);

    free(argv);
}
