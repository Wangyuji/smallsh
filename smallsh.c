/**************************************************************************************
 * Author: Matthew Solbrack
 * Date: 7/30/2019
 * Program: smallsh (Program 3 - CS 344)
 * Description: This is my own shell in C, similar to bash. The shell will run command
 * line instructions and return the results similar to other shells. The shell has 
 * 3 built in commands: exit, cd, and status. It will also support comments. 
 * ***********************************************************************************/
#include <stdio.h> //print and scanf libraries
#include <string.h> //string usage
#include <stdlib.h>
#define MAX 100


int main()
{
	while(1)
	{
	 	char str[100]; //Capture the string entered by the user

		// Making it look like a terminal; ":" is used as a prompt for each command line
		fflush(stdin);	//flush the output buffer 
		printf(": ");
		fgets(str, MAX, stdin);
		str[strlen(str)-1] = '\0'; //remove the newline from the string
		

		// exit when the user enters "exit $"
		if(strcmp(str, "exit $") == 0) { exit(0); } 



	}

	return 0;
}
