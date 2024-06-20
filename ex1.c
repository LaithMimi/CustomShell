/*
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/wait.h>
//#include <stdbool.h>
//#include <signal.h>
//
//#define MaxCmdLen 1024
//#define MaxArg 5
//static int numOfCmd=0,activeAliases=0,scriptLines=0,quotesNum=0,ErrorFlag =0,
//            background=0;
//volatile int jobsCounter = 0;
//
//typedef struct AliasNode{
//    char *name;
//    char *cmdLine;
//    struct AliasNode *next;
//} AliasNode;
//
//AliasNode *aliasList=NULL;
//int aliasCount = 0;
//
//bool searchForExit(char* argv[]);
//void displayPrompt();
//void printAliases();
//int  pairsOfQuotes(char *token, char ch);
//void removeAlias(char *name);
//void defAlias(char *name, char *cmd);
//int  executeAliases(char* argv[]);
//void cmdExecution(char *argv[]);
////void cd(char *path);
//void parseAlias(char *cmd, char **argv,int argCount);
//void checkFunctions(char *argv[], int argCount);
//void handleCmd(char *cmd, char ch);
//void executeScriptFile(const char *fileName);
//
//int main(){
//    char *cmd=(char*)malloc(MaxCmdLen*sizeof(char));
//    char ch ='"';
//
//    if(cmd==NULL){
//        perror("Error: Memory Allocation failed");
//        return 1;
//    }
//
//    while (1) {
//        if (strcmp(cmd, "exit_shell") == 0) {
//            printf("%d\n", quotesNum);
//            break;
//        }
//        displayPrompt();
//        if (fgets(cmd,MaxCmdLen,stdin)==NULL){
//            perror("ERR");
//            free(cmd);
//            return 1;
//        }
//        handleCmd(cmd, ch);
//    }
//
//    free(cmd);
//    AliasNode *current = aliasList;
//    while (current != NULL) {
//        AliasNode *next = (AliasNode *) current->next;
//        free(current->name);
//        free(current->cmdLine);
//        free(current);
//        current = next;
//    }
//    return 0;
//}
//bool searchForExit(char* argv[]) {
//    AliasNode *current = aliasList;
//    while (current != NULL) {
//        if (strcmp(argv[0], current->name) == 0) {
//            if (strcmp(current->cmdLine, "exit_shell") == 0) {
//                return true;
//            } else {
//                // Replace the command with the alias command
//                strcpy(argv[0], current->cmdLine);
//            }
//        }
//        current = (AliasNode *) current->next;
//    }
//    return false;
//}
//void displayPrompt(){
//    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, activeAliases, scriptLines);
//}
//int pairsOfQuotes(char *token, char ch) {
//    int count = 0;
//    while (*token) {
//        if (*token == ch) {
//            count++;
//        }
//        token++;
//    }
//    return count/2;
//}
//void removeAlias(char *name) {
//    AliasNode *current = aliasList;
//    AliasNode *previous = NULL;
//
//    while (current != NULL) {
//        if (strcmp(current->name, name) == 0) {
//            if (previous == NULL) {
//                aliasList = current->next;
//            } else {
//                previous->next = current->next;
//            }
//            free(current->name);
//            free(current->cmdLine);
//            free(current);
//            aliasCount--;
//            return;
//        }
//        previous = current;
//        current = current->next;
//    }
//}
//void defAlias(char *name, char *cmd) {
//    AliasNode *current = aliasList;
//
//    while (current != NULL) {// Check for existing alias
//        if (strcmp(current->name, name) == 0) {
//            // Found existing alias, override
//            free(current->cmdLine); // Free memory of old command
//            current->cmdLine = strdup(cmd); // Assign new command
//            return; // No need to create a new node
//        }
//        current = current->next;
//    }
//
//    AliasNode *newNode = (AliasNode *)malloc(sizeof(AliasNode));
//    if (newNode == NULL) {
//        perror("ERR");
//        exit(EXIT_FAILURE);
//    }
//    //strdup allocates and copies the string
//    newNode->name = strdup(name);
//    newNode->cmdLine = strdup(cmd);
//    newNode->next = (struct AliasNode *) aliasList;
//    aliasList = newNode;
//    aliasCount++;
//}
//void printAliases() {
//    AliasNode *current = aliasList;
//    printf("Aliases list:\n");
//    while (current != NULL) {
//        printf("alias %s='%s'\n", current->name, current->cmdLine);
//        current = (AliasNode *) current->next;
//    }
//}
//void parseAlias(char *cmd, char **argv,int argCount) {
//    char *delim = "=";
//    char *token = strtok(cmd, delim);
//    while (token != NULL) {
//        argv[argCount] = token;
//        argCount++;
//        token = strtok(NULL, delim);
//    }
//    argv[argCount+1] = NULL;
//    if (argCount == 1 ) {
//        printAliases();
//    }else if (argCount == 2) {
//        defAlias(argv[0], argv[1]);
//        activeAliases = aliasCount;
//    } else {
//        perror("ERR");
//    }
//}
//int executeAliases(char** argv) {
//    for (int i = 0; i < aliasCount; i++) {
//        if (strcmp(argv[0], aliasList[i].cmdLine) == 0) {
//            // Found the alias, replace the command
//            char *aliasedCommand = aliasList[i].cmdLine;
//
//            // Tokenize the aliased command
//            char *token;
//            char *newArgv[MaxArg ];
//            int newArgCount = 0;
//
//            token = strtok(aliasedCommand, " ");
//            while (token != NULL && newArgCount < MaxArg) {
//                newArgv[newArgCount++] = token;
//                token = strtok(NULL, " ");
//            }
//            newArgv[newArgCount] = NULL;
//
//            // Execute the aliased command
//            execvp(newArgv[0], newArgv);
//
//            perror("ERR");
//            return -1;
//
//        }
//    }
//    return 0;
//}
///*void cd(char *path) {
////    if (path == NULL || strcmp(path, "") == 0) {
////        path = getenv("home");
////        if (path == NULL) {
////            perror("cd: home environment variable not set\n");
////            return;
////        }
////    }
////
////    if (chdir(path) != 0) {
////        perror("cd");
////    }
////}*/
//
///*
// *  implementing a signal handler for SIGCHLD. This signal is sent to a parent process when a child process terminates.
// *  By handling this signal, we can keep track of background processes and reset variables accordingly.
// *
// * */
//
//void cmdExecution(char **argv) {
//    pid_t PID = fork();
//    if (PID == -1) {
//        perror("ERR");
//        ErrorFlag = 1;
//        exit(EXIT_FAILURE);
//    }
//    else if (PID == 0) { // Child process
//        if (executeAliases(argv) == 0) {
//            execvp(argv[0], argv);
//            perror("ERR");
//            usleep(100000);
//            exit(EXIT_FAILURE);
//        }
//        else {
//            exit(EXIT_FAILURE);
//        }
//    }
//    else { // Parent process
//
//        usleep(100000);
//        if(!background) {
//            int status;
//            wait(&status); // Wait for the child process to complete
//
//            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
//                (numOfCmd)++; // Increment only if the command executed successfully
//                ErrorFlag = 0; // Indicate success
//            } else {
//                ErrorFlag = 1; // Indicate error
//            }
//
//        }else{
//            jobsCounter++;
//            printf("[%d] %d\n", jobsCounter, PID);
//        }
//    }
//}
//void checkFunctions(char **argv, int argCount) {
//     if (strcmp(argv[0], "source") == 0) {
//        if (argCount < 2) {
//            fprintf(stderr, "source: too few arguments\n");
//            usleep(100000);
//        } else {
//            if (strlen(argv[1]) < 3 || strcmp(argv[1] + strlen(argv[1]) - 3, ".sh") != 0) {
//                perror("ERR: the file doesn't end with .sh\n");
//                usleep(100000);
//            } else {
//                numOfCmd++;
//                executeScriptFile(argv[1]);
//            }
//        }
//    }
//     else if (strcmp(argv[0], "exit_shell") == 0) {
//        return;
//    }
//     else {
//         cmdExecution(argv);
//    }
//}
//void orOperator(char *cmd) {
//    char *delim = "||";
//    char *token;
//    char *commands[MaxArg + 1];
//    int cmdCount = 0;
//
//    token = strtok(cmd, delim);
//    while (token != NULL) {
//        commands[cmdCount] = token;
//        cmdCount++;
//        token = strtok(NULL, delim);
//        if (cmdCount > 3) {
//            perror("ERR");
//            exit(EXIT_FAILURE);
//        }
//    }
//    ErrorFlag=0;
//    for (int i = 0; i < cmdCount ; i++) {
//        handleCmd(commands[i],'"');
//    }
//}
//void andOperator(char *cmd) {
//    char *delim = "&&";
//    char *token;
//    char *commands[MaxArg + 1];
//    int cmdCount = 0;
//
//    token = strtok(cmd, delim);
//    while (token != NULL) {
//        commands[cmdCount] = token;
//        cmdCount++;
//        token = strtok(NULL, delim);
//        if (cmdCount > 3) {
//            perror("ERR");
//            exit(EXIT_FAILURE);
//        }
//    }
//
//    for (int i = 0; i < cmdCount && ErrorFlag != 1; i++) {
//        handleCmd(commands[i], '"');
//    }
//    ErrorFlag = 0;
//
//    if (strstr(commands[1], "||")) {
//        cmdCount = 0;
//        token = strtok(commands[1], "||");
//        while (token != NULL) {
//            commands[cmdCount] = token;
//            cmdCount++;
//            token = strtok(NULL, delim);
//        }
//        orOperator(commands[1]);
//    }
//}
//void handleCmd(char *cmd, char ch) {
//    //first I should remove the \n
//    if (strlen(cmd) > 0 && cmd[(strlen(cmd)) - 1] == '\n') {
//        cmd[strlen(cmd) - 1] = '\0'; // Remove trailing newline character
//    }
//    if (strlen(cmd) > MaxCmdLen) {
//        perror("ERR");
//        return;
//    }
//
//    if (strstr(cmd, "&&")) {
//        andOperator(cmd);
//        return;
//    }
//    if (strstr(cmd, "||")) {
//        orOperator(cmd);
//        return;
//    }
//    if (cmd[strlen(cmd) - 1] == '&') {
//        background = 1;
//        cmd[strlen(cmd) - 1] = '\0';  // Remove the '&' character
//    }
//    char *delim = " =";
//    char *token = strtok(cmd, delim);
//    char **argv = (char **) malloc((MaxArg + 1) * sizeof(char *));
//
//    if (argv == NULL) {
//        perror("ERR");
//        return;
//    }
//
//    int argCount = 0;
//
//    if (token != NULL && strcmp(token, "alias") == 0) {
//        (numOfCmd)++;
//        // Remove single quotes from the command
//        char *command = strtok(NULL, "");
//        if (command != NULL) {
//            char *ptr = strchr(command, '\'');
//
//            while (ptr != NULL) {
//                memmove(ptr, ptr + 1, strlen(ptr));
//                ptr = strchr(ptr, '\'');
//            }
//            strcpy(cmd,command);
//        }
//        parseAlias(cmd,argv,argCount);
//    }
//    else if(token != NULL && strcmp(token, "unalias") == 0) {
//        while (token != NULL) {
//            argv[argCount++] = token;
//            token = strtok(NULL, " ");
//        }
//        argv[argCount] = NULL;
//        if (argCount == 2) {
//            removeAlias(argv[1]);
//            activeAliases = aliasCount;
//            (numOfCmd)++;
//            return;
//        }
//        else {
//            perror("ERR");
//        }
//    }
////    if (token != NULL && strcmp(token, "echo") == 0) {
////        char *message = cmd + 5;
////
////        // Check if the message is enclosed in quotes
////        if (message[0] == '"' && message[strlen(message) - 1] == '"') {
////            // Remove the quotes
////            message[strlen(message) - 1] = '\0';
////            message++;
////        }
////        (numOfCmd)++;
////        printf("%s\n", message);
////    }
//    else {
//        while (token != NULL) {
//            if (argCount <= MaxArg) {
//                argv[argCount] = token;
//                argCount++;
//            }
//            else {
//                perror("ERR");
//                break;
//            }
//            quotesNum += pairsOfQuotes(token, ch);
//            token = strtok(NULL, delim);
//        }
//
//        argv[argCount] = NULL;
//    }
//
//    if (argCount == 0) {
//        free(argv);
//        return;
//    }
//    if(searchForExit(argv)){
//        printf("The number of quotes is: %d\n", quotesNum);
//        exit(0);
//    }
//    checkFunctions(argv, argCount);
//    free(argv);
//}
//
//void executeScriptFile(const char *fileName) {
//    FILE *fp = fopen(fileName, "r");
//    if (fp == NULL) {
//        perror("ERR");
//        return;
//    }
//
//    char line[MaxCmdLen];
//    char ch = '"';
//
//    // Read the first line to check for #!/bin/bash
//    if (fgets(line, sizeof(line), fp)) {
//        // Remove trailing newline character if any
//        if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
//            line[strlen(line) - 1] = '\0';
//        }
//
//        if (strcmp(line, "#!/bin/bash") != 0) {
//            perror("ERR: No #!/bin/bash\n");
//            fclose(fp);
//            return;
//        }
//
////        handleCmd(line, numOfCmd, activeAliases, scriptLines, &quotesNum, ch);
//        (scriptLines)++;
//
//    }
//    else {
//        printf("Error reading first line of script file\n");
//        fclose(fp);
//        return;
//    }
//
//    // Process remaining lines
//    while (fgets(line, sizeof(line), fp)!=NULL||!feof(fp)){
//        // Remove trailing newline character if any
//        if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
//            line[strlen(line) - 1] = '\0';
//        }
//        if (line[0] != '#') {
//            handleCmd(line,ch);
//        }
//        (scriptLines)++;
//    }
//
//    if (ferror(fp)) {
//        perror("ERR");
//    }
//
//    fclose(fp);
//}*/
/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

#define MaxCmdLen 1024
#define MaxArg 5

static int numOfCmd=0, activeAliases=0, scriptLines=0, quotesNum=0, ErrorFlag=0, background=0;
volatile int jobsCounter=0;

typedef struct AliasNode {
    char *name;
    char *cmdLine;
    struct AliasNode *next;
} AliasNode;

typedef struct {
    char cmd[MaxCmdLen];
} Job;

AliasNode *aliasList=NULL;
int aliasCount=0;
Job jobList[10];

void displayPrompt();
void handleCmd(char *cmd, char ch);
int pairsOfQuotes(char *token, char ch);
void cmdExecution(char **argv, char *errFile);
void processOperators(char *cmd);
void handle_background(char *cmd);
void displayJobs();
void printAliases();
void removeAlias(char *name);
void defAlias(char *name, char *cmd);
void parseAlias(char *cmd, char **argv, int argCount);
int executeAliases(char **argv);
bool searchForExit(char* argv[]);
void checkFunctions(char *argv[], int argCount);
void executeScriptFile(const char *fileName);

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
        if (strcmp(cmd, "jobs\n") == 0) {
            displayJobs();
            continue;
        }
        handleCmd(cmd, ch);
    }

    free(cmd);
    AliasNode *current = aliasList;
    while (current != NULL) {
        AliasNode *next = (AliasNode *)current->next;
        free(current->name);
        free(current->cmdLine);
        free(current);
        current = next;
    }
    return 0;
}

void displayPrompt() {
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, activeAliases, scriptLines);
}

void handleCmd(char *cmd, char ch) {
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
    if (strstr(cmd, "&")) {
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
    char *errorFile = NULL;

    int argCount = 0;
    while (token != NULL) {
        if (strcmp(token, "2>") == 0) {
            token = strtok(NULL, delim);
            if (token != NULL) {
                errorFile = token;
            }
            break;
        }
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
    if (searchForExit(argv)) {
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

void processOperators(char *cmd) {
    char *token;
    char *commands[MaxArg + 1];
    int cmdCount = 0;

    token = strtok(cmd, "||");
    while (token != NULL) {
        commands[cmdCount++] = token;
        token = strtok(NULL, "||");
        if (cmdCount > MaxArg) {
            perror("ERR");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < cmdCount; i++) {
        if (!strstr(commands[i], "&&")) {
            handleCmd(commands[i], '"');
        } else {
            char *tmpToken;
            char *tmpCommands[MaxArg + 1];
            int tmpCmdCount = 0;

            tmpToken = strtok(commands[i], "&&");
            while (tmpToken != NULL) {
                tmpCommands[tmpCmdCount++] = tmpToken;
                tmpToken = strtok(NULL, "&&");
            }

            for (int j = 0; j < tmpCmdCount; j++) {
                handleCmd(tmpCommands[j], '"');
                if (ErrorFlag) {
                    break;
                }
            }

            if (ErrorFlag) {
                break;
            }
        }
        ErrorFlag = 0;
    }
}

void cmdExecution(char **argv, char *errFile) {
    pid_t PID = fork();
    if (PID == -1) {
        perror("ERR");
        exit(EXIT_FAILURE);
    } else if (PID == 0) {
        if (errFile != NULL) {
            int fd = open(errFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("ERR");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        if (executeAliases(argv) == 0) {
            execvp(argv[0], argv);
            perror("ERR");
            usleep(100000);
            exit(EXIT_FAILURE);
        } else {
            exit(EXIT_FAILURE);
        }
    } else {
        if (!background) {
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                numOfCmd++;
                ErrorFlag = 0;
            } else {
                ErrorFlag = 1;
            }
        } else {
            jobsCounter++;
            printf("[%d] %d\n", jobsCounter, PID);
        }
    }
}

void handle_background(char *cmd) {
    strcpy(jobList[jobsCounter].cmd, cmd);
    cmd[strlen(cmd) - 1] = '\0';

    char *delim = " =";
    char *token = strtok(cmd, delim);
    char **argv = (char **)malloc((MaxArg + 1) * sizeof(char *));

    if (argv == NULL) {
        perror("ERR");
        return;
    }
    char *errFile = NULL;

    int argCount = 0;
    while (token != NULL) {
        if (strcmp(token, "2>") == 0) {
            token = strtok(NULL, delim);
            if (token != NULL)
                errFile = token;
            break;
        }
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
    background = 1;
    cmdExecution(argv, errFile);
    background = 0;
    free(argv);
}

void displayJobs() {
    for (int i = 0; i < jobsCounter; i++) {
        printf("[%d]   Running               %s\n", i + 1, jobList[i].cmd);
    }
}

void printAliases() {
    AliasNode *current = aliasList;
    printf("Aliases list:\n");
    while (current != NULL) {
        printf("alias %s='%s'\n", current->name, current->cmdLine);
        current = (AliasNode *)current->next;
    }
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
        current = (AliasNode *)current->next;
    }
}

void defAlias(char *name, char *cmd) {
    AliasNode *newAlias = (AliasNode *)malloc(sizeof(AliasNode));
    newAlias->name = strdup(name);
    newAlias->cmdLine = strdup(cmd);
    newAlias->next = aliasList;
    aliasList = newAlias;
    aliasCount++;
}

void parseAlias(char *cmd, char **argv, int argCount) {
    if (argCount == 0) {
        printAliases();
        return;
    } else if (argCount == 1) {
        removeAlias(argv[0]);
    } else if (argCount == 2) {
        defAlias(argv[0], argv[1]);
    } else {
        perror("ERR");
    }
}

int executeAliases(char **argv) {
    AliasNode *current = aliasList;
    while (current != NULL) {
        if (strcmp(current->name, argv[0]) == 0) {
            char *aliasCmd = strdup(current->cmdLine);
            char *token = strtok(aliasCmd, " ");
            char **aliasArgv = (char **)malloc((MaxArg + 1) * sizeof(char *));
            if (aliasArgv == NULL) {
                perror("ERR");
                free(aliasCmd);
                return 1;
            }

            int aliasArgCount = 0;
            while (token != NULL) {
                if (aliasArgCount <= MaxArg) {
                    aliasArgv[aliasArgCount] = token;
                    aliasArgCount++;
                } else {
                    perror("ERR");
                    break;
                }
                token = strtok(NULL, " ");
            }
            aliasArgv[aliasArgCount] = NULL;

            int result = execvp(aliasArgv[0], aliasArgv);
            free(aliasArgv);
            free(aliasCmd);
            return result;
        }
        current = current->next;
    }
    return 0;
}

bool searchForExit(char* argv[]) {
    if (strcmp(argv[0], "exit_shell") == 0) {
        printf("The number of quotes is: %d\n", quotesNum);
        exit(0);
    }
    return false;
}

void checkFunctions(char *argv[], int argCount) {
    if (strcmp(argv[0], "alias") == 0) {
        parseAlias(argv[1], argv + 1, argCount - 1);
    } else if (strcmp(argv[0], "script") == 0) {
        executeScriptFile(argv[1]);
    } else {
        cmdExecution(argv, NULL);
    }
}

void executeScriptFile(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("ERR");
        return;
    }

    char line[MaxCmdLen];
    while (fgets(line, sizeof(line), file)) {
        handleCmd(line, '"');
        scriptLines++;
    }

    fclose(file);
}
*/