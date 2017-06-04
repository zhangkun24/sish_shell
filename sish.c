/*
 * sish-shell.c
 *
 * CSE422 Project 1
 * Wei Wang (ID: ) & Kun Zhang (ID: )
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include "sish.h"

#define LIMIT 256 // max number command tokens
#define MAXLINE 1024 // max number of characters
/**
 *	Displays the prompt for the shell
 */
void shell_print() {

	printf("sish >> ");
}

/*
 * method to initialize the shell
 */
void init() {

	SISH_PID = getpid();
	// Interactive if STDIN is the terminal
	SISH_IS_INTERACTIVE = isatty(STDIN_FILENO);

	if (SISH_IS_INTERACTIVE) {
		// Loop until in the foreground
		while (tcgetpgrp(STDIN_FILENO) != (SISH_PGID = getpgrp()))
			kill(SISH_PID, SIGTTIN);
		//ignore signals SIGABRT SIGALRM SIGHUB SIGTERM SIGUSR1 SIGUSR2
		signal(SIGABRT,SIG_IGN);
		signal(SIGALRM,SIG_IGN);
		signal(SIGHUP,SIG_IGN);
		signal(SIGTERM,SIG_IGN);
		signal(SIGUSR1,SIG_IGN);
		signal(SIGUSR2,SIG_IGN);
		// Signal handlers
		act_child.sa_handler = signalHandler_child;
		act_int.sa_handler = signalHandler_int;
		act_stop.sa_handler=signalHandler_stop;
		act_continue.sa_handler=siganlHandler_continue;
		act_quit.sa_handler=signalHandler_quit;
		sigaction(SIGCHLD, &act_child, 0);
		sigaction(SIGINT, &act_int, 0);
		sigaction(SIGTSTP,&act_stop,0);
		sigaction(SIGCONT,&act_continue,0);
		sigaction(SIGQUIT,&act_quit,0);
		// make the shell process as the process group leader
		setpgid(SISH_PID, SISH_PID);
		SISH_PGID = getpgrp();
		if (SISH_PID != SISH_PGID) {
			printf("Error, the shell is not process group leader");
			exit(EXIT_FAILURE);
		}
		// set the foreground process group ID
		tcsetpgrp(STDIN_FILENO, SISH_PGID);

		// get the parameters associated with the shell
		tcgetattr(STDIN_FILENO, &SISH_TMODES);

		// get the current directory
		currentDirectory = (char*) calloc(1024, sizeof(char));
	} else {
		printf("Error, the shell is not interactive.\n");
		exit(EXIT_FAILURE);
	}
}
/**
 * Method used initialize the historylist
 */
void init_history() {
	int i;
	for (i = 0; i < 100; i++) {
		memset(historylist.cmd[i], '\0', 100);
	}
}
/**
 * Method used initialize the local variable
 */
void init_local(){
	int i;
	for (i=0; i<VNUM;i++){

		memset(local[i].variable_name,'\0',100);
		memset(local[i].value,'\0',100);
	}
}



/*
 * signal handler for SIGCHLD
 */
void signalHandler_child(int p) {
	// Wait for all dead processes.and store the last background process PID
	while (waitpid(-1, &child_value, 0) > 0) {
		;
	}
}

/*
 * Signal handler for SIGINT
 */
void signalHandler_int(int p) {
	// send a SIGTERM signal to the child process
	if (kill(pid, SIGTERM) == 0) {
		printf("\nProcess %d received a SIGINT signal\n", pid);
		is_terminate = 1;
	} else {
		printf("child process has been terminated!\n");
	}
}
/*
 * Signal handler for SIGSTSP
 */
void signalHandler_stop(int p){
	if (kill(pid,SIGTSTP)==0){
		printf("\n forground process %d is stopted\n",pid);
	}
	else{
		printf("\n no forground process is working\n");
	}
}
/*
 * Signal handler for SIGCONT
 */
void siganlHandler_continue(int p){
	if (kill(pid,SIGCONT)==0){
		printf("\n process %d is continue\n",pid);
	}
	else{
		printf("no foreground process is in stop status");
	}
}
/*
 * Signal handler for SIGQUIT
 */
void signalHandler_quit(int p){
	if(kill(pid,SIGQUIT)==0){
		printf("\n process %d receives a quit signal\n",pid);
	}
}

/*
 * Method to change directory
 */
int changeDirectory(char* args[]) {
	// go to the home directory without input path
	if (args[1] == NULL) {
		chdir(getenv("HOME"));
		return 1;
	}
	// change the directory as input
	else {
		if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
			return -1;
		}
	}
	return 0;
}

/*
 * Set W1 W2' set the value of the local variable W1 to the value W2
 *
 */
int set(char **args) {
	int i,j,is_in=0;

	  for (i=0;i<100;i++){
	    if (strcmp(args[1],local[i].variable_name)==0){
	    		strcpy(local[i].value,args[2]);
	    		is_in=1;
	        break;
	    }
	  }
 if(is_in==0){
	  for (j=0; j<100;j++){
	    if(strcmp(local[j].variable_name,"\0")==0){
	    	strcpy(local[j].variable_name,args[1]);
	    	strcpy(local[j].value,args[2]);
	      break;
	     }
	    }
}
	  	  return 1;
}
/*
 * echo command
 */
int echo_command(char *args[]){
	int i=1;
	while (args[i]!=NULL){
		printf("%s ",args[i]);
		i++;
	}
	printf("\n");

	return 1;
}
/*
 * show command
 */
int show_command(char *args[]){

	//format show w1
if(args[2]==NULL){
	//check for variable substitution
	if(strcmp(args[1],"$$")==0){
		printf("%d\n",(int)SISH_PID);
		return 1;
	}
	else if (strcmp(args[1],"$?")==0){
		printf("%d\n",WEXITSTATUS(child_value));
		return 1;
	}
	else if(strcmp(args[1],"$!")==0){
		printf("%d\n",(int)back_ground);
		return 1;
	}
	//check for local variable
	int is_variable=0;
	int i;
	for (i=0; i<100;i++){
		if (strcmp(args[1],local[i].variable_name)==0){
			is_variable=1;
			break;
		}
	}
	if(is_variable){
		printf("%s\n",local[i].value);
	}
	else{
	printf("there is not a local variable named %s\n",args[1]);
	}
	return 1;
}
//format show w1 w2
else {
	int is_variable_w1=0;
	int index_w1=0;
	int is_variable_w2=0;
	int index_w2=0;
	int i;
	for (i=0; i<100; i++){
		if(strcmp(args[1],local[i].variable_name)==0){
			is_variable_w1=1;
			index_w1=i;
		}
		if(strcmp(args[2],local[i].variable_name)==0){
			is_variable_w2=1;
			index_w2=i;
		}
	}
	if (is_variable_w1 && is_variable_w2){
		printf("%s %s \n",local[index_w1].value,local[index_w2].value);
	}
	//w1 is a local variable and w2 is not a local variable
	else if(is_variable_w1 && !is_variable_w2){
		printf("%s=%s \n",args[1],local[index_w1].value);
	}
	else if (! is_variable_w1 && ! is_variable_w2){
		printf("\n");
	}
	else{
		printf("%s=%s \n",args[2],local[index_w2].value);
	}
	return 1;
}
	}
/*
 * help command (show readme.txt)
 */
int help_command(char *args[]){
	FILE *fp;
	char line[256];
	fp=fopen("readme.txt","r");
	while(fgets(line,256,fp)!=NULL){
		fputs(line,stdout);
	}
	return 1;
}
/*
 * kill -n pid command
 */
int kill_command(char *args[]){
	//format kill -n pid
	if (args[2]!=NULL){
		char *signal_num;
		signal_num=strtok(args[1],"-");
		kill(atoi(args[2]),atoi(signal_num));
	}
	//format kill pid
	else{
		kill(atoi(args[1]),SIGTERM);
	}
	return 1;
}


/*
 * 'unset W1' unset a previously set local variable W1
 */
int unset(char *args[]) {
	  int i,j,temp_index=0;
	  int is_in=0;
	    for (i=0;i<100;i++){
	    if (strcmp(args[1],local[i].variable_name)==0){
	    	  strcpy(local[i].variable_name,"\0");
	    	  strcpy(local[i].value,"\0");
	      is_in=1;
	      temp_index=i;
	      break;
	    }
	  }
	   if(is_in==1){
	    for (j=temp_index;j<99;j++){
	      if (strcmp(local[j+1].variable_name,"\0")==0){
		break;
	      }
	      else{
	    	  local[j]=local[j+1];
		}
	    }
	   }
	   else {
		   printf("not found local variable %s \n",args[1]);
	   }

	  return 1;
}

/**
 * Method used to list the content of current directory
 */
int dir(char *args[]) {
	DIR *dir;
	struct dirent *ent;
	char* path = getcwd(currentDirectory, 1024);
	dir = opendir(path);
	if (dir != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			printf("%s\n", ent->d_name);
		}
		closedir(dir);
	} else {
		printf("Failed to read directory %i", EXIT_FAILURE);
	}
	return 1;

}

/*
 * Method that add the input commands from user to the historylist.cmd
 */
void add_history(char args[]) {
// we define the size of historylist is 100, so when the size is more than 100, it will delete the fist command in the historylist and move the historylist.cmd[i+1] to historylist.cmd[i]
	if (historylist.size == 100) {
		int i;
		for (i = 0; i < HISTORY_SIZE - 1; i++) {
			strcpy(historylist.cmd[i], historylist.cmd[i + 1]);
		}
		strcpy(historylist.cmd[HISTORY_SIZE-1], args);
	}
//if the current of historylist is no more than 100, it will add the command to the historylist.cmd[historylist.size]
	else {
		strcpy(historylist.cmd[historylist.size], args);
		historylist.size++;

	}
}
/**
 * Method used to print the all latest 100 commands from user.
 */
int history(char *args[]) {
	int i;
	for (i = 0; strcmp(historylist.cmd[i], "\0") != 0; i++) {
		printf("%d %s\n", i + 1, historylist.cmd[i]);
	}
	return 1;
}
/**
 * Method used to print the latest n command from user, if the n is more than the number of current historylist.size, it will print all history commands from user.
 */

int history_n(char *args[]) {
	int index;
	//cast the “n” to int n
	int cmd_num = atoi(args[1]);

	if (cmd_num > historylist.size) {
		history(args);
	} else {
		int i = 1;
		for (index = historylist.size - cmd_num - 1;
				index != historylist.size - 1; index++) {
			printf("%d %s\n", i, historylist.cmd[index]);
			i++;
		}
	}
	return 1;
}

/**
 * / 'export' command to set environment variables
 *   e.g export name wei  (set the name with value wei to the environment)
 */
int export(char *args[]) {
	setenv(args[1], args[2], 1);
	return 1;
}
/**
 * / 'unexport' command to undefine environment variables
 *  e.g  unexport name (if variable name is in the envrionment, it will delete the name variable)
 */
int unexport(char *args[]) {
	unsetenv(args[1]);
	return 1;
}
/**
 * / 'environ' display to the terminal a listing of all environment strings that are currently defined
 *
 */

int environ_list(char *args[]) {
	char **env = environ;
	while (*env) {
		printf("%s\n", *env);
		env++;
	}
	return 1;
}
/**
 * 'pause' pause the operation of the shell until the 'Enter' key is pressed
 */

int pause_command(char *args[]) {
	puts("Execution paused, press ENTER to resume");
	while (getchar() != '\n')
		;
	return 1;
}
// 'wait l' the shell waits for process l (a pid) to complete.

void wait_command(char *args[]){
	//if l is -1, the shell waits for any children to compete.
    int status;
	int waitPid;
	waitPid=atoi(args[1]);
	if(waitPid==-1){
		if(wait(NULL)==-1){
			printf("no child process\n");
		}
			}
	else{
		waitpid(waitPid,&status,0);
	}
}

/**
 * Method used to launch a program. (execvp)
 */
void launch_program(char **args, int is_background) {
	if ((pid = fork()) == -1) {
		printf("Child process create error\n");
		return;
	}
	// child process
	if (pid == 0) {
		// set the child process to ignore SIGINT signals
		// parent process handle with signalHandler_int


		// set parent=<pathname>/sish-shell as an environment variable for the child
		setenv("parent", getcwd(currentDirectory, 1024), 1);

		// end the process when launch non-existing commands
		if (execvp(args[0], args) == -1) {

				kill(getpid(), 9);


		}
	}

	// executed by the parent
	else{
	// If not in background, wait for the child to finish.
		//foreground
	if (is_background == 0) {
		waitpid(pid, &child_value, 0);
	} else {
		// the SIGCHILD handler take care of the childs
		back_ground=pid;
		printf("background  process ID is : %d\n", pid);
	}
	}
}

/**
 * Method for managing I/O redirection
 */
void IO_redirection(char * args[],int option,int background) {

	int i=0;
	char *temp[256];
	int fd; // file descriptor
	while(args[i]!=NULL){
		if ((strcmp(args[i],">")==0) || (strcmp(args[i],"<")==0)){
			break;
		}
		temp[i]=args[i];
		i++;
	}
	temp[i]=NULL;

	if ((pid = fork()) == -1) {
		printf("fork error, can’t fork process\n");
		return;
	}
	//child process
	if (pid == 0) {
		//option 0: output redirection
		if (option == 0) {
			//  open the file for write only
			fd = open(args[i+1], O_CREAT | O_TRUNC | O_WRONLY, 0600);
			// replace the standard output with the file
			dup2(fd, STDOUT_FILENO);
			close(fd);
			// Option 1: input and output redirection
		} else if (option == 1) {
			// open file for read only
			fd = open(args[i+1], O_RDONLY, 0600);
			// replace the standard input with the input file
			dup2(fd, STDIN_FILENO);
			close(fd);
			// replace the standard output with the output file
			fd = open(args[i+3], O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fd, STDOUT_FILENO);
			close(fd);
			// Option 2: input redirection
		} else if (option == 2) {
			//open file for read only
			fd = open(args[i+1], O_RDONLY, 0600);
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		// when fork a child process ,we need to add the parent=currentdirectory to the child process
		setenv("parent", getcwd(currentDirectory, 1024), 1);
		// ececvp program
		if (execvp(temp[0], temp) == -1) {
						kill(getpid(), 9);
		}
	}
	//parent process
	else{
	if(background==0){
	waitpid(pid, &child_value, 0);
	}
	//background with symbol "!"
	else{
		back_ground=pid;
		printf("background process pid is %d\n",pid);
		//do nothing
	}

	}

}

/*
 * Method for pipes managing.
 */
void exec_pipe(char * args[],int background) {
	// File descriptors. 0: output, 1: input of the pipe
	int fd[2];
	int fd2[2];

	int num_cmds = 0;  //number of commands separated by ‘|’
	char *cmd[256];
	pid_t pid;
	int end = 0;

	// calculate the number of commands separated by ‘|’
	int l = 0;
	while (args[l] != NULL) {
		if (strcmp(args[l], "|") == 0) {
			num_cmds++;
		}
		l++;
	}
	num_cmds++;

	// configure pipes and replace standard input and output
	int i = 0;
	int j = 0;
	int k = 0;
	while (args[j] != NULL && end != 1) {
		k = 0;
		// use an array of pointers to store the commands
		while (strcmp(args[j], "|") != 0) {
			cmd[k] = args[j];
			j++;
			if (args[j] == NULL ) {
				// when no more arguments
				end = 1;
				k++;
				break;
			}
			k++;
		}
		// the last position of the command be NULL
		cmd[k] = NULL;
		if (strcmp(cmd[k-1],"!")==0){
			cmd[k-1]=NULL;
		}
		j++;

		//  connect the inputs and outputs of two different commands
		if (i % 2 != 0) {
			pipe(fd); // for odd i
		} else {
			pipe(fd2); // for even i
		}

		pid = fork();
		if (pid == -1) {
			if (i != num_cmds - 1) {
				if (i % 2 != 0) {
					close(fd[1]); // for odd i
				} else {
					close(fd2[1]); // for even i
				}
			}
			printf("fork error, can’t fork child process\n");
			return;
		}
		// child process
		if (pid == 0) {
			// the first command
			if (i == 0) {
				dup2(fd2[1], STDOUT_FILENO);
			}
			// the last command, its output in the terminal
			else if (i == num_cmds - 1) {
				if (num_cmds % 2 != 0) {
					dup2(fd[0], STDIN_FILENO);
				} else {
					dup2(fd2[0], STDIN_FILENO);
				}
				// commands in the middle, use two pipes
			} else { // for odd i
				if (i % 2 != 0) {
					dup2(fd2[0], STDIN_FILENO);
					dup2(fd[1], STDOUT_FILENO);
				} else { // for even i
					dup2(fd[0], STDIN_FILENO);
					dup2(fd2[1], STDOUT_FILENO);
				}
			}

			if (execvp(cmd[0], cmd) == -1) {

				kill(getpid(), 9);

			}
		}

		// close file descriptors on patrent
		if (i == 0) {
			close(fd2[1]);
		} else if (i == num_cmds - 1) {
			if (num_cmds % 2 != 0) {
				close(fd[0]);
			} else {
				close(fd2[0]);
			}
		} else {
			if (i % 2 != 0) {
				close(fd2[0]);
				close(fd[1]);
			} else {
				close(fd[0]);
				close(fd2[1]);
			}
		}
		if (background==0){
		waitpid(pid, NULL, 0);
		}
		else {
			//do nothing
			back_ground=pid;
			printf("background process pid is %d \n",pid);
		}
		i++;
	}
}

/*
 * Method to handle the commands entered via the standard input
 */
int exec_command(char * args[]) {
	int j = 0;

	//int fd;  //file descriptor
	//int standardOut;

	int is_background = 0;
	int is_output=0;
	int is_input=0;
	int is_pipe=0;

	//check for I/O redirection ,background, pipe
	while(args[j]!=NULL){
		if (strcmp(args[j],"!")==0){
			is_background=1;
		}
		if(strcmp(args[j],">")==0){
			is_output=1;
		}
		if(strcmp(args[j],"<")==0){
			is_input=1;
		}
		if(strcmp(args[j],"|")==0){
			is_pipe=1;
		}
		j++;
	}

	// 'exit'  exit the shell and return status 0
	if (strcmp(args[0], "exit") == 0 && args[1] == NULL)
	{
		kill(-SISH_PGID,9);
		exit(0);
	}
//exit the shell and return the status with int args[1]
	else if (strcmp(args[0], "exit") == 0 && args[1] != NULL) {
		int i;
		i = atoi(args[1]);
		kill(-SISH_PGID,9);
		exit(i);
	} else if (strcmp(args[0], "dir") == 0) {
		dir(args);
	} else if (strcmp(args[0], "set") == 0) {
		set(args);
	}
		else if(strcmp(args[0],"unset")==0){
			unset(args);
		}
	else if (strcmp(args[0], "history") == 0 && args[1] == NULL) {
		history(args);
	} else if (strcmp(args[0], "history") == 0 && args[1] != NULL) {
		history_n(args);
	}
	else if(strcmp(args[0],"echo")==0){
		echo_command(args);
	}
	else if(strcmp(args[0],"show")==0){
		show_command(args);
	}
	else if(strcmp(args[0],"pause")==0){
		pause_command(args);
	}
	else if(strcmp(args[0],"wait")==0){
		wait_command(args);
	}
	else if(strcmp(args[0],"kill")==0){
		kill_command(args);
	}

	// 'clr' command clears the screen and display a new command line
	else if (strcmp(args[0], "clr") == 0)
		printf("%c%c%c%c%c%c", 27, '[', 'H', 27, '[', 'J');
	// 'chdir’' command change current directory
	else if (strcmp(args[0], "chdir") == 0)
		changeDirectory(args);
	// 'environ'  command display to the terminal a listing of all environment strings
	else if (strcmp(args[0], "environ") == 0) {
		environ_list(args);
	}
	else if(strcmp(args[0],"help")==0){
		help_command(args);
	}
	// 'export’' command to set environment variables
	else if (strcmp(args[0], "export") == 0)
		export(args);
	// 'unexport’' command to undefine environment variables
	else if (strcmp(args[0], "unexport") == 0)
		unexport(args);

	else if(is_pipe){
		exec_pipe(args,is_background);
	}

	//only input
	else if(is_input && !is_pipe && !is_output){
		IO_redirection(args,2,is_background);
		return 1;
	}
	//only output
	else if(is_output && !is_pipe && !is_input ){
		IO_redirection(args,0,is_background);
		return 1;
	}
	//input and output
	else if(is_output && !is_pipe && is_input){
		IO_redirection(args,1,is_background);
		return 1;
	}
	else {
		// launch the program
		launch_program(args, is_background);
	}
	return 1;
}

/*
 * Main method of this sish shell
 */
int main(int argc, char *argv[], char ** envp) {
	char line[MAXLINE]; // input buffer
	char * tokens[LIMIT]; // array for command tokens
	char file[MAXLINE];
	int numTokens;
	int is_batch=0;
	FILE *fp;
	pid = -999; // initialize pid to a invalid number

	init();
	init_history();
	init_local();
	is_terminate = 0;
	//batch model detect
	if(argv[1]!=NULL){
	if (strcmp(argv[1],"-f")==0){
		is_batch=1;
		is_terminate=1;
		strcpy(file,argv[2]);
		fp=fopen(file,"r");
	}
	}


	// We set our extern char** environ to the environment, so that
	// we can treat it later in other methods
	environ = envp;

	// We set shell=<pathname>/simple-c-shell as an environment variable for
	// the child
	setenv("shell", getcwd(currentDirectory, 1024), 1);

	// Main loop, where the user input will be read and the prompt
	// will be printed
	while (TRUE) {
		// We print the shell prompt if necessary
		if (is_terminate == 0)
			shell_print();

		// We empty the line buffer
		memset(line, '\0', MAXLINE);
		//batch model

		// We wait for user input

		if (is_batch==0){
			fgets(line, MAXLINE, stdin);
			is_terminate = 0;
		}else{
			if (fgets(line,MAXLINE,fp)==NULL){
				exit(0);
			}
		}

//if the command is not empty, add it to the historylist
		if (strcmp(line, "\n") != 0) {
			add_history(line);
		}

		// If nothing is written, the loop is executed again
		if ((tokens[0] = strtok(line, " \n\t\"")) == NULL)
			continue;
		// We read all the tokens of the input and pass it to our
		// commandHandler as the argument
		numTokens = 1;
		while ((tokens[numTokens] = strtok(NULL, " \n\t\"")) != NULL)
			numTokens++;
		//detect if the command is "repeat"
		if (strcmp(tokens[0], "repeat") == 0) {
			char line_history[MAXLINE];
			memset(line_history, '\0', MAXLINE);
			char * tokens_history[LIMIT]; // array for the different tokens in the command
			int num_history_tokens;

			if (tokens[1] == NULL) {
				strcpy(line_history, historylist.cmd[historylist.size - 2]);
				tokens_history[0] = strtok(line_history, " \n\t\"");
				num_history_tokens = 1;
				while ((tokens_history[numTokens] = strtok(NULL, " \n\t\""))
						!= NULL)
					num_history_tokens++;
				exec_command(tokens_history);
			} else if (tokens[1] != NULL) {

				int cmd_num;
				cmd_num = atoi(tokens[1]);
				strcpy(line_history, historylist.cmd[cmd_num - 1]);
				tokens_history[0] = strtok(line_history, " \n\t\"");
				num_history_tokens = 1;
				while ((tokens_history[numTokens] = strtok(NULL, " \n\t\""))
						!= NULL)
					num_history_tokens++;
				exec_command(tokens_history);

			}
		}
		//detect if it is the batch model
		/**else if(strcmp(tokens[0],"-f")==0){
			char buf[MAXLINE];
			FILE *fp;
			if((fp=fopen(tokens[1],"r"))==NULL){
				perror("open files error");
				exit(1);
			}
			while(fgets(buf,MAXLINE,fp)!=NULL){

				char *file_tokens[LIMIT];
				int fileTokens;
				if((file_tokens[0] = strtok(buf, " \n\t\""))==NULL){
					continue;
				}
				add_history(buf);
				fileTokens=1;
				while ((file_tokens[fileTokens] = strtok(NULL, " \n\t\"")) != NULL)
					fileTokens++;
				exec_command(file_tokens);
				int i=0;
				while(file_tokens[i]!=NULL){
					memset(file_tokens[i],'\0',100);
					i++;
				}
				memset(buf,'\0',100);
			}

		}**/
		else{
		//if the command is not the "repeat" run exec_command nor "-f" command
		exec_command(tokens);

		}

	}

	exit(0);
}

