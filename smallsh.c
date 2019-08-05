/**************************************************************************************
 * Author: Matthew Solbrack
 * Date: 7/30/2019
 * Program: smallsh (Program 3 - CS 344)
 * Description: This is my own shell in C, similar to bash. The shell will run command
 * line instructions and return the results similar to other shells. The shell has 
 * 3 built in commands: exit, cd, and status. It will also support comments. Everything
 * within this file is in reference to lecture material unless otherwise noted.  
 * ***********************************************************************************/
#include <stdio.h> //print and scanf libraries
#include <string.h> //string usage
#include <stdlib.h>
#include <signal.h> //sigaction and struct
#include <unistd.h> // for exec()
#include <sys/types.h>
#include <fcntl.h> // for fctl use
#include <sys/wait.h> // waitpid()

#define MAX 2048

//Global Variable
int backgroundSwitch = 1; // Switch for the SIGTSTP


//Function: catch_SIGSTP
// Input: signo 
// Description: This function is used when CTRL-Z is used by the user. it will 
//     go back and fourth 
void catch_SIGTSTP(int signo) 
{
	if (backgroundSwitch == 1) 
	{
		char* message = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, SIGRTMIN+16);
		fflush(stdin);	//flush the output buffer 
		backgroundSwitch = 0;
	}

	else 
	{
		char* message = "\nExiting foreground-only mode\n";
		write (1, message, SIGRTMIN-4);
		backgroundSwitch = 1;
	}
}


int main()
{

	// A CTRL-C command from the keyboard will send a SIGNINT signal to parents and all childern at the same time
	struct sigaction signal1 = {{0}};
	signal1.sa_handler = SIG_IGN;            
	signal1.sa_flags = 0;                   
	sigfillset(&signal1.sa_mask);
	sigaction(SIGINT, &signal1, NULL);

	// A CTRL-Z command from the kayboard will send a SIGTSTP signal to parent shell process and all children. 
	struct sigaction signal2 = {{0}};
	signal2.sa_handler = catch_SIGTSTP;
	signal2.sa_flags = 0;
	sigfillset(&signal2.sa_mask);
	sigaction(SIGTSTP, &signal2, NULL);

	pid_t 	spawnpid = -5; //child pid variable
 
 	char 	str[MAX], //Capture the string entered by the user
		str_array[512][100], //break string into words
		chngdir[100]; //holds the home directory

	char *	str_args[512]; //capture arguments

	int 	lastExit = 0, // Stores exit status number
		numPids = 0, // stores number of total pids
		signal = 0, // switch for signal
		word_count, // stores the word count from user input
		character_count, // stores the number of characters
		loop, loop2, // used in for loops
		redirection, // track the amount of commands
		childExitMethod, // track exit method
		targetFD, // used for opening files
		result,  // variable to hold the result of dup2() 
		command_loop = 1, // this variable controls the mail while loop
		background = 0; // switch for the background


	// Store pid into a char to replace "$$" with pid source: https://stackoverflow.com/questions/15262315/how_to_conver-pid-t-to-string
	char pid[10];	
	sprintf(pid, "%d", getpid());


	while(command_loop)
	{

		/************************** Get the command from the user ****************************/
		// Making it look like a terminal; ":" is used as a prompt for each command line

		fflush(stdin);	//flush the output buffer 
		printf(": ");
		fgets(str, MAX, stdin);
		str[strlen(str)-1] = '\0'; //remove the newline from the string
	

		/********************** Put user commands into an array of strings ********************/ 
		// replace $$ with pid number and put each word into array
		word_count = 0;
		character_count=0;
		for ( loop = 0; loop <= (strlen(str)); loop++)
		{
			// replace "$$" with pid
			if(str[loop] == '$' && str[loop+1] == '$')
			{
				for(loop2 = 0; loop2 < strlen(pid); loop2++)
				{
					str_array[word_count][character_count] = pid[loop2];
					character_count++;
				}
				loop++;
			}
			else if(str[loop] == ' ' || str[loop] == '\0')
			{
				str_array[word_count][character_count] = '\0';
				character_count = 0;
				word_count++;
			}

			// if & symbol is used at the end of command, turn on background switch
			else if(str[loop] == '&' && character_count == 0 && str[loop+1] == '\0')
			{
				loop = strlen(str);
				background=1;
			}
			// add character to word
			else
			{
				str_array[word_count][character_count] = str[loop];
				character_count++;
			}
		}

	
		/************************************** Exit Command **********************************/
		if(strcmp(str_array[0], "exit") == 0) 
		{
			//Discontinue loop
			command_loop=0;
		}


		/************************ Ignore '#' commands and blank commands **********************/
		// re-prompt for another command when it receives either a blank line or a comment line
		else if(str[0] == '#' || strlen(str) == 0) { }


		/*********************************** cd Command *************************************/
		else if(strcmp(str_array[0], "cd") == 0)
		{
			if(word_count == 1)
			{
				sprintf(chngdir, "%s", getenv("HOME"));
				if (chdir(chngdir) != 0)
					printf("Changing to Home directory failed\n");	
			}
			else if(word_count == 2)
			{
				strcpy(chngdir, str_array[1]);
				if (chdir(chngdir) != 0)
					printf("Change to %s directory failed\n", chngdir);
			}
			else
			{
				printf("Unexpected arguments with cd command\n");
			}
		}


		/********************************** status command **********************************/
		else if(strcmp(str_array[0], "status") == 0)
		{
			if(signal)
			{
				printf("terminated by signal %d\n", signal);
				signal=0;
			}
			else
			{
				printf("exit value %d\n", lastExit);
			}
		}


		/********************************* exec() function **********************************/	
		else 
		{
			//This section creates the argments that is to be put into the exec() function							

		
			/*********************** fork child and handle error ***********************/
			if ((spawnpid = fork()) == -1)
			{
				perror("fork() error");
				exit(1);
			}

			/********************** direct child process here *************************/
			else if (spawnpid == 0)
			{

				// add ^C as default
				signal1.sa_handler = SIG_DFL;
				sigaction(SIGINT, &signal1, NULL);

				/*************** seperate cmd from direction ************************/
				redirection = 0;
				// Put arguments in array
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

				// put directional features in dup2()
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

				/*************** execute the command *******************************/
								
				str_args[redirection] = NULL;	// to make sure the last argument is NULL 
				if (execvp(str_args[0], str_args))
				{	
					printf("No such file or directory\n");	
					fflush(stdout);
					exit(2);
				}

				// close any files that were opened
				fcntl(targetFD, F_SETFD, FD_CLOEXEC);		
			}
			else
			{
				/*********************** background *********************************/
				if (background && backgroundSwitch) 
				{
					//parent has no hang
					waitpid(spawnpid, &childExitMethod, WNOHANG);
					printf("Background pid is %d\n", spawnpid);

					// increment numPids to keep track of background pids 
					numPids++;			
				}
				else 
				{
					waitpid(spawnpid, &childExitMethod, 0);

					if (strcmp("kill", str_array[0]) == 0) {} //ignore kill 
					else if (WIFEXITED(childExitMethod))
					{	
						signal=0;
						lastExit = WEXITSTATUS(childExitMethod);
					}
					else
					{
						signal = WTERMSIG(childExitMethod);
						printf("terminated by signal %d\n", signal);
					}
						
				}

		
			}

		/************************************** Check background PID *************************/
			while ((spawnpid = waitpid(-1, &childExitMethod, WNOHANG)) > 0)
			{
				if(numPids == 0){}
				else if (WIFEXITED(childExitMethod))
				{
					signal=0;
					lastExit = WEXITSTATUS(childExitMethod);
					printf("Background pid %d is done: exit value %d\n", spawnpid, lastExit);
				}
				else
				{
					signal = WTERMSIG(childExitMethod);
					printf("Background pid %d is done: terminated by signal %d\n", spawnpid, signal);
				}
				numPids--; // numPids
				fflush(stdout);
			}		
		}
	background=0;
	}
	return 0;
}
