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
#include <signal.h> //sigaction and struct
#include <unistd.h>
#include <sys/types.h>

#define MAX 100





int main()
{

	struct sigaction ignore_action = {0};
	ignore_action.sa_handler = SIG_IGN;

	sigaction(SIGINT, &ignore_action, NULL);

	pid_t spawnpid = -5;

 	char str[100]; //Capture the string entered by the user
	char * last_two; // variable to check for " &" in user input
	char * in, out; // two variables to hold < or >

	


	while(1)
	{
		// Making it look like a terminal; ":" is used as a prompt for each command line
		fflush(stdin);	//flush the output buffer 
		printf(": ");
		fgets(str, MAX, stdin);
		str[strlen(str)-1] = '\0'; //remove the newline from the string

		//If the " &" variable is at the end 		
		last_two = &str[strlen(str)-2];
		if (strcmp(" &", last_two) == 0)
			printf("The & was recognized");
		// exit when the user enters "exit $"
		if(strcmp(str, "exit $") == 0) { exit(0); }
		else if(str[0] == '#') { break; }
	 
		else 
			spawnpid = fork();

			if (spawnpid == -1)
			{
				perror("Error with command\n");
				exit(1);
			}
			else if (spawnpid == 0)
			{
				if (strstr(




			}
			switch (spawnpid)
			{
				case -1:
					perror("Error with command");
					exit(1);
					break;
				case 0: 
								



	}

	return 0;
}
