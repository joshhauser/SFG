#include "../headers/shell.h"
#include "../headers/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * tests:
 * mv: marche bien
 * ls: marche bien
 * mkdir : marche bien 
 * rmdir : marche bien
 * touch : marche bien 
 * rm : marche bien 
 * move : marche bien 
 * cd : marche bien 
 * df : marche bien 
 * copy : marche bien
 * lsall : marche bien ( a ajouter au man )
 * lsR: marche bien (a ajouter au man)
 * chmod: marche bien ( a ajouter au man ) 
 * echo : marche bien ( a ajouter au man )
 * link et unlink : non achevé
 * */


/*get input containing spaces and tabs and store it in argval*/
int getInput()
{
	int argcount = 0;
    fflush(stdout); // vider le buffer
    input = NULL;
    unsigned int buf = 0 ;
    getline(&input,&buf,stdin);
    argcount = 0; 
    while((argval[argcount] = strsep(&input, " \t\n")) != NULL && argcount < ARGMAX-1)
    {
		if(strcmp(argval[argcount],"") != 0)
		{
			argcount++;
		}
	
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
    char* manstr = "\n |￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣|\n |  MAN:                                                      |\n |     mkdir <nomrepertoire> : creer un repertoire            |\n |     rmdir <nomrepertoire> : supprimer un repertoire        |\n |     touch <nomfichier> : creer un fichier                  |\n |     cd <nomrepertoire> : changer le rep courant            |\n |     ls : liste des fichers                                 |\n |     lsall : liste tous les fichiers + date + droits        |\n |     lsR <fichier> : consulter les droits d'un fichier      |\n |     chmod <fichier> (+/-)droit (droit = r/w/rw)            |\n |     mv source destination : déplacer un fichier/repertoire |\n |     rm <nomfichier> : supprimer un fichier                 |\n |     cp <source> <destination> : copier un fichier          |\n |     echo texte > fichier : ecrire d:ans un fichier         |\n |     cat <fichier> : afficher le contenu d'un fichier       |\n |     df: consulter l'etat du disque                         |\n |     exit                                                   |\n |＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿| \n(\\__/) ||\n(•ㅅ•) || \n/ 　 づ \n ";
    printf("%s",manstr);
}
void exitscreen()
{
	printf("A bientot ! \n");
	printf("  _   _\n \\_/-\\_/\n  )   (\n (     )\n  )   (\n /     \\ \n(   9   ) A BIENTOT !!\n(_\\_____/_)`by Josh,Sarra,Imane,Amine,Miriam\n"); 

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
			exitscreen();
            //printf("exit \n");
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
        else if(strcmp(argval[0],"lsall")==0)
        {
            mylsall();
        }
	    else if(strcmp(argval[0],"lsR")==0)
        {
			lsRights(argval[1]);
        }
        else if (strcmp(argval[0],"chmod")==0)
        {
			chmod(argval[1],argval[2]);
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
		else if(strcmp(argval[0],"echo")==0 )
        {	
			if (funcArgCount>=4)
			{
				char* chaine = malloc((300) * sizeof(char));
				strcpy(chaine,"");
				char* destination = argval[i+1];
				for(i = 1;i<funcArgCount ;i++)
				{
					if ( strcmp(argval[i],">") == 0 )
					{
						destination = argval[i+1];
					}
					strcat(chaine,argval[i]);
					strcat(chaine," ");
				}
				remove_string(chaine,destination);
				if ( strchr(chaine,'>') != NULL )
				{
					remove_string(chaine,">");
					echo(chaine,destination);
				}
				else 
				{
					printf("+---- Error: saisir echo texte > destination\n");
				}
		    }
		    else 
		    {
				printf("+---- Error: saisir echo texte > destination\n");
		    }
		}
		else if (strcmp(argval[0],"cat")==0 )
		{
			cat(argval[1]);
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
