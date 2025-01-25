#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "header.h"


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

int executeCommand(char** tokens){

    // Create a new process
    pid_t pid = fork(); // process id 

    if(pid == -1){
        perror("process creation failed");
        return -1;
    }
    else if(pid == 0){
        // CHILD process
        execvp(tokens[0], tokens); // take first token and replace it with the command and take rest of tokens as arguments
        perror("error message if it doesnt execute"); // if the above does not run print an error
        exit(1); // exit function with status 1 to terminate processes
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
