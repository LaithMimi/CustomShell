#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MaxCmdLen 1024
#define MaxArg 5


typedef struct AliasNode{
    char *name;
    char *cmdLine;
    struct AliasNode *next;
} AliasNode;

AliasNode *aliasList=NULL;
int aliasCount = 0;

bool searchForExit(char* argv[]);
void displayPrompt(int numOfCmd, int activeAliases, int scriptLines);
void printAliases();
int  pairsOfQuotes(char *token, char ch);
void removeAlias(char *name);
void defAlias(char *name, char *cmd);
int  executeAliases(char* argv[]);
void processes(char *argv[], int *numOfCmd);
void cd(char *path);
void parseAlias(char *cmd, char **argv,int *activeAliases,int argCount);
void checkFunctions(char *argv[], int argCount, int *activeAliases, int *numOfCmd, int *scriptLines);
void handleCmd(char *cmd, int *numOfCmd, int *activeAliases, int *scriptLines, int *quotesNum, char ch);
void executeScriptFile(const char *fileName, int *numOfCmd, int *scriptLines, int *activeAliases);

int main(){
    char *cmd=(char*)malloc(MaxCmdLen*sizeof(char));
    int numOfCmd=0,activeAliases=0,scriptLines=0,quotesNum=0;
    char ch ='"';

    if(cmd==NULL){
        perror("Error: Memory Allocation failed");
        return 1;
    }
    while (1) {
        if (strcmp(cmd, "exit_shell") == 0) {
            printf("The number of quotes is: %d\n", quotesNum);
            break;
        }
        displayPrompt(numOfCmd,activeAliases,scriptLines);
        if (fgets(cmd,MaxCmdLen,stdin)==NULL){
            perror("Err");
            free(cmd);
            return 1;
        }

        handleCmd(cmd, &numOfCmd, &activeAliases, &scriptLines, &quotesNum, ch);
    }

    free(cmd);
    AliasNode *current = aliasList;
    while (current != NULL) {
        AliasNode *next = (AliasNode *) current->next;
        free(current->name);
        free(current->cmdLine);
        free(current);
        current = next;
    }
    return 0;
}

bool searchForExit(char* argv[]) {
    AliasNode *current = aliasList;
    while (current != NULL) {
        if (strcmp(argv[0], current->name) == 0) {
            if (strcmp(current->cmdLine, "exit_shell") == 0) {
                return true;
            } else {
                // Replace the command with the alias command
                strcpy(argv[0], current->cmdLine);
            }
        }
        current = (AliasNode *) current->next;
    }
    return false;
}
void displayPrompt(int numOfCmd, int activeAliases, int scriptLines){
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, activeAliases, scriptLines);
}
int pairsOfQuotes(char *token, char ch) {
    int count = 0;
    while (*token) {
        if (*token == ch) {
            count++;
        }
        token++;
    }
    return count/2;
}
void removeAlias(char *name) {
    AliasNode *current = aliasList;
    AliasNode *previous = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (previous == NULL) {
                aliasList = current->next;
            } else {
                previous->next = current->next;
            }
            free(current->name);
            free(current->cmdLine);
            free(current);
            aliasCount--;
            return;
        }
        previous = current;
        current = current->next;
    }
}

void defAlias(char *name, char *cmd) {
    AliasNode *current = aliasList;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            fprintf(stderr, "This command already exists\n");
            return;
        }
        current = (AliasNode *) current->next;
    }

    AliasNode *newNode = (AliasNode *)malloc(sizeof(AliasNode));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new alias");
        exit(EXIT_FAILURE);
    }
    //strdup allocates and copies the string
    newNode->name = strdup(name);
    newNode->cmdLine = strdup(cmd);
    newNode->next = (struct AliasNode *) aliasList;
    aliasList = newNode;
    aliasCount++;
}

void printAliases() {
    AliasNode *current = aliasList;
    printf("Aliases list:\n");
    while (current != NULL) {
        printf("alias %s='%s'\n", current->name, current->cmdLine);
        current = (AliasNode *) current->next;
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
    argv[argCount+1] = NULL;
    if (argCount == 1 ) {
        printAliases();
    }else if (argCount == 2) {
        defAlias(argv[0], argv[1]);
        *activeAliases = aliasCount;
    } else {
        perror("Usage: alias name 'command'\n");
    }
}

int executeAliases(char** argv) {
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(argv[0], aliasList[i].name) == 0) {
            // Found the alias, replace the command
            char *aliasedCommand = aliasList[i].cmdLine;

            // Tokenize the aliased command
            char *token;
            char *newArgv[MaxArg ];
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
    return 0;
}
void cd(char *path) {
    if (path == NULL || strcmp(path, "") == 0) {
        path = getenv("home");
        if (path == NULL) {
            perror("cd: home environment variable not set\n");
            return;
        }
    }

    if (chdir(path) != 0) {
        perror("cd");
    }
}
void processes(char **argv, int *numOfCmd) {
    pid_t PID = fork();
    if (PID == -1) {
        perror("Error: fork failed");
        exit(EXIT_FAILURE);
    }
    else if (PID == 0) { // Child process
        if (executeAliases(argv) == 0) {
            execvp(argv[0], argv);

            perror("Error executing command");
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
void checkFunctions(char **argv, int argCount, int *activeAliases, int *numOfCmd, int *scriptLines) {
    if (strcmp(argv[0], "cd") == 0) {
        if (argCount == 2) {
            cd(argv[1]);
        } else {
            perror("Usage: cd <directory>\n");
        }
    }
    else if (strcmp(argv[0], "source") == 0) {
        if (argCount == 2) {
            if(strcmp(argv[1]+ strlen(argv[1]-3),".sh")!=0){
                printf("ERR: the file doesn't end with .sh\n");
            }
            executeScriptFile(argv[1], numOfCmd, scriptLines, activeAliases);
        }
        else {
            perror("Usage: source <script_file>\n");
        }
    }
    else if (strcmp(argv[0], "exit_shell") == 0) {
        return;
    }
    else {
        processes(argv, numOfCmd);
    }
}
void handleCmd(char *cmd, int *numOfCmd, int *activeAliases, int *scriptLines, int *quotesNum, char ch) {
    //first I should remove the \n
    if (strlen(cmd) > 0 && cmd[(strlen(cmd)) - 1] == '\n') {
        cmd[strlen(cmd) - 1] = '\0'; // Remove trailing newline character
    }
    if (strlen(cmd) > MaxCmdLen) {
        perror("Illegal Command: Too Many Characters\n");
        return;
    }

    char *delim = " =";
    char *token = strtok(cmd, delim);
    char **argv = (char **) malloc((MaxArg + 1) * sizeof(char *));

    if (argv == NULL) {
        perror("Err");
        return;
    }

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
        parseAlias(cmd,argv,activeAliases,argCount);
    }
    else if(token != NULL && strcmp(token, "unalias") == 0) {
        while (token != NULL) {
            argv[argCount++] = token;
            token = strtok(NULL, " ");
        }
        argv[argCount] = NULL;
        if (argCount == 2) {
            removeAlias(argv[1]);
            *activeAliases = aliasCount;
            return;
        }
        else {
            perror("Err");
        }
    }
    else {
        while (token != NULL) {
            if (argCount <= MaxArg) {
                argv[argCount] = token;
                argCount++;
            }
            else {
                perror("Too many arguments\n");
                break;
            }
            *quotesNum += pairsOfQuotes(token, ch);
            token = strtok(NULL, delim);
        }

        argv[argCount] = NULL;
    }

    if (argCount == 0) {
        free(argv);
        return;
    }
    if(searchForExit(argv)){
        printf("The number of quotes is: %d\n", *quotesNum);
        exit(0);
    }

    checkFunctions(argv, argCount, activeAliases, numOfCmd, scriptLines);
    free(argv);
}
void executeScriptFile(const char *fileName, int *numOfCmd, int *scriptLines, int *activeAliases) {
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    char line[MaxCmdLen];
    char ch = '"';
    int quotesNum = 0;

    while (fgets(line, sizeof(line), fp)!=NULL) {
        // Remove trailing newline character if any
        if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        if(strcmp(line,"#!/bin/bash")!=0){
            printf("ERR: No #!/bin/bash\n");
            continue;
        }
        handleCmd(line, numOfCmd, activeAliases, scriptLines, &quotesNum, ch);
        (*scriptLines)++;
    }
    if (ferror(fp)) {
        perror("Error reading file");
    }

    fclose(fp);
}