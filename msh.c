// The MIT License (MIT)
// 
// Copyright (c) 2023 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>


#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 128    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell currently only supports one argument

static void handle_signal (int sig )
{
  printf ("Caught signal %d\n", sig );
}

int main(int argc, char *argv[])
{

  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;                                         
                                                           
    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this for loop and replace with your shell functionality [DID THIS PART]

      int pfd[2];
      char buffer;
 
      if(pipe(pfd) == -1)
      {
        perror("pipe");
        exit(EXIT_FAILURE);
      }

      //if my token is not empty check to see if its first element is either quit or exit
      //if so exit the program
    pid_t my_pid = fork();//create a child pid
    for(int mytok = 0; mytok<token_count;mytok++)
    {
      if( token[0] != NULL)
      {
        int n=0;
        if(strcmp( token[0], "exit")==0 || strcmp( token[0], "quit")==0)
        {
          exit(0);
        }
        if(strcmp( token[0], "cd")== 0)
        {
          chdir(fgets(&buffer, n, stdin));
        }

        if(my_pid == 0)//here we are running the child pid
        {
          //passes a list of comand line arguments to function as an array of *
          int ret = execvp( token[0], token);
          //if somehow less that 0 arguments are passed execl didn't funtion correctyly
          if( ret == -1 )
          {
            perror("execvp failed: ");
            exit(-1);
          }  

          close(pfd[1]);
          while(read(pfd[0], &buffer, 1)>0)
          {
            write(STDOUT_FILENO, &buffer, 1);
          }

          write(STDOUT_FILENO, "\n", 1);
          close(pfd[0]);
          _exit(EXIT_SUCCESS);          

          for(int i =1; i<mytok; i++)
          {
            if(strcmp(token[i], ">")==0)
            {
              int fd = open(token[i+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
              if(fd < 0)
              {
                perror("Can't open output file");
                exit(0);
              }
              dup2(fd, 1);
              close(fd);
              token[i] = NULL;
            }
          }
          execvp( token[1], &token[1] );
        } 
        else
        {
          close(pfd[0]);
          write(pfd[1], token[0], strlen(token[0]));
          close(pfd[1]);
        }
        if(my_pid > 0)//parent is running
        { 
          int status;
          waitpid(my_pid, &status, 0); //wait for child pid to finish running
          
        }
        
      }

      struct sigaction act;
 
  
      memset (&act, '\0', sizeof(act));
 
  /*
    Set the handler to use the function handle_signal()
  */ 
      act.sa_handler = &handle_signal;
 
  /* 
    Install the handler and check the return value.
  */ 
      if (sigaction(SIGINT , &act, NULL) < 0) 
      {
        perror ("sigaction: ");
        return 1;
      }

      while (1) 
      {
        sleep (1);
      }
    }

    // Cleanup allocated memory
    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      if( token[i] != NULL )
      {
        free( token[i] );
      }
    }

    free( head_ptr );

  }

  free( command_string );

  return 0;
  // e1234ca2-76f3-90d6-0703ac120003
}
