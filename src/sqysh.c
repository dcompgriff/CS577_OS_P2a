/*
 ============================================================================
 Name        : sqysh.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>

#define LINE_SIZE 257

/**
 * Shell built-in function headers.
 * */


/**
 * Function for executing child processes header.
 * */



int main(int argc, char* argv[]) {

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

		//Parse input command into white space delimited words.
		numTokens = 0;
		tokStr[numTokens] = strtok(lineInput, " \t\n\v\f\r");
		numTokens += 1;
		printf("Token: \'%s\'\n", tokStr[numTokens - 1]);
		while((tokStr[numTokens] = strtok(NULL, " \t\n\v\f\r")) != NULL){
			numTokens += 1;
			printf("Token: \'%s\'\n", tokStr[numTokens - 1]);
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
		}else if(strcmp(tokStr[0], "ls") == 0){
			//List files in current dir, or dir passed in.
			DIR *d;
			struct dirent *dir;
			if(numTokens > 1){
				d = opendir(tokStr[1]);
			}else{
				d = opendir(".");
			}
			if(d){
				while ((dir = readdir(d)) != NULL){
					printf("%s\n", dir->d_name);
				}
				closedir(d);
			}
		}else{
			//If not shell command:
			//Read arguments until '<', or '>' is found. If found, perform I/O redirection preparations.
			//Read last argument and check to see if '&'.
			//Call child process execute function, and 1) call wait_pid() if no '&', or 2) don't call wait_pid(), and simply monitor when process has completed.
		}

		//Perform background process (zombie) cleanup before providing next prompt.
		//	*Use waitpid() with WNOHANG to check if any of your currently-running background processes have exited
		//TODO: May need to keep array of proc structs with original cmd name, proc pid, and proc pointer.

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
