#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "header.h"
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>

char* tokens[512];
char** history[MAX_HISTORY_SIZE];
char** alias[MAX_ALIAS];

int historyCounter = 0;
int aliasCounter = 0;

char** parse(char input [512])
{
        int i = 1;
        char* token = strtok(input, " \n\t|<>;&");  // Input delimiters
        tokens[0] = token;  // Get the first token
        
        
        while(token != NULL)
        {  // Iterate through, extracting the token
            token = strtok(NULL, " \n\t|<>;&"); // Signal to continue tokenising from the last position
            tokens[i] = token;
            i++;
        }
        
    return tokens;
}


void clearBuffer(char* userInput)
{
      
    int len = strlen(userInput);

    if (len > 0 && userInput[len-1] == '\n') 
    {
        userInput[len-1] = '\0';
    } 
        
    // Clear buffer only if fgets didn't read whole line
    else 
    {
        int ch;
        while((ch = getchar()) != '\n' && ch != EOF);
    }
}



void runCommands(char** tokens, bool newCommand, int depth){
    // Run custom commands that aren't executables, eg getenv
    // If doesn't work either then print error
    if (isHistoryCommand(tokens)) 
    { 
        runHistoryCommand(tokens);
        newCommand = false;
    }
    else if(isAlias(tokens)) {
        if (!runAlias(tokens, depth)) {  // Try to expand aliases
            newCommand = false;  // If no alias was expanded, keep it as a normal command
        }
    }
    else if(strcmp(tokens[0], "cd") == 0){ 
        changeDirectory(tokens);
    }
    else if (strcmp(tokens[0], "getpath") == 0){
        getPath(tokens);
    }

    else if (strcmp(tokens[0], "setpath") == 0){
        setPath(tokens);
    }
    else if(strcmp(tokens[0], "history") == 0)
    {
        addToHistory(tokens);   // This is so that the command "history" is displayed in the printed history
        newCommand = false;

        printHistory(tokens);
    }
    else if(strcmp(tokens[0], "alias") == 0)
    {
        if(tokens[1] == NULL)
        {
            printAliases();
        }
        else 
        {
            addAlias(tokens);
        }
    }
    else if(strcmp(tokens[0], "unalias") == 0)
    {
        removeAlias(tokens);
    }
    else
    {
       executeSystemCommand(tokens); 
    }

    if(newCommand) 
    {
        addToHistory(tokens); // Everything gets added except for history commands and commands recalled from history
    }
}

int executeSystemCommand(char** tokens)
{

    // Create a new process
    pid_t pid = fork(); // process id  // parent and child process

    if(pid == -1)
    {
        perror("Process creation failed\n");
        return -1;
    }
    else if(pid == 0)
    {
        // CHILD process
        if (execvp(tokens[0], tokens) == -1) {  // execvp returns -1 on failure
            perror(tokens[0]); // Shows the first token as a failed command
            exit(1);  // exit function with status 1 to terminate processes
        }
    }
    else
    {
        int status;
        waitpid(pid, &status, 0); // Load the status returned from the child process into status
        if(WIFEXITED(status))   // Checks if the child process terminated normally by exit
        { 
            return WEXITSTATUS(status); // Returns its exit status
        }

    }
    return 0;
}

void getPath(char** tokens)
{
    if(tokens[1] != NULL)
    {
        fprintf(stderr, "Error: Too many parameters\n");
    }
    else 
    {
        printf("PATH: %s \n", getenv("PATH")); 
    }
}

void setPath(char** tokens){

    if(tokens[1] == NULL) {
        fprintf(stderr, "Error: Too few parameters - must include a path to set\n");
        return;
    } // sizeof(tokens) > (sizeof(tokens[0])*2
    else if(tokens[2] != NULL) {
        fprintf(stderr, "Error: Too many parameters\n");
        return;
    }
    else {
        setenv("PATH", tokens[1], 1);
    }
}

void changeDirectory(char** tokens)
{

    if(tokens[1] == NULL) 
    {         // No directory specified, therefore change directory to home
        chdir(getenv("HOME"));              
    }
    else if (tokens[2] != NULL) 
    {
        fprintf(stderr, "Error: Too many parameters\n");
        return;
    } 
    else 
    {    // Once directory is proven to be valid
        if (chdir(tokens[1]) == -1)         //  chdir returning -1 means it was not successdul
        {       
            if(errno == 2) 
            {
                fprintf(stderr, "'%s', No such file or directory\nEnter a valid file or directory\n", tokens[1]);
            }
            else if(errno == 20)
            {
                fprintf(stderr, "'%s' is not a directory\nEnter a valid directory\n", tokens[1]);
            }
        }
    }
}

bool isHistoryCommand(char** tokens) 
{
    if(tokens[0][0] == '!') 
    {
        return true;
    }
    else
    {
        
    }
    return false;
}

void addToHistory(char** tokens) 
{
    if (historyCounter < MAX_HISTORY_SIZE) 
    {
        // Allocate memory for a new history entry
        history[historyCounter] = malloc(sizeof(char*) * 512);  // Allocate array of string pointers [[malloc],[malloc],[]]
        
        int i;
        for (i = 0; tokens[i] != NULL; i++) 
        {
            history[historyCounter][i] = strdup(tokens[i]);  // Copy each token
        }
        history[historyCounter][i] = NULL;  // Null-terminate the command
        historyCounter++;
    }
    else 
    {
        // Free the oldest entry before shifting
        for (int i = 0; history[0][i] != NULL; i++) 
        {
            free(history[0][i]);  // Free each token
        }
        free(history[0]); // Free the array of token pointers
        
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) 
        {
            history[i] = history[i + 1];
        }

        // Allocate memory for the new command at the last position
        history[MAX_HISTORY_SIZE - 1] = malloc(sizeof(char*) * 512);
        int i;
        for (i = 0; tokens[i] != NULL; i++) 
        {
            history[MAX_HISTORY_SIZE - 1][i] = strdup(tokens[i]);  // Copy each token
        }
        history[MAX_HISTORY_SIZE - 1][i] = NULL;  // Null-terminate
    }
}

void runHistoryCommand(char** tokens) 
{
    memmove(tokens[0], tokens[0] + sizeof(char), strlen(tokens[0])); // copy memory to new space !1
    if(tokens[0][0] == '!' && historyCounter > 0) // case if its !!
    {
        if(tokens[0][0] == '!' && !(tokens[0][1] == '\0')) // edge ase to prevent !!<no>
        {
            printf("Unexpected input following !!. Please enter either !!, !<no> or !-<no>.\n"); // return feedback to user for them attempting this
        }
        else // otherwise code should be valid
        {     
            runCommands(history[historyCounter-1], false, 0); // execute the extracted command
        }
    }
    else if(tokens[0][0] == '-')  // in the case if its !-<no>
    {
        memmove(tokens[0], tokens[0] + sizeof(char), strlen(tokens[0])); // Remove the '-' from the string
        if(!checkAllDigits(tokens[0])) { // Checks that we aren't inputting something like !1a, since atoi below disregards the a even though it's not a valid input
            fprintf(stderr, "You have %d history commands in history. Please enter a valid history number.\n", historyCounter);
            return;
        }
        int num = atoi(tokens[0]); // cast the following string value to an integer
        if (num == 0) // edge case if 0 is the number
        {
            fprintf(stderr, "'-0' is not a valid positive or negative history integer.\n"); // return error feedback to the user
        }
        if(num >= 1 && num <= historyCounter) // If we have a valid number
        {
            if (history[historyCounter-num] != NULL) {
                runCommands(history[historyCounter-num], false, 0);
            }
            else {
                fprintf(stderr, "You have %d history commands in history. Please enter a valid history number.\n", historyCounter); // print error feedback in case history is null
            }
            
        }
        else // error feedback if somehow the cast is wrong
        {
            fprintf(stderr, "You have %d history commands in history. Please enter a valid history number.\n", historyCounter);
        }
    }
    else // in the case its just !<no>
    {
        if(!checkAllDigits(tokens[0])) { // Checks that we aren't inputting something like !1a, since atoi below disregards the a even though it's not a valid input
            fprintf(stderr, "You have %d history commands in history. Please enter a valid history number.\n", historyCounter);
            return;
        }
        int num = atoi(tokens[0]); // ascii to int
        if (num >= 1 && num <= historyCounter) 
        {
            if (history[num-1] != NULL) {
                runCommands(history[num-1], false, 0);
            }
            else { //error handling
                fprintf(stderr, "You have %d history commands in history. Please enter a valid history number.\n", historyCounter);
            }
        }
        else // handling feedback for 0 case
        {
            if (tokens[0][0] == '0')
            {
                fprintf(stderr, "'0' is not a valid positive or negative history integer.\n");
            }
            fprintf(stderr, "You have %d history commands in history. Please enter a valid history number.\n", historyCounter);
        }
    }
    memmove(tokens[0] + 1, tokens[0], strlen(tokens[0]) + 1); // we remove the ! mark to handle each case this is adding it back using mem dest, src and length of thing
    tokens[0][0] = '!';
}

bool checkAllDigits(char* token) { // Is used to check invalid history commands eg !1a, !-2b.
    for(int i = 0; token[i] != '\0'; i++) {
        if(isdigit(token[i]) == 0) {
            return false;
        }
    }
    return true;
}

void printHistory(char** tokens){
    if (historyCounter == 0){
        fprintf(stderr, "History list is empty.\n");
        return;
    }
    else if (tokens[1] != NULL) {
            fprintf(stderr, "Error: Too many parameters\n");
            return;
        }

    char* command = malloc(512);

    // Put the history commands into command[]
    for(int i = 0; i < historyCounter; i++){
    command[0] = '\0';
        for(int j = 0; history[i][j] != NULL; j++){     
            strcat(command, history[i][j]);
            strcat(command, " ");
        }
        // Print the commands
        printf("%d: %s\n", i + 1, command);             
        
    }
    free(command);
}

void saveHistory(const char* filename) 
{
    // sets the path for .hist_list to be in the home dir
    char hist_path[512];
    strcpy(hist_path, getenv("HOME"));
    strcat(hist_path, "/");
    strcat(hist_path, filename);

    // opens the file to write to
    FILE *file = fopen(filename, "w");
    if (!file) 
    {
        perror("Error - could not open history file");
        return;
    }
    
    // Write the number of history entries at the top of the file
    fprintf(file, "%d\n", historyCounter);
    
    // Write each command as a line in the file
    for (int i = 0; i < historyCounter; i++) 
    {
        // Convert tokens back to a command line
        char command[512] = "";
        for (int j = 0; history[i][j] != NULL; j++) 
        {
            if (j > 0) 
            {
                strcat(command, " ");
            }
            strcat(command, history[i][j]);
        }
        // Write the command to the file
        fprintf(file, "%s\n", command);
    }
    
    fclose(file);
    printf("History saved to %s\n", filename);
}


void loadHistory(const char* filename) 
{
    // sets the path for .hist_list to be in the home dir
    char hist_path[512];
    strcpy(hist_path, getenv("HOME"));
    strcat(hist_path, "/");
    strcat(hist_path, filename);
    
    // opens the file to read from
    FILE *file = fopen(filename, "r");
    if (!file) 
    {
        perror("Error - could not open history file");
        return;
    }
    
    // Free existing history if any
    for (int i = 0; i < historyCounter; i++) 
    {
        for (int j = 0; (history)[i][j] != NULL; j++) 
        {
            free((history)[i][j]);
        }
        free((history)[i]);
    }

    // Read the number of history entries
    int numEntries;
    if (fscanf(file, "%d\n", &numEntries) != 1) 
    {
        perror("Error reading history count");
        fclose(file);
        return;
    }
    
    historyCounter = 0;
    char line[512];
    
    // Read each line and add it to history
    while (historyCounter < numEntries && historyCounter < MAX_HISTORY_SIZE && fgets(line, sizeof(line), file)) 
    {
        // Remove newline if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') 
        {
            line[len-1] = '\0';
        }
        
        // Parse the line into tokens
        char lineCopy[512];
        strcpy(lineCopy, line);
        char** parsed = parse(lineCopy);
        
        // Create a new history entry
        (history)[historyCounter] = malloc(sizeof(char*) * 512);
        
        // Copy each token
        int i;
        for (i = 0; parsed[i] != NULL; i++) 
        {
            (history)[historyCounter][i] = strdup(parsed[i]);
        }
        (history)[historyCounter][i] = NULL;  // Null-terminate
        
        (historyCounter)++;
    }
    
    fclose(file);
    printf("Loaded %d commands from history file %s\n", historyCounter, filename);
}

void addAlias(char** tokens) {
    // Check if we have enough parameters
    if (tokens[2] == NULL) {
        fprintf(stderr, "Error: correct formatting is for usage: alias name command [args...]\n");
        return;
    }
    
    // Check if we reached the maximum number of aliases

    // Check if alias alrrunhistoryeady exists
    int existingIndex = -1;
    for (int i = 0; i < aliasCounter; i++) {
        if (strcmp(tokens[1], alias[i][0]) == 0) {
            existingIndex = i;
            break;
        }
    }

    // Check if alias list is full AND alias doesn't already exist in array
    if (aliasCounter >= MAX_ALIAS && existingIndex == -1) {
        fprintf(stderr, "No more aliases can be set.\n");
        return;
    }
    // If alias exists
    if (existingIndex != -1) {
        // Free existing command tokens
        for (int j = 0; alias[existingIndex][j] != NULL; j++) {
            free(alias[existingIndex][j]);
        }
        free(alias[existingIndex]);
        
        // Allocate new command
        alias[existingIndex] = malloc(sizeof(char*) * 512);
        
        // Copy alias name and command tokens
        alias[existingIndex][0] = strdup(tokens[1]);
        int j;
        for (j = 2; tokens[j] != NULL; j++) {
            alias[existingIndex][j-1] = strdup(tokens[j]);
        }
        alias[existingIndex][j-1] = NULL;  // Null-terminate
        
        printf("Previous alias '%s' has been overwritten.\n", tokens[1]);
    } 
    else {
        // Create new alias
        alias[aliasCounter] = malloc(sizeof(char*) * 512);
        
        // Copy alias name and command tokens
        alias[aliasCounter][0] = strdup(tokens[1]);
        int j;
        for (j = 2; tokens[j] != NULL; j++) {
            alias[aliasCounter][j-1] = strdup(tokens[j]);
        }
        alias[aliasCounter][j-1] = NULL;  // Null-terminate
        
        printf("Alias '%s' successfully assigned\n", tokens[1]);
        aliasCounter++;
    }
}

bool removeAlias(char** tokens) {

    // unalias requires one parameter
    if(tokens[1] == NULL) {
        fprintf(stderr, "Error: Too few parameters - must include alias to remove\n");
        return 0;
    }
    else if (tokens[2] != NULL) {
        fprintf(stderr, "Error: Too many parameters\n");
        return 0;
    } else if(aliasCounter == 0){
        fprintf(stderr, "Error: Alias list is empty, there are no aliases to remove\n");
        return 0;
    }

    // Attempt to find alias in the array
    for(int i = 0; i < aliasCounter; i++){
        // If alias specified is found in the alias array
        if(strcmp(tokens[1], alias[i][0]) == 0) {
            for(int j = 0; alias[i][j] != NULL; j++) {
                // Remove the alias command
                free(alias[i][j]);
            }
            // Remove the alias name from alias
            free(alias[i]);
            if(i != aliasCounter -1) {
                // Shift alias elements up to make room for more aliases
                alias[i] = alias[aliasCounter-1];
            }
            aliasCounter--;
            printf("Alias '%s' successfully removed.\n", tokens[1]);
            // Returning 1 indicates removeAlias was successful
            return 1;
        }
    }
    // If code gets here that means the alias does not exist
    fprintf(stderr, "Error: Alias does not exist\n");
    return 0;
}

void printAliases(void) {
    if (aliasCounter == 0){
        fprintf(stderr, "Alias list is empty\n");
        return;
    }

    char* name = malloc(512);
    for(int i = 0; i < aliasCounter; i++){  // Iterates through the array
        name[0] = '\0';  // NULL makes it a string
        for(int j = 0; alias[i][j] != NULL; j++){
            strcat(name, alias[i][j]);  // Adds the alias to the string
            strcat(name, " ");
        }
        printf("%d: %s\n", i + 1, name);  // Prints out each item
    }
    free(name); 
}

bool isAlias(char** tokens) {
    for(int i = 0; i < aliasCounter; i++){
        if(strcmp(tokens[0], alias[i][0]) == 0) {
            return 1;
        }
    }
    return 0;
}

bool runAlias(char** tokens, int depth) {
    if (depth >= 3) {
        fprintf(stderr, "Error: Alias loop limit reached\n");
        return false;
    }

    for (int i = 0; i < aliasCounter; i++) {
        if (strcmp(tokens[0], alias[i][0]) == 0) {
            // Count the number of tokens in the alias expansion
            int alias_token_count = 0;
            while (alias[i][alias_token_count + 1] != NULL) {
                alias_token_count++;
            }

            char* new_tokens[512] = {0};
            
            // Copy alias expansion tokens
            for (int j = 0; j < alias_token_count; j++) {
                new_tokens[j] = alias[i][j+1];
            }

            // If there were additional original tokens, append them
            int original_token_count = 0;
            while (tokens[original_token_count + 1] != NULL) {
                original_token_count++;
            }

            for (int j = 0; j < original_token_count; j++) {
                new_tokens[alias_token_count + j] = tokens[j+1];
            }

            new_tokens[alias_token_count + original_token_count] = NULL;

            
            runCommands(new_tokens, true, depth + 1); // Run the expanded command
            return true;
        }
    }
    
    return false;
}


void saveAlias(const char* filename) {
    // sets the path for .aliases to be in the home dir
    char alias_path[512];
    strcpy(alias_path, getenv("HOME"));
    strcat(alias_path, "/");
    strcat(alias_path, filename);

    // opens the file to write to
    FILE *file = fopen(alias_path, "w");
    if (!file) {
        perror("Error - could not open history file");
        return;
    }
    
    // Write the number of history entries at the top of the file
    fprintf(file, "%d\n", aliasCounter);
    
    // Write each command as a line in the file
    for (int i = 0; i < aliasCounter; i++) {
        // Convert tokens back to a command line
        char command[512] = "";
        for (int j = 0; alias[i][j] != NULL; j++) {
            if (j > 0) {
                strcat(command, " ");
            }
            strcat(command, alias[i][j]);
        }
        // Write the command to the file
        fprintf(file, "%s\n", command);
    }
    
    fclose(file);
    printf("Aliases saved to %s\n", filename);
}


void loadAlias(const char* filename) {
    // sets the path for .aliases to be in the home dir
    char alias_path[512];
    strcpy(alias_path, getenv("HOME"));
    strcat(alias_path, "/");
    strcat(alias_path, filename);

    // opens the file to read to
    FILE *file = fopen(alias_path, "r");
    if (!file) {
        perror("Error - could not open alias file\n");
        return;
    }
    
    // Free existing history if any
    for (int i = 0; i < aliasCounter; i++) {
        for (int j = 0; (alias)[i][j] != NULL; j++) {
            free((alias)[i][j]);
        }
        free((alias)[i]);
    }

    // Read the number of history entries
    int numEntries;
    if (fscanf(file, "%d\n", &numEntries) != 1) {
        perror("Error reading alias count\n");
        fclose(file);
        return;
    }
    
    aliasCounter = 0;
    char line[512];
    
    // Read each line and add it to history
    while (aliasCounter < numEntries && aliasCounter < MAX_ALIAS && fgets(line, sizeof(line), file)) {
        // Remove newline if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Parse the line into tokens
        char lineCopy[512];
        strcpy(lineCopy, line);
        char** parsed = parse(lineCopy);
        
        // Create a new history entry
        (alias)[aliasCounter] = malloc(sizeof(char*) * 512);
        
        // Copy each token
        int i;
        for (i = 0; parsed[i] != NULL; i++) {
            (alias)[aliasCounter][i] = strdup(parsed[i]);
        }
        (alias)[aliasCounter][i] = NULL;  // Null-terminate
        
        (aliasCounter)++;
    }
    
    fclose(file);
    printf("Loaded %d commands from alias file %s\n", aliasCounter, filename);
}
