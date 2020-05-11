/*
    Name: Shubham Arya
    ID: 1001650536
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

pid_t fork(void);
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports 11 arguments
#define _GNU_SOURCE
#define max_pid 15

int main(int argc, char *argv[])
{
    char *cmd_str=(char*)malloc(MAX_COMMAND_SIZE);
    int index_history = 0;
    int count = 0;
    pid_t showpids[50];
    pid_t pid = 0 ;
    char *history[50];
    while(1)
    {
        printf ("msh> ");
        
        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while(!fgets (cmd_str, MAX_COMMAND_SIZE, stdin));

        //If user presses enter, it will loop msh> until user enters a value
        
        while(strcmp(cmd_str,"\n")==0)
        {
            printf("msh>");
            while(!fgets (cmd_str, MAX_COMMAND_SIZE, stdin));
        }
        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];
        int token_count = 0;
                                                                  
        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;
        //char *history[15];
        
        //Typing !n will result in your shell running the nth command
        //The token is converted from array to integer and stored in index
        //which is then used to find the element  at the nth posiition
        //and then copied to cmd_str and then used in execl to execute it
        if(strchr(cmd_str,'!')!=NULL)
        {
            int index = atoi(&cmd_str[1]);
            //This is to check if the index enetered by the user exist or not
            if(index > 15 || index > index_history)
            {
                //if it doesn't exists, then cmd_str will have notFound which
                //be used to tell the user that the command in history is not
                //found. It is done to avoid segmentation fault and out of range
                strcpy(cmd_str,"CommandnotFound");
            }
            else
            {
                strncpy(cmd_str,history[index],strlen(history[index]));
            }
        }
        char *working_str  = strdup(cmd_str);

        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;
        
        // Tokenize the input stringswith whitespace used as the delimiter
        while(((arg_ptr=strsep(&working_str,WHITESPACE))!=NULL)&&(token_count<MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
            if( strlen( token[token_count] ) == 0 )
            {
                token[token_count] = NULL;
            }
            token_count++;
        }
        
        //history[] is used to store the contents of token[0] in each loop
        //so that when a user wants to see the history, we can show the contents
        //of the history array
        history[index_history]=strdup(token[0]);
        if((index_history<15))
        {
            //history[index_history]=strdup(token[0]);
            index_history++;
        }
        else
        {
            int i;
            //Once the history goes over 15, new_index is used to readjust the
            //index so that we can have the latest 15 commands that were input
            //This is done so that only the last 15 commands are shown and not
            //all the commands that have been typed
            int new_index = index_history -15;
            for( i=1;i<=15;i++)
            {
                history[i-1]=history[new_index];
                new_index=new_index + 1;
            }
            index_history++;
        }
        
        //This if statement is executed when !n is enetered where n is greater
        //15 or n is not yet a command in history. This is done to avoid
        //segmentation fault and also print out that the command is not found in history
        if(strcmp(token[0],"CommandnotFound")==0)
        {
            printf("Command not found in history.\n");
        }
        //returns 0 when the user wants to exit or quit the shell
        else if((strcmp(token[0],"exit")==0)||(strcmp(token[0],"quit")==0))
        {
            return 0;
        }
        //changes the current directory of the process using chdir()
        else if(strcmp(token[0],"cd")==0)
        {
            chdir(token[1]);
            //just to print the current working directory
            //printf("%s\n", getcwd(directory, 100));
        }
        //goes through the array of history and prints out the contents inside
        //the history[] by iterating through it when user enters history
        else if(strcmp(token[0],"history")==0)
        {
            int i;
            if(index_history < 15)
            {
                for( i=0; i<index_history; i++)
                {
                    printf("%d. %s\n",i,history[i]);
                }
            }
            else
            {
                for( i=0; i<15; i++)
                {
                    printf("%d. %s\n",i,history[i]);
                }
            }
        }
        //goes through the array of showpids of type pid and prints out the contents
        //inside the showpids[] by first checking if the number of pids is greater
        //than 15 or not and then iterating through the array
        else if(strcmp(token[0],"showpids")==0)
        {
            int i=0;
            int showpid_counter=0;
            if(count<max_pid)
            {
               
                for(i=0;i<count;i++)
                {
                    if(showpids[i]<=0||showpids[i]>99999)
                    {
                    }
                    else
                    {
                        showpid_counter++;
                        printf("%d: %d\n",showpid_counter,showpids[i]);
                    }
                }
            }
            else
            {
                for(i=0;i<max_pid;i++)
                {
                    if(showpids[i]<=0 || showpids[i]>99999)
                    {
                    }
                    else
                    {
                        printf("%d: %d\n",i,showpids[i]);
                        showpid_counter++;
                    }
                }
            }
            fflush(NULL);
        }
        else
        {
            pid = fork();
            //As long as count is less than 15, it will store the pid of each process
            //in showpid. But if the count goes over 15, then only the last 15
            //pids of the processes will be stored.
            if(count<max_pid)
            {
                showpids[count] = pid;
            }
            else
            {
                int i;
                for(i=0;i<max_pid;i++)
                {
                    showpids[i] = showpids[i+1];
                }
                showpids[max_pid] = pid;
            }
            if(pid==0)
            {
                execvp(token[0], token);
                //if exec runs succesfully, then the print statement won't print
                //otherwise it'll print out the command not found
                printf("%s: Command not found.\n",token[0]);
                fflush(NULL);
                return 0;
            }
            else
            {
                //waits for the child pid to finish before parent can begin
                int status;
                wait(&status);
                fflush(NULL);
            }
        }
        count++;
        free(working_root);
    }
    return 0;
}
