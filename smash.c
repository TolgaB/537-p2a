//CREATED BY TOLGA BESER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

//define funcs
int parseCommand(char **argv, int argC);
int removePath(char *rmStr);
int runProg(char *progPth, char **progArgs,int progArgC);
char *removeWhiteSpace(char *str);
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
		
		//first parse to see if semicolon commands exist
		char *cmdToken;
		//commands that are run one after the other
		while ((cmdToken = strsep(&line, ";")) != NULL) {
			//take out the whitespaces
			removeWhiteSpace(cmdToken);
			printf("CMD-TOKEN: %s\n", cmdToken);
			//now look for & statements
			int secPlace = 0;
			char **parArr = malloc(sizeof(char *) * strlen(cmdToken));
			char *parToken;
			while ((parToken = strsep(&cmdToken,"&"))) {
				parArr[secPlace] = removeWhiteSpace(parToken);
				secPlace++;
			}
			//TODO: NEED TO FREE MEM
			int *childPid = malloc(sizeof(int) * secPlace);
			int child = 0;
			int rc;
			int builtin = 0;
			//need to loop through parArr
			for (int i = 0; i < secPlace;i++) {
				//now we run parse the commands and run them
                        	int place = 0;
				char **inputArr = malloc(sizeof(char *) * strlen(parArr[i]));
                        	char *token;
                        	while ((token = strsep(&parArr[i], " ")) != NULL) {
                                	inputArr[place] = token;
                                	place++;
                        	}
				//TODO: CAN BUILT INS BE RUN PARALLEL?
				if ((strcmp(inputArr[0],"exit") == 0) || (strcmp(inputArr[0],"cd") == 0) || (strcmp(inputArr[0],"path") == 0)) {
                                        builtin = 1;
					parseCommand(inputArr,place);		
				} else {
					//TODO: prob have to rewrite this to do the commands in the function
					//run the command in a manner that they can be executed parallely
					rc = fork();
					if (rc != 0) {
						childPid[child] = rc;
						child++;
					}
                                        if (rc == 0) {	
                                                printf("running cmd\n");
                                                //the child process
                                                parseCommand(inputArr,place);
						printf("NEVER RUN\n");
                                        }
				}
			}
			if ((rc != 0) && (builtin == 0)) {
				//parent process
				while ((rc = waitpid(-1,NULL,0)) != -1) {
					printf("%d PS DIED\n",rc);
				}
			}
		}
		printf("passed out\n");
		//TODO: proper freeing of allocated mem
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
	else if ((strcmp(argv[0],"cd") == 0)) {
		if (argC != 2) {
			throwErr();
		}
		//first check if it can be accessed
		if (access(argv[1],F_OK | X_OK) == 0) {
			if (chdir(argv[1]) == -1) {
				throwErr();
			}
			return 0;
		} else {
			//TODO:Not sure if this counts as an error
			throwErr();
		}
	}
	else if (strcmp(argv[0],"path") == 0) {
		if (argC < 2) {
			throwErr();
		}
		//various implementation of path command
		
		//add implementation
		if (strcmp(argv[1],"add") == 0) {
			if (argC != 3) {
				throwErr();
			}
			//add the given string to the start of the path	
			//add a space to the end of the string for easy parsing between paths
			char *addToken = argv[2];
			//TODO:check for segfaults here
			strcat(addToken," ");
			strcat(addToken,path);
			path = addToken;
			return 0;
		}
		if (strcmp(argv[1],"clear") == 0) {
			if (argC != 2) {
				throwErr();
			}
			//clear the path so that nothing other than built-ins can be run
			path = "";
			return 1;
		}
		if (strcmp(argv[1],"remove") == 0) {
			if (argC != 3) {
				throwErr();
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
	//TODO:in the future check for error in num of args
	//means it must be a non built in command 
	else {
		int valid = 0;
		char *token;
		char *pathDup = strdup(path);
        	while ((token = strsep(&pathDup, " ")) != NULL) {
        		//TODO:check for segfaults here
			char *newPth = malloc(sizeof(token)+sizeof(argv[0])+1);
			strcat(newPth,token);
			//TODO: not sure if we handle like this
			if (newPth[strlen(newPth)-1] != '/') {
				strcat(newPth,"/");
			}
			strcat(newPth,argv[0]);
			if (access(newPth,X_OK) != -1) {
				valid = 1;
				runProg(newPth,argv,argC);
			}
			free(newPth);
		
		}
		//no access to the file
		if (valid == 0) {
			throwErr();
		}

	}
	//should never get to the end of the func
	throwErr();
	return 1;
}


int runProg(char *progPth,char **progArgs,int progArgC) {
		printf("runProg\n");
		//create the child process
		char **pgAr = malloc(sizeof(char *) * (progArgC+1));
		//have the child process run the prog
		for (int i = 0; i < progArgC; i++) {
			pgAr[i] = progArgs[i];
		}
		//overwrite path with right string
		pgAr[0] = progPth;
		pgAr[progArgC] = NULL;
		if (execv(progPth,pgAr) == -1) {
			//free the part of memory that was allocated on the heap
			for (int i = 0; i < (progArgC+1); i++) {
				free(pgAr[i]);
			}
			free(pgAr);
			throwErr();
		}
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

//method created with help from stackoverflow
char *removeWhiteSpace(char *str) {
	//remove from front
	while((isspace((unsigned char)*str))) {
		str++;
	}
	char *end = str + (strlen(str))-1;
	while(end > str && isspace((unsigned char)*end)) {
		end--;
	}
	end[1] = '\0';
	return str;
}


void throwErr() {
	char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
}
