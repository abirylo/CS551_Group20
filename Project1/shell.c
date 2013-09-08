//shell.c

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
#define MAX_PROFILE_SIZE 127
#define MAX_COMMAND_LINE_SIZE 1024

//Golbal Varibles set to Defualts
char PROMPT[MAX_PROFILE_SIZE] = "$ ";
char HOME_DIRECTORY[MAX_PROFILE_SIZE] = "/";
int ALARM_TIME = 5; //seconds

int MAX_ARGUMENTS = 128;

int aliasSize = 0;
char *aliasList[2][MAX_ALIAS_COMMANDS];

int parseProfile(char *path);
int parseLine(const char *commandLine, char **argv);
int evaluate(char **argv, int argc);
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
        }
    }
    
    //read profile.txt
    parseProfile(profilePath);
    
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

int evaluate(char **argv, int argc)
{
    if(argc <= 0)
        return -1;
    
    //do alias checking 
    //if( aliasSize > 0)
    //{
    //    for(int i=0; i<aliasSize; i++)
    //    {
    //        strcmp(aliasList[0,i], arg)
    //    }
    //}
    
    if(strcmp("exit", argv[0]) == 0){
        exit(0);
    }
    else
    {
        printf("Not Valid Command\n");
    }
    
    //find if command is in /bin or /usr/bin
    return 0;
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

