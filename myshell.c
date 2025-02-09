#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

#define CWD_MAXSIZE 1024
#define MAX_TOKENS 100

// Custom signal handler for SIGINT (Ctrl + C)
void handle_sigint(int sig) {}

// Custom signal handler for SIGTSTP (Ctrl + Z)
void handle_sigtstp(int sig) {}

void parseInput(char *input, char ** tokens, int *num_tokens)
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
    char *token;
    int count = 0;

	if (strlen(input) < 1) return;

    while ((token = strsep(&input, " ")) != NULL) {
        if (*token != '\0') {  // Skip empty tokens
            tokens[count++] = token;
        }
    }

	tokens[count] = NULL;

    *num_tokens = count;  // Set the number of tokens found
}

void executeCommand(char ** args, int args_len)
{
	// This function will fork a new process to execute a command

	if (strcmp(args[0], "cd") == 0) {
		if (args_len < 2) {
	   	 printf("Shell: cd: missing argument\n");
		} 
		else if(args_len > 2){
			printf("Shell: cd: too many arguements\n");
		}
		else if (chdir(args[1]) != 0) {
		    perror("Shell: cd");
		}
	}
	else if (strcmp(args[0], "\n") == 0) {}
	else if (strcmp(args[0], "exit") == 0) {
		printf("Exiting shell...\n");
		exit(0);
	}
	else {
		int child_pid = fork();
		if (child_pid < 0) {
			perror("Fork failed");
			exit(1);
		} 
		else if (child_pid == 0) {
			if (execvp(args[0], args) < 0) {
				printf("Shell: Incorrect command\n");
				exit(1);
			}
		} 
		else {
			wait(NULL);
		}
	}
}

void executeParallelCommands(char ** args, int args_len)
{
	// This function will run multiple commands in parallel
	int found = 0;

	for (int i = 0; i < args_len && !found; i++) {
		if (strcmp(args[i], "&&") == 0) {
			found = 1;
			args[i] = NULL;

			if (strcmp(args[0], "cd") == 0) {
				executeCommand(args, i);
				executeParallelCommands(args+i+1, args_len-i-1);
			}
			else {
				int child_pid = fork();
				if (child_pid < 0) {
					perror("Fork failed");
					exit(1);
				} 
				else if (child_pid == 0) {
					executeCommand(args, i);
					exit(0);
				} 
				else {
					executeParallelCommands(args+i+1, args_len-i-1);
					wait(NULL);
				}
			}
		}
	}
	if (!found)
		executeCommand(args, args_len);
}

void executeSequentialCommands(char ** args, int args_len)
{	
	// This function will run multiple commands in sequence
	int found = 0;

	for (int i = 0; i < args_len && !found; i++) {
		if (strcmp(args[i], "##") == 0) {
			found = 1;
			args[i] = NULL;

			executeCommand(args, i);
			executeSequentialCommands(args+i+1, args_len-i-1);
		}
	}

	if (!found)
		executeCommand(args, args_len);
}

void executeCommandRedirection(char ** args, int args_len)
{
	// This function will run a single command with output redirected to an output file specificed by user
	int i;
	for (i = 0; strcmp(args[i], ">") != 0 && i < args_len; i++) {}

	args[i] = NULL;
	char * outfile = strcat(strdup("./"), args[i+1]);

	if (strcmp(args[0], "cd") == 0) {
		if (args_len < 2) {
	   	 printf("Shell: cd: missing argument\n");
		} 
		else if(args_len > 2){
			printf("Shell: cd: too many arguements\n");
		}
		else if (chdir(args[1]) != 0) {
		    perror("Shell: cd");
		}
	}
	else if (strcmp(args[0], "\n") == 0) {}
	else if (strcmp(args[0], "exit") == 0) {
		printf("Exiting shell...\n");
		exit(0);
	}
	else {
		int child_pid = fork();
		if (child_pid < 0) {
			perror("Fork failed");
			exit(1);
		} 
		else if (child_pid == 0) {
			close(STDOUT_FILENO); 
			open(outfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
			if (execvp(args[0], args) < 0) {
				printf("Shell: Incorrect command\n");
				exit(1);
			}
		} 
		else {
			wait(NULL);
		}
	}
}

void executePipelineCommands(char ** args, int args_len) {
	char * args_copy[MAX_TOKENS];
	int args_copy_len = 0;
	int i, last_pipe;

	for (i = 0; i < args_len && strcmp(args[i], "|") != 0; i++) {
		args_copy[args_copy_len++] = strdup(args[i]);
	}
	last_pipe = i;
	if (i < args_len) {
		args_copy[args_copy_len++] = strdup(">");
		args_copy[args_copy_len++] = strdup("tmp.out");
		args_copy[args_copy_len] = NULL;
		executeCommandRedirection(args_copy, args_copy_len);

		while (i < args_len-1 && strcmp(args[i+1], "|") != 0) {
			args[i] = args[i+1];
			i++;
		}

		FILE *sourceFile, *destFile;
		char ch;

		// Open the source file in read mode
		sourceFile = fopen("tmp.out", "r");
		if (sourceFile == NULL) {
			perror("Error opening source file");
			return;
		}

		// Open the destination file in write mode
		destFile = fopen("tmp1.out", "w");
		if (destFile == NULL) {
			perror("Error opening destination file");
			fclose(sourceFile); // Close the source file
			return;
		}

		// Copy the contents of the source file to the destination file
		while ((ch = fgetc(sourceFile)) != EOF) {
			fputc(ch, destFile);
		}

		// Close the files
		fclose(sourceFile);
		fclose(destFile);

		args[i] = strdup("tmp1.out");
		executePipelineCommands(args+last_pipe, args_len-last_pipe);
	}
	else {
		args_copy[args_copy_len] = NULL;
		executeCommand(args_copy, args_copy_len);
		remove("tmp.out");
		remove("tmp1.out");
	}
	
}

int main()
{
	// Initial declarations
	char cwd[CWD_MAXSIZE];
	char * input_string = NULL;
	size_t input_len = 0;
	ssize_t input_read_len;
	int num_tokens;
	char ** tokenized_input = malloc(MAX_TOKENS * sizeof(char *));
	char * input_copy;

	signal(SIGINT, handle_sigint);   // Handle Ctrl + C
    signal(SIGTSTP, handle_sigtstp); // Handle Ctrl + Z

	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			printf("%s", cwd);
		}
		printf("$");

		// accept input with 'getline()'
		input_read_len = getline(&input_string, &input_len, stdin);
		if (input_read_len > 1) input_string[input_read_len-1] = '\0';

		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		input_copy = strdup(input_string);
		parseInput(input_copy, tokenized_input, &num_tokens);	
		
		if(strcmp(tokenized_input[0], "exit") == 0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}

		if (num_tokens < 1) continue;
		
		if(strstr(input_string, "&&") != NULL)
			executeParallelCommands(tokenized_input, num_tokens);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(strstr(input_string, "##") != NULL)
			executeSequentialCommands(tokenized_input, num_tokens);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(strstr(input_string, ">") != NULL)
			executeCommandRedirection(tokenized_input, num_tokens);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else if(strstr(input_string, "|") != NULL)
			executePipelineCommands(tokenized_input, num_tokens);
		else
			executeCommand(tokenized_input, num_tokens);		// This function is invoked when user wants to run a single commands		

		free(input_copy);
	}
	
	return 0;
}
