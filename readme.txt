SISH
------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------

Table of Contents

	+ sish

	+ Internal Commands
		- show
		- set
		- unset
		- export
		- unexport
		- environ
		- chdir
		- exit
		- wait
		- clr
		- dir
		- echo
		- help
		- pause
		- history
		- repeat
		- kill

	+ Externals
		- Pipes/Forks/ IO Redirection
		- Ampersand (&)

	+ Signals
		- Ignored 
		- Dispositioned

------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------
Name 
	sish

Description
	This is an incomplete shell which indicates that it is missing many features included in other shells.

Synopsis
	sish [-x] [-d <level>] [-f file [arg] … ]

Options

	-x: The command line after variable substitution is displayed before the command is evaluated.  

	-d <DebugLevel>: Output debug messages. DebugLevel=0 means don't output any debug messages. DebugLevel=1 outputs enough infomation so that correct operation can be verified. Larger integers can be used as the user sees fit to indicate more detailed debug messages.

	-f file [Arg] ... Input is from a file instead of stdin; i.e., file is a shell script. If there are arguments, the strings are assigned to the shell variables $1, $2, etc. The location of File follows the rules described in Search Path below.	
	
------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------

Internal Commands
------------------------------------------------------------------------------------------

show

Name

	show - display the word(s) followed by a newline. 

Synopsis

	show [ Word ] [ … ] - shows the variable Word and its value. Additional words could be specified and each would be defined.   

Options

	show must be followed with a word which should be the key (name) of the variable that has been stored in the Shell environment. Any additional words are optional, but must be separated by a space from its previous word in order for the Shell to recognizs. 

--------------------------------------------------------------------------------------------------------------------------------------------

set

Name

	set - set a variable with a certain value in the Shell environment.

Synopsis

	set [Word1] [Word2] 

Options

	set must be followed by exactly two words. The first word is the variable name and the second word would be the value stored with the variable. The two words are separated by a space in order for them to be recognized as 2 different words.

------------------------------------------------------------------------------------------

unset

Name

	unset - unset a previously set local varialbe from the Shell environment.

Synopsis

	unset [Word]

Options

	unset must be followed by exactly one word. The word is the variable name which will be unset from the list of environment variablesof the Shell.

------------------------------------------------------------------------------------------

export

Name

	export - export a global variable with a value to the environment while setting its value

Synopsis

	export [Word1] [Word2]

Options

	export must be followed by exactly two words, separated by a space to indicate that they are indeed two different words. The first word indicates the name of the variable which should be synced with the variable of actual Shell. If the variable does not exist the variable is not exported. The second variable is the value in which would be stored associated with the exported variable. 

------------------------------------------------------------------------------------------

unexport

Name

	unexport - unexport the globle variable from the environment 

Synopsis

	unexport [Word]

Options

	unexport must be followed by exactly one word. The word indicates the name of the variable which will be unexported, or unset from the Shell environment. If the variable does not exist the variable is not unexported. 

------------------------------------------------------------------------------------------

environ

Name

	environ - display to the terminal a listing of all environment strings that ate currentlu defined.

Synopsis

	environ - shows all environment variables

Options

	The environ command takes no additional command arguments and displays all the environment variables associated with the Shell.

------------------------------------------------------------------------------------------

chdir

Name

	Chdir - Changes the current directory of the shell.

Synopsis

	chdir [/path] - sets the absolute path
	chdir [path] - sets a relative path to the current working directory
	chdir [.] - the current directory
	chdir [..] - moves up its parent directory from the current directory
	chdir - sets the working directory to the home directory

Options

	chdir takes an additional command which is the directory to which the working directory will be switched to. The chdir command on its own will set the working directory to the home directory in which the executable for the Shell lies in.

------------------------------------------------------------------------------------------


exit

Name

	exit - exits the Shell and return status. If no value is given, the shell should exit with a status code of 0.

Synopsis

	exit [int i] - exit the shell with i as the return value.

Options

	exit command must be followed by an int value in order for the command to be valid. The shell will then exit and return the int value. As Signals are blocked, this is the only way to properly exit the Shell.

------------------------------------------------------------------------------------------

wait

Name

	wait - waits on the specified process to finish.

Synopsis

	wait [pid_t pid] - pid in the form of int

Options

	wait takes an additional command of type pid_t which is an integer for it to be a valid command. The pid is the id of the process in which the Shell will wait on before continuing with any operations. If the pid is -1, the shell wait for any children to complete. Note that you can't wait on grandchildren.

------------------------------------------------------------------------------------------

clr

Name

	clr - Clear the terminal screen and display a new command line prompt at the top.

Synopsis

	clr - clears the screen.

Options

	The command must only consist of “clr” as any other command syntax would give a command error. 

------------------------------------------------------------------------------------------

dir

Name

	dir - list the contents of the current directory.

Synopsis

	dir - lists content of directory.

Options

	dir takes no additional arguments, otherwise will give a command error. dir on its own is a valid command and will list the items in the directory.


------------------------------------------------------------------------------------------

echo

Name

	echo - display a specific statement on the terminal followed by a new line.

Synopsis

	echo [comment] - display the comment followed by a new line. 

Options

	echo command requires an comment that will be displayed in the terminal as a string. Not following the format will result in a command error.

------------------------------------------------------------------------------------------

help

Name

	help - loads and display this user manual page.

Synopsis

	help - no additional commands.

Options

	help must take only one argument which is “help” with no additional arguments. 

------------------------------------------------------------------------------------------

pause

Name

	pause - pauses the operation of the shell until the 'Enter' key is pressed.

Synopsis

	pause - pauses the operation until 'Enter' key is pressed.

Options

	pause takes no additional arguments. Upon calling pause, the operation of the shell is stopped until 'Enter' key is pressed. 

------------------------------------------------------------------------------------------

history

Name

	history - list to the display the last 100 executed commands.

Synopsis

	history [int n] - n is optional. If n is specified, it shows the last n commands in the history list. Otherwise shows the full history list.

Options

	history by itself lists the full history commands regardless of failure or success of the execution. history followed by an integer lists the past integer amount of commands. 

------------------------------------------------------------------------------------------

repeat

Name

	repeat - repeats a previous command. Doesn’t necessarily have to be the last executed command

Synopsis

	repeat [int n] - Int n is optional. If n is not specified then repeat should execute te previous command. If n is specified, it repeat the command in the history list which corresponds to the nth line.

Options

	repeat must have the syntax of only one argument of 'repeat' or 'repeat' followed by an integer. Any other argument syntax will give a command error. 

------------------------------------------------------------------------------------------

kill

Name

	kill - send a signal to the specified process.

Synopsis

	kill -<signal #> <pid> - send a signal to the specified process.<signal #> is the numeric id associated with the signal to be sent, and <pid> is the process ID number. 

Options

	Must have either 2 or 3 arguments. <signal #> is optional. If signal # is specified, then the kill command should send the SIGTERM signal by default.  

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Externals

------------------------------------------------------------------------------------------

The sish shell supports external commands that would utilize the actual shell commands. Refer to the Bash man page for more details on the different types of commands available to the actual shell.

Pipes / Forks / IO redirection

	sish >> [process] [-d] [Process] [-d] [process] [-d] [process]

	-d signifies the pipe (|) or IO redirections (< or > or >> or <<)

	The sish shell allows up to 4 commands with 3 pipes(or stdin/out redirections) in between. Each command is executed by the child after forking the node and is piped (or IO redirection). 

Ampersand (&)

	Marking a process with an Ampersand(&) mark will make the process continue as a background process. No Ampersand mark will indicate that the process wil be run in the foreground.

------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------

Signals

------------------------------------------------------------------------------------------

Terminal-Generated Signals

	Signals that are dispositioned (redirected) include:
		- SIGINT 
			This signal is generated through the keyboard (<ctrl-c>). This signal is caught and is relayed onto any foreground processes.
		- SIGQUIT
			This signal is generated through thr keyboard (ctrl + \). This signal is caught and is relayed onto any foreground processes.
		- SIGCONT
			This signal is relayed onto child processes.
		- SIGSTOP
			This signal is relayed onto child processes.

Other Signals

	Signals that are ignored:
		- SIGABRT
		- SIGALRM 
		- SIGHUP
		- SIGTERM
		- SIGUSR1
		- SIGUSR2
------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------
