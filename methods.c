#include <string.h>
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