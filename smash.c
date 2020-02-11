//CREATED BY TOLGA BESER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//define funcs
int parseCommand(char **argv, int argC);
int removePath(char *rmStr);
void throwErr();


//default values
char *path = "/bin";

int main(int argc, char** argv) {
	//Make sure that program is started with valid inputs
	if (argc > 2) {
		//Throw error if input isn't valid
		char error_message[30] = "An error has occurred\n";
        	write(STDERR_FILENO, error_message, strlen(error_message)); 
		exit(1);
	}
	//loop through waiting for user input
	int batchMode = 0;
        char *line = NULL;
        size_t linecap = 0;
        size_t linelen;
        FILE *fp = stdin;
	if (argc == 2) {
		batchMode = 1;
		//read from an input file
		fp = fopen(argv[1],"r");
		if (fp == NULL) {
			throwErr();
		}
	}
	//Loop through the input
	if (batchMode == 0) {
		printf("smash> ");
	}
	//need to implement EOF thingy
	while (getline(&line, &linecap, fp) != -1) {
		//remove the '\n' at the end of line
		line[strlen(line)-1] = '\0';
		//parse the input into constituent string
		int place = 0;
		//TODO:THIS MIGHT MISS OUT ON THE FIRST COMMAND IF THERE ARE NO SPACES AFTER


		//TODO:THIS CANNOT BE HARDCODED IN SIZE IT WILL CAUSE ERRORS IN THE FUTURE
		char *inputArr[100];
		char *token;
		while ((token = strsep(&line, " ")) != NULL) {
			inputArr[place] = token;
			place++;
		}
		//run the command
		if (parseCommand(inputArr,place) != 1) {
			//dont know what to say
		}
		if (batchMode == 0) {
			printf("smash> ");
		}
	}		
	return 0;
}

//TODO:READ SPEC FOR ERRORS, most print statements should be errors
//returns 1 if command executed correctly
int parseCommand(char** argv, int argC) {
	//check for built-in commands first
	if ((strcmp(argv[0],"exit") == 0)) {
		//if args supplied after exit throw error
		if (argC != 1) {
			throwErr();
		}
		exit(0);
	}
	if ((strcmp(argv[0],"cd") == 0)) {
		if (argC != 2) {
			//not correct usage of cd
			printf("USAGE:cd [dirName]\n");
			return 0;
		}
		//implementation of cd
		if (chdir(argv[1]) == -1) {
			throwErr();
		}
	}
	if (strcmp(argv[0],"path") == 0) {
		if (argC < 2) {
			//not correct usage of path
			printf("USAGE:path [arg] [opt-arg]\n");
			return 0;
		}
		//various implementation of path command
		
		//add implementation
		if (strcmp(argv[1],"add") == 0) {
			if (argC != 3) {
				printf("USAGE:path add [arg]\n");
				return 0;
			}
			//add the given string to the start of the path	
			//add a space to the end of the string for easy parsing between paths
			char *addToken = argv[2];
			strcat(addToken," ");
			strcat(addToken,path);
			path = addToken;
			return 0;
		}
		if (strcmp(argv[1],"clear") == 0) {
			if (argC != 2) {
				printf("USAGE:path clear\n");
				return 0;
			}
			//clear the path so that nothing other than built-ins can be run
			path = "";
			return 1;
		}
		if (strcmp(argv[1],"remove") == 0) {
			if (argC != 3) {
				printf("USAGE:path remove [arg]\n");
				return 0;
			}
			//remove the given string from the path
			if(removePath(argv[2]) == -1) {
				//error statement
				throwErr();
			}
			return 1;
		}
		//user called path without a known command
		throwErr();
	}
	return 1;
}

//TODO:FIX THIS METHOD TO WORK
//return -1 if not found
//method created with help from stack overflow
int removePath(char *rmStr) {
	int rm = 0;
	//TODO: not sure if allowed to use malloc
	char *newPath = malloc(sizeof(path));
	char *token;
	while ((token = strsep(&path, " ")) != NULL) {
        	if (strcmp(token,rmStr) != 0) {
			strcat(newPath, token);
			strcat(newPath, " ");
		} else {
			rm = 1;
		}
	}
	if (rm == 0) {
		return -1;
	}
	path = strdup(newPath);
	free(newPath);
	return 0;
}



void throwErr() {
	char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
}
