/* Program created by Ethan Foss. This program creates a shell interface for the user to interact with.
Functionality includes the implementation of a unix command-line as well as access to the past 10 commands stored as history. */
#define MAX_LINE 80
//Retrieved from https://stackoverflow.com/questions/26284110/strdup-confused-about-warnings-implicit-declaration-makes-pointer-with
//This was used to alleviate the warnings given when implementing the strdup() function from the POSIX system.
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static void addHistory(char** args, int index, int processID);
static char* lastHistory(void);
static char* getHistory(int n);
static void enumerateHistory();

//Static global String and integer arrays to store the history of commands and the history of PIDs
static char* history[10];
static int historyID[10];

int main(void){

	char *args[MAX_LINE/2 + 1];
	int should_run = 1;

	while(should_run){
		begin:
		printf("CSCI3120>");
		fflush(stdout);

		int index = 0;
		char temp[MAX_LINE+1];
		char *tokString;

		//Retrieve the line of user input
		fgets(temp, MAX_LINE+1, stdin);

		//Tokenize the string and divide it into the String array "args"
		tokString = strtok(temp, " ");
		while(tokString != NULL){
			args[index] = tokString;
			tokString = strtok(NULL, " ");
			index++;	
		} 
		//Add a NULL terminator
		args[index] = NULL;
		//Remove any possible newline character by adding a NULL character
		int finalArgLen = strlen(args[index-1]);
		args[index-1][finalArgLen-1] = '\0';

		//Set the value of the command pointer to the first command (with no parameters)
		char *command = strdup(args[0]);

		//Exit the program if the user opted to "exit"
		if(strcmp(command, "exit") == 0){
			should_run = 0;
			goto end;
		}

		//If the user entered the command "!!", execute the most recent command from history
		if(strcmp(command, "!!") == 0){

			if(history[0] == NULL){
				printf("No commands in history.\n");
				goto begin;
			}

			printf("Command: %s\n", lastHistory());

			tokString = strtok(lastHistory(), " ");
				index = 0;
				while(tokString != NULL){
					args[index] = tokString;
					tokString = strtok(NULL, " ");
					index++;
				} 
				args[index] = NULL;
				free(command);
				command = strdup(args[0]);

		//If the user entered the command "!" along with an integer 'N' as a parameter, execute the 'Nth' command from history
		} else if(strcmp(command, "!") == 0){
			if(strcmp(args[1], "1") == 0 || strcmp(args[1], "2") == 0 || strcmp(args[1], "3") == 0 || strcmp(args[1], "4") == 0
			 || strcmp(args[1], "5") == 0 || strcmp(args[1], "6") == 0 || strcmp(args[1], "7") == 0 || strcmp(args[1], "8") == 0 
			 || strcmp(args[1], "9") == 0 || strcmp(args[1], "10") == 0){
	
				if(history[atoi(args[1]-1)] == NULL){
					printf("No such command in history.\n");
					goto begin;
				}

				printf("Command: %s\n", getHistory(atoi(args[1])-1));

				tokString = strtok(getHistory(atoi(args[1])-1), " ");
				index = 0;
				while(tokString != NULL){
					args[index] = tokString;
					tokString = strtok(NULL, " ");
					index++;
				} 
				args[index] = NULL;
				free(command);
				command = strdup(args[0]);

			}
		//If the user entered the command "history", enumerate the last 10 commands and PIDs stored in history
		} else if(strcmp(command, "history") == 0){
			enumerateHistory();
			//Return to beginning of loop
			goto begin;
		}

		//Create a child process
		pid_t pid = fork();

		//If the child process creation failed, exit with a failure
		if(pid == -1){
			exit(EXIT_FAILURE);
		}

		//If the user DID NOT include an '&' in their command, have the parent process wait
		if(index>=1){
			if(strcmp(args[index-1], "&") != 0){
				wait(NULL);
			//If the command included an '&', DO NOT have the parent process wait and remove the '&' from the command
			} else {
				index--;
				args[index] = NULL;
			}
		} 

		//If parent process: add command and parameters as well as PID to history
		if(pid > 0){
			addHistory(args, index-1, pid);
		//If child process: execute command
		} else if(pid == 0){
			execvp(command, args);
			exit(0);
		}

	}
	end:
	return 0;
}

//Function adds command and PID parameters to history
static void addHistory(char** args, int index, int processID){
	//Shift all commands in the history array
	for(int i=9; i>0; i--){
		if(history[i-1] != NULL){
			history[i] = strdup(history[i-1]);
		}
		historyID[i] = historyID[i-1];
	}

	char* copied = "";
	//Store a hard-copy of the values pointed to by "args". Concatenate the array into one single string.
	for(int i=0; i<=index; i++){
		copied = strcat(strdup(copied), strdup(args[i]));
		copied = strcat(strdup(copied), " ");
	}
	copied = strcat(strdup(copied), "\0");

	//Add the copied command to history
	history[0] = strdup(copied);
	//Free the memory allocated to "copied"
	free(copied);
	//Add the PID to history
	historyID[0] = processID;
}

//Function returns the last command from history
static char* lastHistory(void){
	return strdup(history[0]);
}

//Function returns the Nth command from history
static char* getHistory(int n){
	return strdup(history[n]);
}

//Function enumerates the past 10 commands and PIDs stored in history
static void enumerateHistory(){
	printf("\nID\tPID Command\n");
	for(int i=0; i<10; i++){
		printf("%d\t%d %s\n", i+1, historyID[i], history[i]);
	}
}
