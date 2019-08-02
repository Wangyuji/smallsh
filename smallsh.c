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
#include <unistd.h> // for exec()
#include <sys/types.h>
#include <fcntl.h> // for fctl use

#define MAX 2048







int main()
{

	struct sigaction ignore_action = {0};
	ignore_action.sa_handler = SIG_IGN;

	sigaction(SIGINT, &ignore_action, NULL);

	pid_t spawnpid = -5;

 	char 	str[MAX], //Capture the string entered by the user
		str_array[512][100]; //break string into words

	char *	str_args[512]; //capture arguments

	int 	word_count, character_count, loop, redirection, childExitMethod = -5, targetFD, result; //keeps track of words 
	


	while(1)
	{
		// Making it look like a terminal; ":" is used as a prompt for each command line
		fflush(stdin);	//flush the output buffer 
		printf(": ");
		fgets(str, MAX, stdin);
		str[strlen(str)-1] = '\0'; //remove the newline from the string
		
		word_count = 0;
		character_count=0;
		for (loop = 0; loop <= (strlen(str)); loop++)
		{
			if(str[loop] == ' ' || str[loop] == '\0')
			{
				str_array[word_count][character_count] = '\0';
				character_count = 0;
				word_count++;
			}
			else
			{
				str_array[word_count][character_count] = str[loop];
				character_count++;
			}
		}



		//If the " &" variable is at the end 		
		if (strcmp("&", str_array[word_count - 1]) == 0)
			printf("The & was recognized");


		// exit when the user enters "exit $"
		if(strcmp(str_array[0], "exit") == 0) { exit(0); }


		// re-prompt for another command when it receives either a blank line or a comment line
		else if(str[0] == '#' || strlen(str) == 0) { }

		else 
		{
			//This section creates the argments that is to be put into the exec() function							

		

			if ((spawnpid = fork()) == -1)
				perror("fork() error");
			else if (spawnpid == 0)
			{
				redirection = 0;
				for (loop = 0; loop < word_count; loop++)
				{
					if(strcmp(str_array[loop], ">") == 0 || strcmp(str_array[loop], "<") == 0)
						break;
					else
					{
						str_args[redirection] = str_array[loop];
						redirection++;
					}			
				}
				while (loop < word_count)
				{
					if(strcmp(str_array[loop], ">") == 0)
					{
						targetFD = open(str_array[loop+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
						if (targetFD == -1)
						{
							perror("targetopen()");
							exit(1);
						}
						result = dup2(targetFD, 1);
						if (result == -1)
						{
							perror("source dup2()");
							exit(1);
						}
						loop = loop + 2;
					}
					else if(strcmp(str_array[loop], "<") == 0)
					{
						targetFD = open(str_array[loop+1], O_RDONLY);
						if(targetFD == -1)
						{
							perror("Error Opening File");
							exit(1);
						}
						result = dup2(targetFD, 0);
						if (result == -1)
						{
							perror("source dup2()");
							exit(1);
						}
						loop = loop + 2;
					}	
				}
								
				str_args[redirection] = NULL;	// to make sure the last argument is NULL 
				
				execvp(str_args[0], str_args); 
				perror("");	
				fcntl(targetFD, F_SETFD, FD_CLOEXEC);		
				exit(0);	
			}
			else
			{
				waitpid(spawnpid, &childExitMethod, 0);

			}
		}
	}
	return 0;
}
