//CREATED BY TOLGA BESER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

//define funcs
int parseCommand(char **argv, int argC, int outputToFile, char *outputFileName);
int removePath(char *rmStr);
int runProg(char *newPth, char **progArgs,int progArgC, int outputToFile, char *outputFileName);
char *removeWhiteSpace(char *str);
void throwErr(int exitErr);
int getInput(FILE *fp, int batchMode);

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
        FILE *fp = stdin;
	int batchMode = 0;
	if (argc == 2) {
		batchMode = 1;
		//read from an input file
		fp = fopen(argv[1],"r");
		if (fp == NULL) {
			throwErr(1);
		}
	}
	getInput(fp, batchMode);
	return 0;
}


int getInput(FILE *fp, int batchMode) {
	//loop through waiting for user input
        char *line = NULL;
        size_t linecap = 0;
        size_t linelen;
	//Loop through the input
        printf("smash> ");
	fflush(stdout);
        //need to implement EOF thingy
        while ((getline(&line, &linecap, fp) != -1)) {
	   	//remove the '\n' at the end of line
		char *lineCopy = strdup(line);
		if (batchMode) {
			lineCopy[strlen(lineCopy)-1] = '\0';
		}
                //first parse to see if semicolon commands exist
                char *cmdToken;
                //commands that are run one after the other
                while ((cmdToken = strsep(&lineCopy, ";")) != NULL) {
                        //take out the whitespaces
                        removeWhiteSpace(cmdToken);
                        int outputToFile = 0;
                        int incorrectInput = 0;
                        //look for redirection
                        int redPlace = 0;
                        char **redArr = malloc(sizeof(char *) * strlen(cmdToken));
                        char *redToken;
                        while ((redToken = strsep(&cmdToken,">"))) {
                                if (redPlace >= 2) {
                                        //wont be able to leave the func?
                                        incorrectInput = 1;
                                        throwErr(0);
                                }
                                redArr[redPlace] = removeWhiteSpace(redToken);
                                redPlace++;
                        }
                        if (incorrectInput) continue;

                        char *outputFileName;
                        //make sure that only one file is given if output
                        if (redPlace > 1) {
                                //we now know that all outputs should go to specified file
                                outputToFile = 1;
                                //make sure that there is only one output file
                                //TODO: is more then one output file an end prog error?
                                char *outputCopy = strdup(redArr[1]);
                                char *outputToken;
                                int num = 0;
                                while ((outputToken = strsep(&outputCopy, " "))) {
                                        outputFileName = outputToken;
                                        num++;
                                }
                                if (num > 1) {
                                        incorrectInput = 1;
                                }
                        }

                        if (incorrectInput) {
                                throwErr(0);
                                continue;
                        }

                        //now look for & statements
                        int secPlace = 0;
                        char **parArr = malloc(sizeof(char *) * strlen(redArr[0]));
                        char *parToken;
                        while ((parToken = strsep(&redArr[0],"&"))) {
                                parArr[secPlace] = removeWhiteSpace(parToken);
                                secPlace++;
                        }
                        //TODO: NEED TO FREE MEM
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
					if (strcmp(inputArr[0],"exit") == 0) {
					//	fclose(fp);
					}
                                        parseCommand(inputArr,place,outputToFile, outputFileName);
                                } else {
                                        //TODO: prob have to rewrite this to do the commands in the function
                                        //run the command in a manner that they can be executed parallely
					rc = fork();
                                        if (rc == 0) {
                                                //the child process 
						//only redirect leftmost
						if (i != (secPlace-1)) {
								outputToFile = 0;
						}
                                                parseCommand(inputArr,place,outputToFile, outputFileName);
                                        }
                                }
				//free(inputArr);
                        }
			//free(parArr);
			//free(redArr);
                        if ((rc != 0) && (builtin == 0)) {
                                //parent process
                                while ((rc = waitpid(-1,NULL,0)) != -1) {
                                }
                        }
                }
		printf("smash> ");
		fflush(stdout);
        }
	return 0;
}

//TODO:READ SPEC FOR ERRORS, most print statements should be errors
//returns 1 if command executed correctly
int parseCommand(char** argv, int argC, int outputToFile, char *outputFileName) {
	//check for built-in commands first
	if ((strcmp(argv[0],"exit") == 0)) {
		//if args supplied after exit throw error
		if (argC != 1) {
			throwErr(0);
		}
		exit(0);
	}
	else if ((strcmp(argv[0],"cd") == 0)) {
		if (argC != 2) {
			throwErr(0);
		}
		//first check if it can be accessed
		else if (access(argv[1],F_OK | X_OK) == 0) {
			if (chdir(argv[1]) == -1) {
				throwErr(0);
			}
		} else {
			throwErr(0);
		}
		return 0;
	}
	else if (strcmp(argv[0],"path") == 0) {
		if (argC < 2) {
			throwErr(0);
		}
		//various implementation of path command
		
		//add implementation
		if (strcmp(argv[1],"add") == 0) {
			if (argC != 3) {
				throwErr(0);
			}
			//add the given string to the start of the path	
			//add a space to the end of the string for easy parsing between paths
			char *addToken = argv[2];
			//TODO:check for segfaults here
			strcat(addToken," ");
			strcat(addToken,path);
			path = addToken;
		}
		else if (strcmp(argv[1],"clear") == 0) {
			if (argC != 2) {
				throwErr(0);
			}
			//clear the path so that nothing other than built-ins can be run
			path = "";
		}
		else if (strcmp(argv[1],"remove") == 0) {
			if (argC != 3) {
				throwErr(0);
			}
			//remove the given string from the path
			if(removePath(argv[2]) == -1) {
				//error statement
				throwErr(0);
			}
		} else {
			//user called path without a known command
			throwErr(0);
		}
		return 0;
	}
	//TODO:in the future check for error in num of args
	//means it must be a non built in command 
	else {
		int valid = 0;
		char *token;
		char *pathDup = strdup(path);
        	while ((token = strsep(&pathDup, " ")) != NULL) {
        		//TODO:check for segfaults here
			char *newPth = malloc(sizeof(char) * (sizeof(token)+sizeof(argv[0])+10));
			strcat(newPth,token);
			//TODO: not sure if we handle like this
			if (newPth[strlen(newPth)-1] != '/') {
				strcat(newPth,"/");
			}
			strcat(newPth,argv[0]);
			if (access(newPth,X_OK) != -1) {
				valid = 1;
				runProg(newPth,argv,argC,outputToFile,outputFileName);
			}
			//free(newPth);	
		}
		//no access to the file
		if (valid == 0) {
			throwErr(1);
		}
		return 0;
	}
	//should never get to the end of the func
	throwErr(0);
	return 1;
}


int runProg(char *newPth,char **progArgs,int progArgC, int outputToFile, char *outputFileName) {
		//create the child process
		char **pgAr = malloc(sizeof(char *) * (progArgC+1));
		//have the child process run the prog
		for (int i = 0; i < progArgC; i++) {
			pgAr[i] = progArgs[i];
		}
	        if (outputToFile) {
			FILE *new_f = fopen(outputFileName, "w");
			int new_fd = fileno(new_f);
			dup2(new_fd,1);
			dup2(new_fd,2);
		}
		pgAr[progArgC] = NULL;
		if (execv(newPth,pgAr) == -1) {
			//free the part of memory that was allocated on the heap
			free(pgAr);
			throwErr(0);
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


void throwErr(int exitErr) {
	char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        if (exitErr == 1) {
		exit(1);
	}
}
