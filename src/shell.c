#include "../headers/shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void prompt() {
  system("echo [ShellLite]:~$ ");
}

/*get input containing spaces and tabs and store it in argval*/
void getInput()
{
	int argcount = 0;
    fflush(stdout); // vider le buffer
    input = NULL;
   // ssize_t buf = 0;
    size_t * restrict buf = 0 ;
    getline(&input,&buf,stdin);
    // Copy into another string if we need to run special executables
    input1 = (char *)malloc(strlen(input) * sizeof(char));
    strncpy(input1,input,strlen(input));
    argcount = 0; 
    while((argval[argcount] = strsep(&input, " \t\n")) != NULL && argcount < ARGMAX-1)
    {
        // do not consider "" as a parameter
        if(sizeof(argval[argcount])==0)
        {
            free(argval[argcount]);
        }
        else argcount++;
    }
    free(input);
}

//message d'acceuil 
void screenfetch()
{
    char* welcomestr = "\n |￣￣￣￣￣￣￣￣|\n |  BIENVENUE     |\n | SUR MYSHELL    | \n |＿＿＿＿＿＿＿＿| \n(\\__/) ||\n(•ㅅ•) || \n/ 　 づ \n";


    printf("%s",welcomestr);
}

void launch_shell(int argc, char* argv[])
{
	int exitflag = 0;
	screenfetch();
   
    while(exitflag==0)
    {
      
        printf("[ShellLite]:~$ "); //print user prompt
        getInput();

        if(strcmp(argval[0],"exit")==0 || strcmp(argval[0],"z")==0)
        {
            printf("exit \n");
            break;
        }
        else if(strcmp(argval[0],"screenfetch")==0)
        {
            screenfetch();
        }
        else if(strcmp(argval[0],"mv")==0 )
        {
           printf("mv \n");
        }
        else if(strcmp(argval[0],"pwd")==0 )
        {
            printf("pwd \n");
        }
        else if(strcmp(argval[0],"cd")==0)
        {
            //char* path = argval[1];
           printf("cd \n");
        }
        else if(strcmp(argval[0],"mkdir")==0 )
        {
           // char* foldername = argval[1];
            printf("creer \n");
        }
        else if(strcmp(argval[0],"rmdir")==0)
        {
           // char* foldername = argval[1];
            printf("suppr \n");
        }
        else if(strcmp(argval[0],"clear")==0 )
        {
           printf("clear \n");
        }
        
        else if(strcmp(argval[0],"cp")==0 )
        {
            //char* file1 = argval[1];
            //char* file2 = argval[2];
              printf("cp");
            /*if(argcount > 2 && strlen(file1) > 0 && strlen(file2) > 0)
            {
                function_cp(file1,file2);
            }
            else
            {
                printf("+--- Error in cp : insufficient parameters\n");
            }
			*/
      }
  
	}  
}
