/**
 * +-----------+
 * |  ShellB   |
 * +-----------+
 *	@author Kinjal Ray
 *	@version 1.1
 *	Implementation of a simple Shell
 *
 *	Compile:
 *		gcc shellb.c -lreadline -o shellb
 *	Run:
 *		./shellb
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

/*	Color Definitions Begin	*/
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define BLUE "\x1b[34m"
#define BOLD "\033[1m"
#define RESET "\x1b[0m"
/*	Color Definitions End	*/

/*	Shorthand definition to clear terminal screen	*/
#define cls() printf("\e[1;1H\e[2J");

/*
 *	Function: trimString
 *	Trims whitespaces from the ends of a string
 *	Parameters:
 *		char* str (OUTPUT)	- input string to be trimmed
 *Returns:
 *		None
 */
void trimString(char *str)
{
	if (str[0] == ' ' || str[0] == '\n')
	{
		/*	Move whole string one location to the right if there is
				whitespace in the beginning	*/
		memmove(str, str + 1, strlen(str));
	}

	if (str[strlen(str) - 1] == ' ' || str[strlen(str) - 1] == '\n')
	{
		/*	Terminate string earlier if ther is whitespace at the end	*/
		str[strlen(str) - 1] = '\0';
	}
}

/*
 *	Function: tokenizeString
 *	Splits a string according to the given delimiter
 *	Parameters:
 *		char**args (OUTPUT)	- array of tokens created
 *		int *argc (OUTPUT)		- number of tokens created
 *		char *str							- input string to tokenize
 *const char* delim			- delimiter used to tokenize the string
 *	Returns:
 *		None
 */
void tokenizeString(char **args, int *argc, char *str, const char *delim)
{
	char *token;
	int pc;
	token = strtok(str, delim);
	int cnt = -1;
	while (token)
	{
		args[++cnt] = (char*) malloc((sizeof(token) + 1) *sizeof(char));
		strcpy(args[cnt], token);
		trimString(args[cnt]);
		token = strtok(NULL, delim);
	}
	args[++cnt] = NULL;
	*argc = cnt;
}

/*
 *	Function: execCmd
 *	Execute a single command
 *	Parameters:
 *		char**args	- array of strings containing the command in the format
 *									{<command>, < parameter 1>, ..., < parameter N>, NULL}
 *	Returns:
 *		None
 */
void execCmd(char **args)
{
	if (fork() == 0)
	{
		execvp(args[0], args);
		exit(1);
	}
	else
	{
		wait(NULL);
	}
}

/*
 *	Function: executePiped
 *	Execute multiple commands joined by I/O pipes ("|")
 *	Parameters:
 *		char**argStr	- array of commands to execute in pipes
 *		int argc			- number of commands
 *	Returns:
 *		None
 */
void executePiped(char **argStr, int argcnt)
{
	/*	Using the same code submitted in assignment on 05.02.2020	*/
	int fd[2], i, cnt;
	pid_t pid[argcnt];
	int prev;
	char *args[200];

	for (i = 0; i < argcnt; i++)
	{
		/*	Each command is tokenized and executed like a single command
				after assigning proper file descriptors for input and output	*/
		tokenizeString(args, &cnt, argStr[i], " ");
		if (i != argcnt - 1)
		{
			pipe(fd);
		}
		pid[i] = fork();
		if (pid[i] == 0)
		{
			if (i != 0)
			{
				dup2(prev, 0);
			}
			if (i != argcnt - 1)
			{
				dup2(fd[1], 1);
			}

			execvp(args[0], args);
			exit(1);
		}
		close(prev);
		close(fd[1]);
		prev = fd[0];
	}

	close(fd[0]);
	close(fd[1]);

	for (i = 0; i < argcnt; i++)
		waitpid(pid[i], NULL, 0);
}

/*
 *	Function: execSerial
 *	Execute multiple commands joined by ";" serially
 *	Parameters:
 *		char**argStr	- array of commands to execute serially
 *		int argc			- number of commands
 *	Returns:
 *		None
 */
void execSerial(char **argStr, int argcnt)
{
	int i, pc, status;
	char *argv[100];
	for (i = 0; i < argcnt; i++)
	{
		tokenizeString(argv, &pc, argStr[i], " ");
		if (fork() == 0)
		{
			execvp(argv[0], argv);
			printf("Error executing %s\n" , argv[0]);
			exit(1);
		}
		else
		{
			wait(&status);
		}
	}
}

/*
 *	Function: execLogicalAND
 *	Conditionally execute multiple commands joined by "&&" serially
 *	Parameters:
 *		char**argStr	- array of commands to execute condtionally
 *		int argc			- number of commands
 *	Returns:
 *		None
 */
void execLogicalAND(char **argStr, int argcnt)
{
	int i, pc, status;
	char *argv[100];
	for (i = 0; i < argcnt; i++)
	{
		tokenizeString(argv, &pc, argStr[i], " ");
		if (fork() == 0)
		{
			execvp(argv[0], argv);
			printf("Error executing %s\n" , argv[0]);
			exit(1);
		}
		else
		{
			wait(&status);

			/*	Stop execution if exit code is non-zero	*/
			if (status != 0)
				break;
		}
	}
}

/*
 *	Function: execLogicalOR
 *	Conditionally execute multiple commands joined by "||" serially
 *	Parameters:
 *		char**argStr	- array of commands to execute condtionally
 *		int argc			- number of commands
 *	Returns:
 *		None
 */
void execLogicalOR(char **argStr, int argcnt)
{
	int i, pc, status;
	char *argv[100];
	for (i = 0; i < argcnt; i++)
	{
		tokenizeString(argv, &pc, argStr[i], " ");
		if (fork() == 0)
		{
			execvp(argv[0], argv);
			printf("Error executing %s\n" , argv[0]);
			exit(1);
		}
		else
		{
			wait(&status);
			/*	Stop execution if exit code is zero	*/
			if (status == 0)
				break;
		}
	}
}

int main(int argc, char **argv)
{
	char buf[500], *buffer[100],
		*args1[100], *args2[100], *token,
		cwd[1024], user[100], host[100];
	char *tmp_in;
	int argcnt = 0;

	// printf("%d\n", argc);

	if (argc > 1)
	{
		freopen(argv[1], "r", stdin);
	}

	gethostname(host, sizeof(host));
	getlogin_r(user, sizeof(user));

	cls();

	printf(RED "----------------\n"
		RED BOLD ">>>> ShellB <<<<\n"
		RESET RED "----------------\n"
		BOLD GREEN "Author:"
		BLUE "Kinjal Ray "
		RESET BLUE "(kinjalray10@gmail.com)\n"
		RESET "\n");

	while (1)
	{
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		{
			sprintf(buf , GREEN BOLD "%s@%s"
				RESET ":"
				BLUE BOLD "%s"
				RESET "=+> ", user, host, cwd);
			tmp_in = readline(buf);
			// Only add non-empty lines to history.
			if (strlen(tmp_in) != 0)
			{
				add_history(tmp_in);
				strcpy(buf, tmp_in);
				free(tmp_in);
			}
			else
			{
				continue;
			}
		}
		else 
			perror("getcwd failed\n");


		//check if only a simple command needs to be executed or multiple piped commands or other types
		if (strstr(buf, "||"))
		{
			//logical OR execution
			tokenizeString(buffer, &argcnt, buf, "||");
			execLogicalOR(buffer, argcnt);
		}
		else if (strchr(buf, '|'))
		{
			//pipe execution
			tokenizeString(buffer, &argcnt, buf, "|");
			executePiped(buffer, argcnt);
		}
		else if (strchr(buf, ';'))
		{
			//serial execution

			tokenizeString(buffer, &argcnt, buf, ";");
			execSerial(buffer, argcnt);
		}
		else if (strstr(buf, "&&"))
		{
			//logical AND execution
			tokenizeString(buffer, &argcnt, buf, "&&");
			execLogicalAND(buffer, argcnt);
		}
		else
		{
			tokenizeString(args1, &argcnt, buf, " ");

			/************************************/
			/*****	Internal Commands Begin	*****/
			/************************************/
			if (strstr(args1[0], "cd"))
			{
				//change directory command
				if (args1[1] == NULL)
				{
					args1[1] = (char*) malloc((6 + strlen(user)) *sizeof(char));
					strcpy(args1[1], "/home/");
					strcat(args1[1], user);
				}
				chdir(args1[1]);
			}
			else if (strstr(args1[0], "pwd"))
			{
				//print working directory command
				printf("%s\n", cwd);
			}
			else if (strstr(args1[0], "exit"))
			{
				//exit builtin command
				exit(0);
			}
			else if (strstr(args1[0], "clear") || strstr(args1[0], "cls"))
			{
				//clear screen command
				cls();
			}
			/***********************************/
			/****** Internal Commands End ******/
			/***********************************/

			else
			{
				execCmd(args1);
			}
		}
		strcpy(buf , "");
	}

	return 0;
}
