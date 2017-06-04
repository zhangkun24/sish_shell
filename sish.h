#define TRUE 1
#define FALSE !TRUE
#define HISTORY_SIZE 1024
#define BUFFSIZE 1024
#define VNUM 100 // max number of variables
//struct array to hold local variables
struct local_variable{
    char variable_name[100];
    char value[100];
}local[VNUM];
// Shell pid, pgid, terminal modes
static pid_t SISH_PID;
static pid_t SISH_PGID;
static int SISH_IS_INTERACTIVE;
static struct termios SISH_TMODES;

static char* currentDirectory;
extern char** environ;

struct sigaction act_child;
struct sigaction act_int;
struct sigaction act_stop;
struct sigaction act_continue;
struct sigaction act_quit;

struct history{
	int size;
	char cmd[HISTORY_SIZE][BUFFSIZE];
}historylist;


//child process pid
pid_t pid;
int is_terminate;
//restore the exit value of the child (last foreground)
int child_value;
//restore the pid of the last background pid
pid_t back_ground;


/**
 * SIGNAL HANDLERS
 */
// signal handler for SIGCHLD */
void signalHandler_child(int p);
// signal handler for SIGINT
void signalHandler_int(int p);
//siganl handler for SIGTSTP
void signalHandler_stop(int p);
//siganl handler for SIGCONT
void siganlHandler_continue(int p);
//signal handler for SIGQUIT
void signalHandler_quit(int p);


int changeDirectory(char * args[]);
