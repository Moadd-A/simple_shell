#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "header.h"
#include <errno.h>

char* tokens[512];

char** parse(char input [512]){
        int i = 1;
        char* token = strtok(input, " \n\t|<>;&");
        tokens[0] = token;  // get the first token
        
        
        while(token != NULL)
        {  // iterate through extracting the token
            token = strtok(NULL, " \n\t|<>;&"); // signal to continue tokenising from the last position
            tokens[i] = token;
            i++;
        }
        tokens[i] = NULL;

        
    return tokens;
}


void clearBuffer(char* userInput){
      
    int len = strlen(userInput);

    if (len > 0 && userInput[len-1] == '\n') {
            userInput[len-1] = '\0';
    } 
        
        // Clear buffer only if fgets didn't read whole line
    else {
          int ch;
          while((ch = getchar()) != '\n' && ch != EOF);
        }
}

int executeSystemCommand(char** tokens){

    // Create a new process
    pid_t pid = fork(); // process id  // parent and child process

    if(pid == -1){
        perror("Process creation failed\n");
        return -1;
    }
    else if(pid == 0){
        // CHILD process
        if (execvp(tokens[0], tokens) == -1) {  // execvp returns -1 on failure
            perror("Error executing command"); // take first token and replace it with the command and take rest of tokens as arguments
            exit(1);  // exit function with status 1 to terminate processes
        }
    }
    else{
        int status;
        waitpid(pid, &status, 0); // load the status returned from the child process into status
        if(WIFEXITED(status)){ // was the child process terminated normally by exit
            return WEXITSTATUS(status); // What was its exit status
        }

    }
    return 0;
}

void runCommands(char** tokens){
            // Run custom commands that aren't executables, eg getenv
            // If doesn't work either then print error
            if(strcmp(tokens[0], "cd") == 0){ // change directory
                changeDirectory(tokens);
                
            }
            else if (strcmp(tokens[0], "getPath") == 0){
                getPath(tokens);
            }
            else if (strcmp(tokens[0], "setPath") == 0){
                setPath(tokens);
            }
            else{
               executeSystemCommand(tokens); 
            }
        }

void getPath(char** tokens){
    if(tokens[1] != NULL){
        fprintf(stderr, "Error: Too many parameters\n");
    }
    else {
        printf("PATH: %s \n", getenv("PATH")); 
    }
}

void setPath(char** tokens){
    if(1) { //
        return; //
    } // COMMENT THIS OUT WHEN WE WANT TO TEST SETPATH - CURRENTLY DISABLED TO AVOID CHANGING PC PATH


    if(tokens[1] == NULL) {
        fprintf(stderr, "Error: Too few parameters - must include a path to set\n");
        return;
    }
    else if(tokens[2] != NULL) {
        fprintf(stderr, "Error: Too many parameters\n");
        return;
    }
    else {
        setenv("PATH", tokens[1], 1);
    }
}

void changeDirectory(char** tokens){

    if(tokens[1] == NULL) {         // No directory specified, therefore change directory to home
        chdir(getenv("HOME"));              
    }
    else if (tokens[2] != NULL) {
        fprintf(stderr, "Error: Too many parameters\n");
        return;
    } 
    else {    // Once directory is proven to be valid
        if (chdir(tokens[1]) == -1) {
            if(errno == 2) {
                fprintf(stderr, "'%s', no such file or directory\nEnter a valid directory\n", tokens[1]);
            }
            else if(errno == 20){
                fprintf(stderr, "'%s' is not a directory\nenter a valid one\n", tokens[1]);
            }
        }
    }
}

// perror() function for a non existent directory?
// stderror for input/output errors (eg too few/to)?
// Ask in lab when to use each, eg we're using stderror for checking parameters