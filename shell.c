#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#define MaxCmdLen 1024
#define MaxArg 5
#define MaxJobs 100

static int numOfCmd = 0, activeAliases = 0, scriptLines = 0, quotesNum = 0,
            ErrorFlag = 0, background = 0, jobsCounter = 0;
pid_t pid;

typedef struct AliasNode {
    char *name;
    char *cmdLine;
    struct AliasNode *next;
} AliasNode;
AliasNode *aliasList = NULL;
int aliasCount = 0;


typedef struct {
    pid_t pid;
    char cmd[MaxCmdLen];
    int active;
} Job;
Job jobList[MaxJobs];

void displayPrompt();
void handleCmd(char *cmd, char ch);
int pairsOfQuotes(char *token, char ch);
void parseAlias(char *cmd, char **argv, int argCount);
void cmdExecution(char **argv,char *errFile,char *originalCmd);
void executeScriptFile(const char *fileName);
void addJobs(char *cmd);
void handle_background(char *cmd);
void processOperators(char *cmd);
void deleteJobs();
void checkJobs();
void displayJobs();
bool searchForExit(char* argv[]);
void defAlias(char *name, char *cmd);
void printAliases();
void removeAlias(char *name);
int executeAliases(char **argv);

int main() {
    char *cmd = (char *)malloc(MaxCmdLen * sizeof(char));
    char ch = '"';

    if (cmd == NULL) {
        perror("Error: Memory Allocation failed");
        return 1;
    }

    while (1) {
        checkJobs();
        deleteJobs();
        displayPrompt();
        if (fgets(cmd, MaxCmdLen, stdin) == NULL) {
            perror("ERR");
            free(cmd);
            return 1;
        }

        if (strcmp(cmd, "exit_shell\n") == 0) {
            printf("%d\n", quotesNum);
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
        AliasNode *next = (AliasNode *) current->next;
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

void defAlias(char *name, char *cmd) {
    AliasNode *current = aliasList;

    while (current != NULL) { // Check for existing alias
        if (strcmp(current->name, name) == 0) {
            // Found existing alias, override
            free(current->cmdLine); // Free memory of old command
            current->cmdLine = strdup(cmd); // Assign new command
            return; // No need to create a new node
        }
        current = current->next;
    }

    AliasNode *newNode = (AliasNode *)malloc(sizeof(AliasNode));
    if (newNode == NULL) {
        perror("ERR");
        exit(EXIT_FAILURE);
    }
    // strdup allocates and copies the string
    newNode->name = strdup(name);
    newNode->cmdLine = strdup(cmd);
    newNode->next = (struct AliasNode *) aliasList;
    aliasList = newNode;
    aliasCount++;
    activeAliases = aliasCount;
}

void printAliases() {
    AliasNode *current = aliasList;
    printf("Aliases list:\n");
    while (current != NULL) {
        printf("alias %s='%s'\n", current->name, current->cmdLine);
        current = (AliasNode *) current->next;
    }
}

void parseAlias(char *cmd, char **argv, int argCount) {
    char *delim = "=";
    char *token = strtok(cmd, delim);
    while (token != NULL) {
        argv[argCount] = token;
        argCount++;
        token = strtok(NULL, delim);
    }
    argv[argCount+1] = NULL;
    if (argCount == 1) {
        printAliases();
    } else if (argCount == 2) {
        defAlias(argv[0], argv[1]);
        activeAliases = aliasCount;
    } else {
        perror("ERR");
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
        current = current->next;
    }
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

int executeAliases(char** argv) {
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(argv[0], aliasList[i].cmdLine) == 0) {
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

            perror("ERR");
            return -1;

        }
    }
    return 0;
}

void deleteJobs() {
    int counter = 0;
    for (int i = 0; i < jobsCounter; i++) {
        // Move active jobs to the front of the list
        if (jobList[i].active) {
            jobList[counter++] = jobList[i];
        }
    }
    jobsCounter = counter;
}

void checkJobs() {
    for (int i = 0; i < jobsCounter; i++) {
        if (jobList[i].active) {
            int status;
            pid_t result = waitpid(jobList[i].pid, &status, WNOHANG);
            if (result == 0) {// Child is still running
                //printf("%s is still running\n",jobList[i].cmd);
                continue;
            } else if (result > 0) { // Child has finished
                //printf("%s has ended\n",jobList[i].cmd);
                jobList[i].active = 0;
            }
        }
    }
}

void displayJobs() {
    for (int i = 0; i < jobsCounter; i++) {
        printf("[%d]   Running               %s\n", i+1,  jobList[i].cmd);
    }
}

void addJobs(char *cmd) {
    strcat(cmd, "&"); // Concatenate "&" to the end of the cmd string
    if (jobsCounter < MaxJobs) {
        jobList[jobsCounter].pid = pid;
        strncpy(jobList[jobsCounter].cmd, cmd, MaxCmdLen);
        jobList[jobsCounter].active = 1;
        jobsCounter++;
    } else {
        fprintf(stderr, "Too many background jobs\n");
    }
}

void handle_background(char *cmd) {
    cmd[strlen(cmd) - 1] = '\0'; // Remove the '&' character
    char *newCmd = cmd;
    background = 1;
    handleCmd(newCmd, '"');
    background = 0;
}

void processOperators(char *cmd) {
    char *commands[3];
    int cmdCount = 0;

    char *cmds[3];
    int counter = 0;

    char *start;
    char *pos;

    char *token;
    token = strtok(cmd, "||");
    while (token != NULL) {
        commands[cmdCount++] = token;
        token = strtok(NULL, "||");
        if (cmdCount > 3) {
            perror("ERR");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < cmdCount; i++) {
        if (strstr(commands[i], "&&")) { //ls && sleep 5 && sleep 10&
            pos = commands[i];
            start = commands[i];
            while (*pos) {
                if (strncmp(pos, "&&", 2) == 0) {
                    *pos = '\0';  // Null terminate the current command
                    cmds[counter++] = start;
                    pos += 2;  // Skip over the "&&"
                    start = pos;
                } else {
                    pos++;
                }
                if (counter > 3) {
                    perror("ERR");
                    exit(EXIT_FAILURE);
                }
            }
            cmds[counter++] = start;
            //cmdCount=counter;
            for (int j = 0; j < counter; j++) {
                char *tmpCmd = cmds[j];

                while (*tmpCmd == ' ') tmpCmd++;
                char *end = tmpCmd + strlen(tmpCmd) - 1;
                while (end > tmpCmd && *end == ' ') end--;
                *(end + 1) = '\0';

                // Check if there's an '&' in the command
                if (strchr(tmpCmd, '&') != NULL && strchr(tmpCmd, '&') == strrchr(tmpCmd, '&')) {
                    handle_background(tmpCmd);
                    if (ErrorFlag) {
                        break;
                    }
                } else {
                    handleCmd(cmds[j], '"');
                    if (ErrorFlag) {
                        break;
                    }
                }
            }
        }
        else {
            char *tmpCmd = commands[i];
            while (*tmpCmd == ' ') tmpCmd++;
            char *end = tmpCmd + strlen(tmpCmd) - 1;
            while (end > tmpCmd && *end == ' ') end--;
            *(end + 1) = '\0';
            // Check if there's an '&' in the command
            if (strchr(tmpCmd, '&') != NULL && strchr(tmpCmd, '&') == strrchr(tmpCmd, '&')) {
                handle_background(tmpCmd);
            }
            else {
                handleCmd(commands[i], '"');
            }
        }
        ErrorFlag = 0;
    }
}

void handleCmd(char *cmd, char ch) {
    char originalCmd[MaxCmdLen];
    strncpy(originalCmd, cmd, MaxCmdLen);
    //first I should remove the \n
    if (strlen(cmd) > 0 && cmd[(strlen(cmd)) - 1] == '\n') {
        cmd[strlen(cmd) - 1] = '\0'; // Remove trailing newline character
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
    char **argv = (char **) malloc((MaxArg + 1) * sizeof(char *));

    if (argv == NULL) {
        perror("ERR");
        return;
    }
    char *errorFile = NULL;
    int argCount = 0;

    if (token != NULL && strcmp(token, "alias") == 0) {
        (numOfCmd)++;
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
        parseAlias(cmd,argv,argCount);
        return;
    }
    else if(token != NULL && strcmp(token, "unalias") == 0) {
        while (token != NULL) {
            argv[argCount++] = token;
            token = strtok(NULL, " ");
        }
        argv[argCount] = NULL;
        if (argCount == 2) {
            removeAlias(argv[1]);
            activeAliases = aliasCount;
            (numOfCmd)++;
            return;
        }
        else {
            perror("ERR");
        }
    }
    if (strcmp(token, "source") == 0) {
        if (argCount < 2) {
            fprintf(stderr, "source: too few arguments\n");
            usleep(100000);
        } else {
            if (strlen(argv[1]) < 3 || strcmp(argv[1] + strlen(argv[1]) - 3, ".sh") != 0) {
                perror("ERR: the file doesn't end with .sh\n");
                usleep(100000);
            } else {
                numOfCmd++;
                executeScriptFile(argv[1]);
            }
        }
    }
    else {
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
            }
            else {
                perror("ERR");
                break;
            }
            quotesNum += pairsOfQuotes(token, ch);
            token = strtok(NULL, delim);
        }

        argv[argCount] = NULL;
    }

    if (argCount == 0) {
        free(argv);
        return;
    }
    if(searchForExit(argv)){
        printf("%d\n", quotesNum);
        exit(0);
    }
    cmdExecution(argv,errorFile,originalCmd);
    free(argv);
}

void cmdExecution(char **argv,char *errFile,char *originalCmd) {
    pid_t PID= fork();
    pid=PID;
    if (PID == -1) {
        perror("ERR");
        exit(EXIT_FAILURE);
    }
    else if (PID == 0) {
        if (errFile != NULL) {
            int fd;
            //O_TRUNC is used to truncate the file to a length of zero if it already exists.
            //O_CREAT This flag is used to create a new file if it does not already exist.
            //The file permissions are set to rw-r--r-- 0644
            fd= open(errFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);//open errFile for writing,
            if (fd == -1) {
                perror("ERR");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDERR_FILENO);//redirect stdout to errFile
            close(fd); //close File descriptor
        }
        if (executeAliases(argv) == 0) {
            execvp(argv[0], argv);
            perror("ERR");
            usleep(100000);
            exit(EXIT_FAILURE);
        }
        else {
            exit(EXIT_FAILURE);
        }
    }
    else {
        if(background==0) {
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                (numOfCmd)++; //increment only if the command executed successfully
                ErrorFlag = 0;
            }
            else {//Child Failed
                ErrorFlag = 1;
            }

        }
        else{
            addJobs(originalCmd);
            (numOfCmd)++;
            printf("[%d] %d\n", jobsCounter , PID);
        }
    }
}

void executeScriptFile(const char *fileName) {
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        perror("ERR");
        return;
    }

    char line[MaxCmdLen];
    char ch = '"';

    // Read the first line to check for #!/bin/bash
    if (fgets(line, sizeof(line), fp)) {
        // Remove trailing newline character if any
        if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        if (strcmp(line, "#!/bin/bash") != 0) {
            perror("ERR: No #!/bin/bash\n");
            fclose(fp);
            return;
        }

//        handleCmd(line, numOfCmd, activeAliases, scriptLines, &quotesNum, ch);
        (scriptLines)++;

    }
    else {
        printf("Error reading first line of script file\n");
        fclose(fp);
        return;
    }

    // Process remaining lines
    while (fgets(line, sizeof(line), fp)!=NULL||!feof(fp)){
        // Remove trailing newline character if any
        if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        if (line[0] != '#') {
            handleCmd(line,ch);
        }
        (scriptLines)++;
    }

    if (ferror(fp)) {
        perror("ERR");
    }

    fclose(fp);
}