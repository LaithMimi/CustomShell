#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 4

typedef struct{
    char *name;
    char *cmdLine;
}Alias;

Alias aliasArr[100];//Storage for aliased cmds
int aliasCount=0;

void defAlias(char *name, char *cmdLine){//add alias
    for(int i=0;i<aliasCount;i++){ //to check if the cmd already exists in aliasing array
        if(strcmp(aliasArr[i].name,name)==0){
            perror("This command already exists");
            return ;
        }
    }
    //if the cmd is new:
    aliasArr[aliasCount].name=name;
    aliasArr[aliasCount].cmdLine=cmdLine;
    aliasCount++;
}
void deleteAlias(char *name){
    bool found =false;
    for(int i=0; i< aliasCount;i++) {
        if (found) {
            aliasArr[i - 1] = aliasArr[i];
        } else if (strcmp(aliasArr[i].name, name) == 0) {
            free(aliasArr[i].name);
            free(aliasArr[i].cmdLine);
            found = true;
        }
    }
    if(found){
        aliasCount--;
    }
    else
        perror("Alias Doesn't Exist");

}
int execute_with_aliases(char* cmd[]) { //checks if the command matches any alias and replaces it.
    for (int i = 0; i < aliasCount; i++) {
        if (strcmp(cmd[0], aliasArr[i].name) == 0) {
            // Alias match - replace command
            printf("Alias match: %s -> %s\n", aliasArr[i].name, aliasArr[i].cmdLine);
            cmd[0] = aliasArr[i].cmdLine;
            break;
        }
    }

        // Execute command
        return execvp(cmd[0], cmd);
}

void displayPrompt(int numOfCmd,int ActiveAliases,int ScriptLines){
    printf("#cmd:%d|#alias:%d|#script lines:%d>",numOfCmd,ActiveAliases,ScriptLines);
}


int main() {
    char *cmd = (char *) malloc(MAX_CMD_LEN * sizeof(char));
    char *argv[MAX_ARGS + 1]; //+1 for the NULL terminator

    const char *delim = " \t\n";

    int numOfCmd = 0;

    int ActiveAliases = 0;
    int ScriptLines = 0;

    pid_t PID;
        if(cmd==NULL){
            perror("Error: Memory Allocation failed");
            return 1;
        }
    while (1) {
        displayPrompt(numOfCmd, ActiveAliases, ScriptLines);

                                /*READ COMMANDS*/
        if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
            perror("Error reading input");
            continue;
        }
        // Remove trailing newline character if any
        if (strlen(cmd) > 0 && cmd[strlen(cmd) - 1] == '\n') {
            cmd[strlen(cmd) - 1] = '\0';
        }

        if (strcmp(cmd, "exit") == 0) {
            //free(cmd);
            break;
        }
                                            /*PARSE COMMANDS*/
        char *token = strtok(cmd, delim);
        int arg_count = 0;
        //loop that reads input and then parses it into tokens,stores them in an cmd_array up to a maximum count
        while (token != NULL) {
                if (arg_count <= MAX_ARGS) {
                    argv[arg_count] = token;
                    arg_count++;
                }else{
                    fprintf(stderr, "Illegal Command: Too Many Arguments\n");
                    arg_count = 0; // Reset argument count to avoid partially filled argv
                    break;
                }
                token = strtok(NULL, delim);
            }
            // Null-terminate the argument list
            argv[arg_count]=NULL;
            if(arg_count==0)  //continue if no cmd found
                continue;

            ScriptLines++; //MAYBE WRONG HERE ....

            if (arg_count > MAX_ARGS) {
                perror("Illegal Command: Too Many Arguments\n");
                continue;
            }
            if (strlen(cmd) > MAX_CMD_LEN) {
                perror("Illegal Command: Too Many Characters\n");
                continue;
            }

                                    /*Aliased commands*/
           if(strcmp(argv[0],"alias")==0){
               if(arg_count==3){//remember max arguments are 4, the bash command take one so there is 3 arguments left
               defAlias(argv[1],argv[2]);
               ActiveAliases=aliasCount;
               }else{
                   perror("Usage: alias name command\n");
               }
               continue;

           }
           else if (strcmp(argv[0], "unalias") == 0){
               if (arg_count==2){
                   deleteAlias(argv[1]);
                   ActiveAliases=aliasCount;
               }
               else{
                   perror("Usage: alias name command\n");
               }
               continue;

           }

           int res=execute_with_aliases(argv);
           if (res == 0) {
            numOfCmd++;
            }
           else {
            printf("\nError executing command %s\n", argv[0]);
            }

                                    /*PROCESSES*/
            PID = fork();
            if (PID == -1) {
                perror("Error: fork failed");
                free(cmd);
                exit(EXIT_FAILURE);;
            }
            else if (PID == 0) { //child process
                                            /*EXECUTE COMMANDS*/
            if (execvp(argv[0], argv) == -1) {
                fprintf(stderr, "\nError executing command %s\n", &cmd[0]);
                continue;
            }
            if (strcmp(cmd, "exit") != 0) {
                perror("Error: Command not found");
                continue;
            }
            }
            else { //parent process

                int status;
                if (waitpid(PID, &status, 0) < 0) {
                    perror("Error: waitpid");
                    free(cmd);
                    continue;
                }

                //check return status
                if (WEXITSTATUS(status) != 0)
                return 1;
            numOfCmd++;
        }

    }
    free(cmd);
    for (int i = 0; i < aliasCount; i++) {
        free(aliasArr[i].name);
        free(aliasArr[i].cmdLine);
    }

    return 0;
}
