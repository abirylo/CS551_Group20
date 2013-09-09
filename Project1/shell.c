//shell.c
//#. Requirement                                                Status
//1. Invoke from ash.                                              x
//		./shell
//2. Execute PROFILE file										   x
//		./shell -p .
//3. Define "prompt sign" in PROFILE                               x
//4. Define home directory in PROFILE							   x
//5. Access executables in /bin and /usr/bin
//6. Execute executable without argument.
//7. Execute with output redirection to file.
//8. Execute with output redirection to program.
//9. Alarm after 5 seconds of execution.
//10. Alarm prompts for termination. 
//11. Alarm can be "OFF" in PROFILE(Time=-1)              			x
//12. Alarm can be turned off via alarm off 							x
// 		alarm off
//13. Alias expansion. 												/
// 		Alias listcontent="ls -l | grep "^d""
//14. Alias namespace duplication verification. 						x
// 		Alias listcontent="ls -l | grep "^d""
// 		Alias listcontent="ls -l | grep "^d""
//15. Global namespace duplication verification for aliases.
//16. if/then/else/fi
//17. Terminate only with exit										x
//18. Handle signals where possible (CTRL+C, CTRL+Z)					x
//19. Handle CTRL+D better

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_ALIAS_COMMANDS 256
//Number of aliases allowed to be defined at once.
#define MAX_PROFILE_SIZE 127
//Number of profile variables?
#define MAX_COMMAND_LINE_SIZE 1024
//Unknown.
#define MAX_COMMAND_LINE_CHARACTERS 4096
//Unknown.
#define MAX_ALIAS_ASSIGNMENT_STRING_LENGTH 1024
//The limiting number of characters in an alias assignment string
#define MAX_ALIAS_COMMAND_LENGTH 50
//The maximum number of characters an alias command can have
#define ALARM_DEFAULT_TIME 5
//The default time for the alarm
#define NUM_WORKING_DIRS 3
//Number of working directories (/usr/bin, /bin, and /home)
//Golbal Varibles set to Defualts
char PROMPT[MAX_PROFILE_SIZE] = "$ ";
char HOME_DIRECTORY[MAX_PROFILE_SIZE] = "/";
char USR_BIN_DIR[] = "/usr/bin";
char BIN_DIR[] = "/bin";

int ALARM_TIME = ALARM_DEFAULT_TIME; //seconds

int MAX_ARGUMENTS = 128;

int aliasSize = 0;
char *aliasList[2][MAX_ALIAS_COMMANDS];

char *workingdirs[3];

int parseProfile(char *path);
int parseLine(const char *commandLine, char **argv);

int addAlias(char **argv, int argc);
void run_cmd(char **argv);

int evaluate(char **argv, int argc);
int evalWithAliases(char **argv, int argc);
void usage(void); 

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
void sigquit_handler(int sig);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

int main(int argc, char **argv)
{       
    char *profilePath = "./profile.txt";
    char c;
    // Redirect stderr to stdout
    dup2(1, 2);
    
    // Parse the command line 
    while ((c = getopt(argc, argv, "hp:")) != EOF) {
        switch (c) {
        case 'h':             // print help message 
            usage();
            break;
        case 'p':             // specify the locaton of profile.txt
            profilePath = optarg;
            break;
        default:
            usage();
            break;
        }
    }
    
    //read profile.txt
    parseProfile(profilePath);
    workingdirs[0]=HOME_DIRECTORY;
    workingdirs[1]=USR_BIN_DIR;
    workingdirs[2]=BIN_DIR;
    int i=0;
    char env[MAX_COMMAND_LINE_SIZE];
    strcpy(env,"PATH= ");
    for(i=0;i<NUM_WORKING_DIRS;i++){
    	strcat(env,workingdirs[i]);
    	strcat(env,":");

    }
    printf("Oldenv:%s \n",getenv("PATH"));
    putenv(env);
    printf("Newenv:%s \n",getenv("PATH"));

    //signal handlers
    Signal(SIGINT,  sigint_handler);   // ctrl-c 
    Signal(SIGTSTP, sigtstp_handler);  // ctrl-z 
    Signal(SIGQUIT, sigquit_handler);  // ctrl-slash 
    Signal(SIGCHLD, sigchld_handler);  // Terminated or stopped child 
     

    while(1)
    {
        char commandLine[MAX_COMMAND_LINE_SIZE];
        char *args[MAX_ARGUMENTS];
        
        //print prompt
        printf("%s", PROMPT);
        fflush(stdout);
        
        //read command prompt
        if ((fgets(commandLine, MAX_COMMAND_LINE_SIZE, stdin) == NULL) 
                && ferror(stdin))
        {
            printf("Error Reading Command Line"); //unrecoverable 
            exit(1);
        } 
         
        //parse data
        int count = parseLine(commandLine, args);
        
        //evaluate based on prompt
        evaluate(args, count);
        
        fflush(stdout);
    }   
}

int parseProfile(char *path)
{
    //TODO:
    //may need more checking for path and parsing arguments for robustness
    int error=0;
    
    //check path 
    if(strstr(path, "/profile.txt") == NULL)
    {
        char *temp = "/profile.txt";
        path = strcat(path, temp);
    }
    
    FILE *profileFD = fopen(path, "r"); //open file for read only
    char line[MAX_PROFILE_SIZE];
    
    while(fgets(line, MAX_PROFILE_SIZE, profileFD) != NULL)
    {
        //remove /n and /r at end of line
        char *temp;
        if((temp = strrchr(line, '\r')) != NULL)
        {
            *temp = '\0';
        }
        if((temp = strrchr(line, '\n')) != NULL)
        {
            *temp = '\0';
        }
        char *token;
        char *variable;
        variable = strtok(line, "=");
        token = strtok(NULL, "=");
        if(strcmp("PROMPT", variable) == 0)
        {
            strcpy(PROMPT, token);
        }
        else if(strcmp("HOME_DIRECTORY", variable) == 0)
        {
            struct stat statbuf;
            
            if(stat(token, &statbuf) >= 0 && S_ISDIR(statbuf.st_mode) > 0)
            {
                strcpy(HOME_DIRECTORY, token);
            }
            else
            {
                error++;
                printf("The HOME_DIRECTORY specified %s does not exist.\nUsing defualt path %s.\n", token, HOME_DIRECTORY);
            }
        }
        else if(strcmp("ALARM_TIME", variable) == 0)
        {
            ALARM_TIME = atoi(token);
        }
        else if(strcmp("ALARM", variable) == 0 && strcmp("OFF",token) == 0)
        {
            ALARM_TIME = -1;
        }
        else if(strcmp("ALARM", variable) == 0 && strcmp("ON",token) == 0)
        {
            continue;
        }
        else if(strcmp("MAX_ARGUMENTS", variable) == 0)
        {
            MAX_ARGUMENTS = atoi(token);
        }
        else
        {
            error++;
            printf("The property %s does not exist.\n", variable);
        }
    }
    error += fclose(profileFD);
    
    return error;
}


int parseLine(const char *commandLine, char **argv)
{
    static char array[MAX_COMMAND_LINE_SIZE]; //need to be able to maintain referance to this outside function
    char *buf = array;
    
    strcpy(buf, commandLine);
    buf[strlen(buf)-1] = ' ';  // replace trailing '\n' with space 
    while (*buf && (*buf == ' ')) // ignore leading spaces 
        buf++;
    
    // Build the argv list 
    int argc = 0;
    char *delim = strchr(buf, ' ');

    while(delim && argc < MAX_ARGUMENTS-1) 
    {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) // ignore spaces
            buf++;
        delim = strchr(buf, ' ');
    }
    argv[argc] = NULL; // last argv is null to end list
    
    return argc;
}
int addAlias(char **argv, int argc){
	//Parse line with example:
	//Alias listcontent=ÒlsÓ
	//Alias listcontent="ls -l | grep "
    //pointer to the alias itself (key)
    char *alias;
    //pointer to the command (value)
    char *command;

    char line[MAX_ALIAS_ASSIGNMENT_STRING_LENGTH];
    //lhs is everything after the Alias argument, but before first equal sign
    char lhs[MAX_ALIAS_COMMAND_LENGTH];
    //rhs is everything after first equal sign, including both starting and ending quotes
    char rhs[MAX_ALIAS_ASSIGNMENT_STRING_LENGTH-MAX_ALIAS_COMMAND_LENGTH];


    int i;
    strcpy(line,"");
    for(i=1;i<argc;i++){
    	strcat(line,argv[i]);
    	strcat(line," ");
    }
    char *search="=";
    alias=strtok(line,search);
    strcpy(lhs,alias);
    strcpy(rhs,"");
    int n=0;
    while(1){
    		command=strtok(NULL,search);
    		if(command != NULL){
    			if (n > 0){
    				strcat(rhs,search);
    			}
    			strcat(rhs,command);
    			n++;
    		}
    		else{
    			break;
    		}
    }
    //do alias checking
    if( aliasSize > 0)
    {
        int i;
        for(i=0; i<aliasSize; i++)
        {
           if(strcmp(aliasList[i][0], lhs) == 0){
        	   printf("You already defined %s as an alias with value %s \n",aliasList[i][0],aliasList[i][1]);
        	   return 1;
           }
        }

    }
    //prepare to truncate the  quotes from the rhs
    char cmdbuf[MAX_ALIAS_ASSIGNMENT_STRING_LENGTH-MAX_ALIAS_COMMAND_LENGTH];
    memmove( &cmdbuf , &rhs[0], (strlen(rhs) - 2)) ;
    aliasList[aliasSize][0]=lhs;
    aliasList[aliasSize][1]=cmdbuf+1;
    aliasSize++;

    return 0;
}

int evaluate(char **argv, int argc)
{
    if(argc <= 0)
        return -1;
    
//Certain specific builtins, exit, alarm settings, and Aliases
    if(strcmp("exit", argv[0]) == 0){
        exit(0);
    }
    else if(strcmp("alarm", argv[0]) == 0 && strcmp("off", argv[1]) == 0){
        ALARM_TIME = -1;
        return 0;
    }
    else if(strcmp("alarm", argv[0]) == 0 && strcmp("on", argv[1]) == 0){
        if (argc==3){
        	ALARM_TIME = atoi(argv[2]);
        }
        else{
    		ALARM_TIME = ALARM_DEFAULT_TIME;
        }
    		return 0;
    }
    else if((strcmp("Alias", argv[0]) == 0 || strcmp("alias", argv[0]) == 0 ) && argc >= 1){
        return addAlias(argv,argc);
    }

    if( aliasSize > 0)
    {
    	return evalWithAliases(argv,argc);
    }
    run_cmd(argv);
    //printf("Basic command not found! : %s \n",argv[0]);
    //find if command is in /bin or /usr/bin
    return 0;
}
int evalWithAliases(char **argv, int argc){
	char	 expandedline[MAX_COMMAND_LINE_CHARACTERS];
	    strcpy(expandedline,"");
	        int i,k,found;
	        for (k=0;k<argc;k++){
	        		found=0;
	        		for(i=0; i<aliasSize; i++)
	        		{
	        			printf("looking up if %s = %s, return %d \n",aliasList[i][0],argv[k],strcmp(aliasList[i][0], argv[k]));
	        			if (strcmp(aliasList[i][0], argv[k]) == 0){
	        				strcat(expandedline,aliasList[i][1]);
	        				found=1;
	        				break;
	        			}

	        		}
	        		if (found == 0){
	        			strcat(expandedline,argv[k]);
	        		}
	        		strcat(expandedline," ");

	        }
	        //char **newargv=expandedLinetoArgv(expandedline);
	        //run_cmd(newargv);
	        printf("Expaneded Command not found! : %s \n", expandedline);
	    return 0;
}
void run_cmd(char **argv)
{
     pid_t  pid;
     int    status;

     if ((pid = fork()) < 0) {     /* fork a child process           */
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }
     else if (pid == 0) {          /* for the child process:         */
          if (execvp(*argv, argv) < 0) {     /* execute the command  */
               printf("ERROR: exec failed with command: %s\n", argv[0]);
               exit(1);
          }
     }
     else {                                  /* for the parent:      */
          while (wait(&status) != pid)       /* wait for completion  */
               ;
     }
}
/*****************
 * Signal handlers
 *****************/
/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        printf("Signal error");
    return (old_action.sa_handler);
}
 
/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    return;
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hp:]\n");
    printf("   -h   print this message\n");
    printf("   -p   specify the location of profile.txt\n");
    exit(1);
}

