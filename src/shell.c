#include "../headers/shell.h"
#include "../headers/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * trucs a faire :
 * changer le prompt en fonction du repertoire :fait
 * vérifier le nombre d'arguemnts saisis.: fait
 * ls : marche toujours bizarre au niveau du type de fichier.
 * **/

/**
 * tests:
 * mv: marche bien
 * ls: marche toujour chelou
 * mkdir : marche bien 
 * rmdir : marche bien
 * touch : marche bien 
 * rm : marche bien 
 * move : marche bien mais les fichers dans le documents move a corriger
 * cd : marche bien (modification du prompt a ajouter)
 * df : marche bien 
 * copy : marche bien
 * write : ? j'ai pas capté comment ça marche 
 * link et unlink pas encore fait
 * */

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
		if(strcmp(argval[argcount],"") != 0)
		{
			argcount++;
		}
	
    }
    free(input);
    printf("%d \n ",argcount);
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
    char* manstr = "\n |￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣|\n |  MAN:                                                      |\n |     mkdir <nomrepertoire> : creer un repertoire            |\n |     rmdir <nomrepertoire> : supprimer un repertoire        |\n |     touch <nomfichier> : creer un fichier      |\n |     cd <nomrepertoire> : changer le rep courant            |\n |     ls : liste des fichers                                 |\n |     mv source destination : déplacer un fichier            |\n |     rm <nomfichier> : supprimer un fichier                 |\n |     cp <source> <destination> : copier un fichier        |\n |     write <nomfichier> <texte> : ecrire dans un fichier    |\n |     df                                                     |\n |     exit                                                   |\n |＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿| \n(\\__/) ||\n(•ㅅ•) || \n/ 　 づ \n ";


    printf("%s",manstr);
}

void launch_shell(int argc, char* argv[])
{
	int exitflag = 0;
	int i ;
	int repcount=1;
	char *prompt = malloc((100) * sizeof(char));
	
	//tableau qui va contenir le chemein vers le rep actuel
	char** folder= NULL; 
	folder = malloc(10 * sizeof(char*)); //10 lignes
	for (i=0;i<10;i++)
	{
		folder[i]= malloc(20*sizeof(char));
		folder[i]="";
	}
	folder[0]=MAIN_FOLDER;
	strcpy(prompt,"");
	strcat(prompt,"[ShellLite]:~$ ");
	strcat(prompt,folder[0]);
    strcat(prompt,"\0");
    
	int funcArgCount = 0;
	
	screenfetch();
    while(exitflag==0)
    {
        printf("%s",prompt); 
        funcArgCount = getInput();
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
			if( (sizeof(argval[2])==0) || funcArgCount != 3 ) 
			{
				printf("Erreur. saisir mv <source> <destination> ou consulter le manuel via help\n");
			} 
			else 
			{
				move(argval[1],argval[2]);
			}
        }
        else if(strcmp(argval[0],"cd")==0 )
        {
			int var;
			var=changeDirectory(argval[1]);
			if (var != -1 && funcArgCount == 2)
			{
				if (strcmp(argval[1],"..")!=0)
				{
					folder[repcount] = argval[1];
					strcat(prompt,folder[repcount]);
					strcat(prompt,"/");
					repcount++;
				}
				else 
				{
					remove_string(prompt,folder[repcount-1]);
					remove_char(prompt,strlen(prompt)-1);
					repcount--;
				
				}
			}
			
        }
        else if(strcmp(argval[0],"ls")==0)
        {
            myls();
        }
        else if(strcmp(argval[0],"mkdir")==0 )
        {
			if ( funcArgCount != 2 )
			{
				printf("Erreur. Saisir mkdir <nom_repertoire> ou consulter help\n");
			}
			else
			{
				createFile(argval[1],'d');
			}
        }
        else if(strcmp(argval[0],"rmdir")==0)
        {
			if ( funcArgCount != 2 )
			{
				printf("Erreur. Saisir rmdir <nom_repertoire> ou consulter help\n");
			}
			else
			{
				removeFolder(argval[1]);
			}
        }
        else if(strcmp(argval[0],"touch")==0)
        {
			if ( funcArgCount != 2 )
			{
				printf("Erreur. Saisir touch <nom_fichier> ou consulter help\n");
			}
			else
			{
				createFile(argval[1],'-');
			}
        }
        else if(strcmp(argval[0],"rm")==0 )
        {
			if ( funcArgCount != 2 )
			{
				printf("Erreur. Saisir rm <nom_fichier> ou consulter help\n");
			}
			else
			{
				removeFile(argval[1]);
			}
		}
        else if(strcmp(argval[0],"cp")==0 )
        {
            char* file1 = argval[1];
            char* file2 = argval[2];
            if(funcArgCount > 2 && strlen(file1) > 0 && strlen(file2) > 0)
            {
                copy(argval[1],argval[2]);
            }
            else
            {
                printf("+--- Error in cp : parametres insuffisants \n");
            }
		}
		else if(strcmp(argval[0],"write")==0 )
        {
			file_t f = openFile(argval[1], W);
			char *c = argval[2];
			writeFile(f, c, strlen(c));
			closeFile(f);
		}
		else if(strcmp(argval[0],"df")==0 )
		{
			diskFree();
		}
		else 
		{
			printf("Commande inconnue. Saisir help afin de consulter le manuel \n");
		}
  
	}
}
