#include <stdbool.h>
#define MAX_HISTORY_SIZE 20
#define MAX_ALIAS 10

char** parse(char input [512]);
// Tokenises the user input, so that the shell can read the input and decide which actions to take, char* tokens[]
// Example usage parse("ls -l") -> ["ls", "-l", "NULL"]


void clearBuffer(char *);
// Clears the input buffer.
// Example usage clearBuffer("ls\n") -> userInput = "ls"

void runCommands(char**, bool, int); 
// Runs custom commands that aren't executables.
// char** is the command, the bool should always be passed as false as it is changed if the command is unrecognised in order to add to history.
// Example usage runCommands("cd /home") changes cwd to home.

int executeSystemCommand(char **);
// Executes commands outside of the shell, such as ls.
// Creates a child process which the shell will wait on.
// Returns 0 if successful, -1 otherwise.
// Example usage executeSystemCommand("Ls -l") will list the current directory with long modifier.

void getPath(char**);   
// Prints the current system path. 
// char** is a parameter that is unused.
// Example usage if PATH is /hello/world will return "PATH: /hello/world"

void setPath(char**);   
// Sets the current system path as specified by the user. 
// Example usage setPath("/hello/world/hi") will set the PATH to /hello/world/hi

void changeDirectory(char**);   
// Changes the current working directory of the system.
// identical to cd

bool isHistoryCommand(char**);
// Returns true if the string is prefaced with !
// Example usage isHistoryCommand("!command") -> true

void addToHistory(char**);
// Adds a command to history array and if its full removes the oldest command (via a circular buffer).

void runHistoryCommand(char**);
// Executes a command from history using the !n syntax, where the n is the index of the previous command in the history array.
// If command is not in history, print an error to the shell.

void printHistory(char**);
// Prints the history array in a human readable format, 
// Example usage with an array ["ls", "cd /home", "pwd"]

//1: ls
//2: cd /home
//3: pwd

bool checkAllDigits(char* token);
// Checks that all of the characters in a given string are digits
// Used as validation during history calls
// Example usage: 
// 
// 23 returns true
// 2b returns false


void saveHistory(const char* filename);
// Saves the history to a file with that name (.hist_list).

void loadHistory(const char* filename);
// Loads the history from a file with that name (.hist_list).

void addAlias(char**);
// Adds a new alias based on the user input, will overwrite a previous alias with the same name.

bool removeAlias(char**);
// Removes a user's stored alias if it exists.
// Returns 1 if alias was successfully removed, 0 if unsuccessful.

void printAliases(void);
// Prints the alias array in a human readable format.

bool isAlias(char**);
// Checks if the given command is an alias, returns 1 if it is, 0 otherwise.

bool runAlias(char**, int);
// Runs aliased commands, has a check for aliasing aliases, not allowing more than 3 recursions. 
// Returns 1 if alias is successfully run, 0 otherwise.

void saveAlias(const char* filename);
// Saves the aliases to a file with that name (.aliases).

void loadAlias(const char* filename);
// Loads the aliases from a file with that name (.aliases).
