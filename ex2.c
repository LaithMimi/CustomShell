#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MaxCmdLen 1024
#define MaxArg 4
#define MaxJobs 100

static int numOfCmd = 0, quotesNum = 0,ErrorFlag =0,
        background=0, jobsCounter = 0;
pid_t pid;

typedef struct {
    pid_t pid;
    char cmd[MaxCmdLen];
    int active;
} Job;

Job jobList[MaxJobs];

void checkJobs();
void displayJobs();
void deleteJobs();
void displayPrompt();
void handleCmd(char *cmd, char ch);
int pairsOfQuotes(char *token, char ch);
void processOperators(char *cmd);
void cmdExecution(char **argv,char *errFile,char *originalCmd);
void addJob(char *cmd);
void handle_background(char *cmd);


int main() {
    char *cmd = (char*)malloc(MaxCmdLen * sizeof(char));
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
    return 0;
}

void deleteJobs(){
    int counter= 0;

    for (int i = 0; i < jobsCounter; i++) {
        if (jobList[i].active) {
            // Move active jobs to the front of the list
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

            if (result == 0) {
                // Child is still running
                printf("%s is still running\n",jobList[i].cmd);
                continue;
            } else if (result > 0) {
                // Child has finished
                printf("%s has ended\n",jobList[i].cmd);
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

void displayPrompt() {
    printf("#cmd:%d|#alias:%d|#script lines:%d>", numOfCmd, 0, 0);
}

void handleCmd(char *cmd, char ch) {
    char originalCmd[MaxCmdLen];
    strncpy(originalCmd, cmd, MaxCmdLen);
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
    cmdExecution(argv,errorFile,originalCmd);
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

void cmdExecution(char **argv,char *errFile,char *originalCmd) {
//    if (strcmp(argv[0], "sleep") == 0 && argv[1] != NULL) {
//        (numOfCmd)++;
//        int duration = atoi(argv[1]); //to cast from string to integer
//        sleep(duration);
//        return;
//    }
    pid_t PID = fork();
    pid=PID;
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
        fprintf(stderr, "%s: Command not found.\n",argv[0]);
        usleep(100000);
        exit(EXIT_FAILURE);
    }
    else { // Parent process
        if(background==0) {
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                (numOfCmd)++; //increment only if the command executed successfully
                ErrorFlag = 0;
            } else {
                ErrorFlag = 1;
            }

        }else{
            addJob(originalCmd);
            (numOfCmd)++;
            printf("[%d] %d\n", jobsCounter , PID);
        }
    }
}

void addJob(char *cmd) {
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
    // Remove the '&' character from the command
    cmd[strlen(cmd) - 1] = '\0';
    char *newCmd =cmd;
    background = 1;
    handleCmd(newCmd,'"');
    background = 0; // Reset background flag
}

