
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


#define MaxCmdLen 1024
#define MaxArg 4

static int numOfCmd = 0, quotesNum = 0,ErrorFlag =0,
        background=0, jobsCounter = 0;

typedef struct {
    char cmd[MaxCmdLen];
} Job;

Job jobList[10];

void displayPrompt();
void handleCmd(char *cmd, char ch);
int pairsOfQuotes(char *token, char ch);
void cmdExecution(char **argv,char *errFile);
void processOperators(char *cmd);
void handle_background(char *cmd);
void displayJobs();

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
    return 0;
}

void displayJobs() {
    for (int i = 0; i < jobsCounter; i++) {
        printf("[%d]   Running               %s\n", i+1,  jobList[i].cmd);
    }
}

void displayPrompt() {
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, 0, 0);
}

void handleCmd(char *cmd, char ch) {
    //remove trailing newline character
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

    if (strcmp(argv[0], "exit_shell") == 0) {
        free(argv);
        return;
    }
    cmdExecution(argv,errorFile);
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

    // First, tokenize by '||'
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
        if (!strstr(commands[i], "&&")) { // Command without '&&'
            handleCmd(commands[i], '"');
        }
        else { // Command with '&&'
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


void cmdExecution(char **argv,char *errFile) {
    pid_t PID = fork();
    if (PID == -1) {
        perror("ERR");
        exit(EXIT_FAILURE);
    }
    else if (PID == 0) { // Child process
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
                (numOfCmd)++; //increment only if the command executed successfully
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

void handle_background(char *cmd) {
    strcpy(jobList[jobsCounter].cmd,cmd);
    // Remove the '&' character from the command
    cmd[strlen(cmd) - 1] = '\0';

    char *delim = " =";
    char *token = strtok(cmd, delim);
    char **argv = (char **)malloc((MaxArg + 1) * sizeof(char *));

    if (argv == NULL) {
        perror("ERR");
        return;
    }

    
    char *errFile=NULL;

    int argCount = 0;
    while (token != NULL) {
        if (strcmp(token,"2>") == 0){
            token= strtok(NULL,delim);
            if(token!=NULL)
                errFile=token;
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

    if (strcmp(argv[0], "exit_shell") == 0) {
        free(argv);
        return;
    }

    background = 1;
    cmdExecution(argv, errFile);
    background = 0; // Reset background flag
    free(argv);
}