#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getcwd function
#include "header.h"
#include "methods.c"

int main(int argc, char *argv[])
{
    char *userInput; 
    userInput = malloc(sizeof(char) * 512); // Allocate some memory for the user's command line input
    char cwd[256];
    char *home = getenv("HOME"); // Can reduce in future (give straight as argument)
    chdir(home);                 // Set current directory to home, so home is the default directory for the shell
    getcwd(cwd, sizeof(cwd));

    printf("$%s> ", cwd);  // Display prompt, displays current directory
    
    while(fgets(userInput, 512, stdin)) // Main program loop
    {
        clearBuffer(userInput); // Clear buffer of excess input and remove newline character at the end

        if(strcmp(userInput, "exit") == 0 ) // Ensures we can safely leave the main program loop.
        {
            printf("Exiting shell..."); 
            break; // Ends the main program loop.
        }
     
        char** tokens = parse(userInput); //  pointer to pointers of tokenised userinput 

        // for (int i = 0; tokens[i] != NULL; i++) {
        //     printf("\"%s\"\n", tokens[i]);
        // }
        if(tokens[0] != NULL){
            executeCommand(tokens);
        }
        

        printf("$%s> ", cwd); 
        
    }
    free(userInput); // Free allocated memory
}
    
