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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

int main(int argc, char* argv[]) {

	char c;
	int reverse_sort = -1;
	int print_number_lines = -1;

	//Read in command line options, and set corresponding flags.
	while ((c = getopt (argc, argv, "rn::")) != -1){
		int i = 0;
		switch (c){
			case 'r':
				//Reverse sorting order.
				reverse_sort = 1;
				break;
			case 'n':
				//Convert option value to number. (Returns 0 if invalid conversion)
				while(optarg[i] != '\0'){
					if(!isdigit(optarg[i])){
						fprintf(stderr, "Option n error, not a number.\n");
						return 1;
					}else{
						i++;
					}
				}

				print_number_lines = atoi(optarg);
				//If invalid argument, return negative.
				if(print_number_lines < 0){
					fprintf(stderr, "Option error `-%d'.\n", print_number_lines);
					return 1;
				}
				break;
			case '?':
				//Unknown option encountered.
				if(isprint(optopt)){
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				}else{
					fprintf(stderr,"Unknown option character `\\x%x'.\n", optopt);
				}
				//Return with exit error.
				return 1;
			default:
				abort();
		  }
	}

	return EXIT_SUCCESS;
}
