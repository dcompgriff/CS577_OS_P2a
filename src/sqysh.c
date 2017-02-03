/*
 ============================================================================
 Name        : sqysh.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
//Basic program libs.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

//For the open() call.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//For wait commands.
#include <sys/types.h>
#include <sys/wait.h>

#define LINE_SIZE 257

//My process structure used to keep track of information about the process.
typedef struct _mproc_struct{
	pid_t pid;
    char command[LINE_SIZE];
}mproc_struct;

/**
 * Function for executing child processes header.
 * */
mproc_struct* execChild(char* tokStr[], int numTokens, char* inputFile, char* outputFile, int isBackgroundTask);



int main(int argc, char* argv[]) {

	//Counter var
	int i = 0;
	//Option chars
	char c;
	//Input file name
	char* fileName;
	//File pointer
	FILE *fp;
	//Input line string
	char lineInput[LINE_SIZE];
	//Tokenized string vars
	int numTokens = 0;
	//At max, a token for each character. Could be reduced since < LINE_SIZE/2 tokens based on whitespace are possible.
	char* tokStr[LINE_SIZE];
	//Processing flags.
	int isInteractive = 0;
	int isFile = 0;
	//Shell command input/output file commands.
	int hasInputFile = 0;
	int hasOutputFile = 0;
	char* inputFile;
	char* outputFile;
	//Flag for background task.
	int isBackgroundTask = 0;

	if(isatty(fileno(stdin)) && argc == 1){
		//Go to interactive mode.
		isInteractive = 1;
		fp = stdin;
		printf("sqysh$ ");
	}else{
		if(argc > 1){
			fp = fopen(argv[1] , "r");
			if(!fp){
				fprintf(stderr,"File not found '%s'.\n", fileName);
				return 1;
			}
			isFile = 1;
			printf("fp is file.");
		}else{
			fp = stdin;
			printf("fp is stdin.");
		}
	}

	//While EOF not yet reached.
	while(fgets(lineInput, LINE_SIZE, fp) != NULL){
		//Reset flag variables.
		hasInputFile = 0;
		hasOutputFile = 0;
		isBackgroundTask = 0;
		inputFile = NULL;
		outputFile = NULL;
		mproc_struct* childProcess = NULL;
		int childStatus;

		//Parse input command into white space delimited words.
		numTokens = 0;
		tokStr[numTokens] = strtok(lineInput, " \t\n\v\f\r");
		numTokens += 1;
		//printf("Token: \'%s\'\n", tokStr[numTokens - 1]);
		while((tokStr[numTokens] = strtok(NULL, " \t\n\v\f\r")) != NULL){
			numTokens += 1;
			//printf("Token: \'%s\'\n", tokStr[numTokens - 1]);
		}
		if(tokStr[0] == NULL){
			printf("\nsqysh$ ");
			continue;
		}

		//If shell command (cd, bg(list alive background processes), pwd, exit), call shell functions.
		if(strcmp(tokStr[0], "cd") == 0){
			if(numTokens > 2){
				fprintf(stderr, "cd: too many arguments\n");
			}
			//If no argument called, switch to "$HOME" dir.
			if(numTokens == 1){
				chdir(getenv("HOME"));
			}
			//Call chdir function.
			if(!chdir(tokStr[1])){
				fprintf(stderr, "cd: %s: %s\n", tokStr[1], strerror(errno));
			}
		}else if(strcmp(tokStr[0], "pwd") == 0){
			//Call pwd function.
			char* dir = get_current_dir_name();
			printf("%s", dir);
		}else if(strcmp(tokStr[0], "exit") == 0){
			//Exit the process.
			if(isFile){
				fclose(fp);
			}
			exit(1);
		}else if(strcmp(tokStr[0], "bg") == 0){
			//List all bg processes that are still alive, with job index, cmd name, and pid.
		}else{
			//Read arguments until '<', or '>' is found. If found, perform I/O redirection preparations.
			i = 1;
			while(i < numTokens){
				if(strcmp("<", tokStr[i]) == 0){
					hasInputFile = 1;
					inputFile = tokStr[i+1];
					i += 2;
				}else if(strcmp(">", tokStr[i]) == 0){
					hasOutputFile = 1;
					outputFile = tokStr[i+1];
					i += 2;
				}else{
					i += 1;
				}
			}
			//Read last argument and check to see if '&'.
			if(strcmp("&", tokStr[numTokens - 1]) == 0){
				isBackgroundTask = 1;
			}
			//Call child process execute function, and 1) call wait_pid() if no '&', or 2) don't call wait_pid(), and simply monitor when process has completed.
			childProcess = execChild(tokStr, numTokens, inputFile, outputFile, isBackgroundTask);

			if(!isBackgroundTask && childProcess != NULL){
			    //printf("Call to wait for child pid.\n");
				waitpid(childProcess->pid, &childStatus, 0);
				//printf("Child returned with status: %d\n", childStatus);
			}
		}



		//Perform background process (zombie) cleanup before providing next prompt.
		//	*Use waitpid() with WNOHANG to check if any of your currently-running background processes have exited
		//TODO: May need to keep array of proc structs with original cmd name, proc pid, and proc pointer.
		//Remember to free the mproc_struct!

		//Before reading the next input, output a prompt.
		if(isInteractive){
			printf("\nsqysh$ ");
		}
	}

	//Close the file if a file was used as input.
	if(isFile){
		fclose(fp);
		printf("file closed.");
	}
	return EXIT_SUCCESS;
}




/**
 * Call fork, exec commands to create either a subchild to wait for, or a subchild to run in the background.
 * */
mproc_struct* execChild(char* tokStr[], int numTokens, char* inputFile, char* outputFile, int isBackgroundTask){

    //Create a new process structure.
    mproc_struct* newProcess = (mproc_struct*)malloc(sizeof(mproc_struct));

    //Call fork.
    pid_t cpid = fork();
    if(cpid == -1){
		printf("Fork error.\n");
		free(newProcess);
		return NULL;
    }
    if(cpid == 0){
    	//Process is the child process, so do child process stuff.
	    int inputFileDescriptor;
	    int outputFileDescriptor;

	    if(inputFile != NULL){
	    	//Open file for input.
			inputFileDescriptor = open(inputFile, O_RDONLY);
			//Make fd 0 point to the input file.
			dup2(inputFileDescriptor, STDIN_FILENO);
			//Close the previous file descriptor.
			close(inputFileDescriptor);
	    }
	    if(outputFile != NULL){
	    	//Open file for output.
			outputFileDescriptor = open(outputFile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
			//Make fd output point to the output file.
			dup2(outputFileDescriptor, STDOUT_FILENO);
			//Close the previous file descriptor.
			close(outputFileDescriptor);
	    }
	    //Determine the arguments to pass to the program.
	    char* tokArg[LINE_SIZE];
	    //Initialize tokArg for when no options are passed.
	    tokArg[0] = tokStr[0];
	    int i;
	    for(i = 1; i < numTokens; i++){
			//Parse command arguments.
			if(strcmp(">",tokStr[i]) == 0){
				tokArg[i] = NULL;
				break;
			}else if(strcmp("<", tokStr[i]) == 0){
				tokArg[i] = NULL;
				break;
			}else if(strcmp("&", tokStr[i]) == 0){
				tokArg[i] = NULL;
				break;
			}else{
				tokArg[i] = tokStr[i];
			}
	    }
	    //Null terminate the tokArg list.
	    tokArg[i] = NULL;

		//Execute the desired program.
		if(execvp(tokStr[0], tokArg)){
			printf("Error on execvp.");
			free(newProcess);
			return NULL;
		}
    }else{
		//Process is the parent process, so do parent process stuff.
		newProcess->pid = cpid;
		strncpy(newProcess->command, tokStr[0], LINE_SIZE);
		return newProcess;
    }
}
