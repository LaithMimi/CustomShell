#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MaxCmdLen 1024
#define MaxArg 5


typedef struct {
    char *name;
    char *cmdLine;
    struct AliasNode *next;
} AliasNode;

AliasNode *aliasList=NULL;
int aliasCount = 0;

bool searchForExit(char* argv[]);
void displayPrompt(int numOfCmd, int activeAliases, int scriptLines);
int pairsOfQuotes(char *token, char ch);
void deleteAlias(char *name);
void defAlias(char *name, char *cmd);
int executeWithAliases(char* argv[]);
void processes(char *argv[], int *numOfCmd);
void parseAlias(char *cmd, char **argv,int *activeAliases,int argCount);
void executeBuiltInCommands(char *argv[], int argCount, int *activeAliases, int *numOfCmd, int *scriptLines);
void handleCommand(char *cmd,int *numOfCmd, int *activeAliases, int *scriptLines,int *quotesNum,char ch);
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

        handleCommand(cmd, &numOfCmd, &activeAliases, &scriptLines,&quotesNum, ch);
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
void deleteAlias(char *name) {
    AliasNode *current = aliasList;
    AliasNode *previous = NULL;

    while (current != NULL && strcmp(current->name, name) != 0) {
        previous = current;
        current = (AliasNode *) current->next;
    }

    if (current == NULL) {
        fprintf(stderr, "Alias doesn't exist\n");
        return;
    }

    if (previous == NULL) {
        aliasList = (AliasNode *) current->next;
    } else {
        previous->next = current->next;
    }
    aliasCount--;
    free(current->name);
    free(current->cmdLine);
    free(current);
}

void defAlias(char *name, char *cmd) {
    AliasNode *newNode = (AliasNode *)malloc(sizeof(AliasNode));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new alias");
        exit(EXIT_FAILURE);
    }
    newNode->name = strdup(name); // strdup allocates and copies the string
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

int executeWithAliases(char** argv) {
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

            perror("Error executing command");
            exit(EXIT_FAILURE);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    else { // Parent process

        usleep(200000);
        int status;
        wait(&status); // Wait for the child process to complete

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            (*numOfCmd)++; // Increment only if the command executed successfully
        }
    }
}
void executeBuiltInCommands(char **argv, int argCount, int *activeAliases, int *numOfCmd, int *scriptLines) {
    if (strcmp(argv[0], "exit_shell") == 0) {
        return;
    }
    else if (strcmp(argv[0], "source") == 0) {
        if (argCount == 2) {
            executeScriptFile(argv[1], numOfCmd, scriptLines, activeAliases);
        }
        else {
            perror("Usage: source <script_file>\n");
        }
    }else {
        processes(argv, numOfCmd);
    }
}
void handleCommand(char *cmd,int *numOfCmd, int *activeAliases, int *scriptLines,int *quotesNum,char ch) {
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
//        printf("%s\n",command);
//        printf("%s\n",cmd);

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
            perror("Err");
        }
    }else {
        while (token != NULL) {
            if (argCount <= MaxArg) {
                argv[argCount] = token;
                // if(strcmp(token,"alias")!=0)
                argCount++;
            } else {
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
        strcpy(cmd,"exit_shell");
        return;
    }
    executeBuiltInCommands(argv, argCount, activeAliases, numOfCmd, scriptLines);
    free(argv);

}

void executeScriptFile(const char *fileName, int *numOfCmd, int *scriptLines, int *activeAliases) {

    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    char line[MaxCmdLen];
    char ch = '"'; // The character to count pairs of quotes
    int quotesNum = 0; // Initialize the count of quotes

    while (fgets(line, sizeof(line), fp)) {
        // Remove trailing newline character if any
        if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        // Execute the command in the script file
        handleCommand(line, numOfCmd, activeAliases, scriptLines, &quotesNum, ch);
        (*scriptLines)++;
    }

    fclose(fp);
}