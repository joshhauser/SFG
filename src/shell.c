#include "../headers/shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * trucs a faire :
 * mettre des saisies sécurisées (commande inconnue)
 * changer le pormpt en fonction du repertoire 
 * ls pour afficher suelement le repertoire courant 
 * **/
 

/*get input containing spaces and tabs and store it in argval*/
int getInput()
{
	int argcount = 0;
    fflush(stdout); // vider le buffer
    input = NULL;
    unsigned int buf = 0 ;
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
    return argcount;
}

//message d'acceuil 
void screenfetch()
{
    char* welcomestr = "\n |￣￣￣￣￣￣￣￣|\n |  BIENVENUE     |\n | SUR MYSHELL    | \n |＿＿＿＿＿＿＿＿| \n(\\__/) ||\n(•ㅅ•) || \n/ 　 づ \n (saisir help afin de consulter le manuel) \n";


    printf("%s",welcomestr);
}
void help()
{
    char* manstr = "\n |￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣|\n |  MAN:                                                      |\n |     mkdir <nomrepertoire> : creer un repertoire            |\n |     rmdir <nomrepertoire> : supprimer un repertoire        |\n |     touch <nomfichier> : creer un fichier      |\n |     cd <nomrepertoire> : changer le rep courant            |\n |     ls : liste des fichers                                 |\n |     mv source destination : déplacer un fichier            |\n |     rm <nomfichier> : supprimer un fichier                 |\n |     copy <source> <destination> : copier un fichier        |\n |     write <nomfichier> <texte> : ecrire dans un fichier    |\n |     df                                                     |\n |     exit                                                   |\n |＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿| \n(\\__/) ||\n(•ㅅ•) || \n/ 　 づ \n ";


    printf("%s",manstr);
}

void launch_shell(int argc, char* argv[])
{
	int exitflag = 0;
	screenfetch();
	char *prompt = malloc((116) * sizeof(char));
	strcat(prompt,"[ShellLite]:~$ ");

	int argcount = 0;
	
    while(exitflag==0)
    {
        printf("%s",prompt); 
        argcount = getInput();

        if(strcmp(argval[0],"exit")==0 || strcmp(argval[0],"z")==0)
        {
            printf("exit \n");
            break;
        }
        else if(strcmp(argval[0],"help")==0)
        {
            help();
        }
        else if(strcmp(argval[0],"mv")==0 )
        {
		   move(argval[1],argval[2]);
        }
        else if(strcmp(argval[0],"cd")==0 )
        {
			changeDirectory(argval[1]);
			/*if (strcmp(argval[1],"..")!=0)
			{
				strcat(prompt,"/");
				strcat(prompt,argval[1]);
				strcat(prompt," ");
			}*/
        }
        else if(strcmp(argval[0],"ls")==0)
        {
            testContent();
            myls();
        }
        else if(strcmp(argval[0],"mkdir")==0 )
        {
            createFile(argval[1],'d');
        }
        else if(strcmp(argval[0],"rmdir")==0)
        {
           	removeFolder(argval[1]);
        }
        else if(strcmp(argval[0],"touch")==0)
        {
           	createFile(argval[1],'-');
        }
        else if(strcmp(argval[0],"rm")==0 )
        {
			removeFile(argval[1]);
		}
        else if(strcmp(argval[0],"copy")==0 )
        {
            char* file1 = argval[1];
            char* file2 = argval[2];
            if(argcount > 2 && strlen(file1) > 0 && strlen(file2) > 0)
            {
                copy(argval[1],argval[2]);
            }
            else
            {
                printf("+--- Error in cp : insufficient parameters\n");
            }
		}
		else if(strcmp(argval[0],"write")==0 )
        {
			file_t f = openFile(argval[0], W);
			char *c = argval[2];
			writeFile(f, c, strlen(c));
			closeFile(f);
		}
		else if(strcmp(argval[0],"df")==0 )
		{
			diskFree();
		}
  
	}  
}
