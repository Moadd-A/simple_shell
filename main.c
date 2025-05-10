#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getcwd function
#include "header.h"
#include "methods.c"


int main(int argc, char *argv[])
{
    
    char *userInput; //user input that we will later parse from the terminal;
    userInput = malloc(sizeof(char) * 512);  // Allocate some memory for the user's command line input
    char cwd[256];               // Current working directory                 
    char *home = getenv("HOME"); // Home directory
    char *path[3] = {NULL, getenv("PATH")}; // Gets the system path
    chdir(home);                 // Set current directory to home, home is the default directory for the shell
    getcwd(cwd, sizeof(cwd));

    loadHistory(".hist_list");
    loadAlias(".aliases");

    printf("$%s> ", cwd);  // Display prompt, displays current directory
    
    while(fgets(userInput, 512, stdin)) // Main program loop
    {
        clearBuffer(userInput); // Clear buffer of excess input and remove newline character at the 1234
        
        char** tokens = parse(userInput);
        
        if(tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) { // first condition checks that we haven't got an empty input
            if(tokens[1] != NULL){ // If exit is in the first token, but we have anoter invalid command
                printf("Too many parameters. Please just type 'exit'\n");
            }
        printf("Exiting shell...\n");
        break;
        }
    
        
        if(tokens[0] != NULL){
            runCommands(tokens, true, 0);       // Figure out if a valid command is being run 
        }
        
        getcwd(cwd, sizeof(cwd));
        printf("$%s> ", cwd);
    }
    
    if (feof(stdin))
    { // Handle Ctrl + D exit
        printf("\nExiting shell...\n");
    }

    free(userInput); // Free allocated memory

    saveHistory(".hist_list");
    saveAlias(".aliases");
    
    // Restores system path regardless of it being changed by user
    setPath(path);  
    // Prints restored system path 
    printf("System path %s restored", path[1]); 
}
