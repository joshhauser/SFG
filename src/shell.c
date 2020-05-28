#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "../headers/shell.h"
#include "../headers/functions.h"
#include "../headers/sgf.h"

// Gets input containing spaces & tabs and store it in argval
int getInput()
{
	int argcount = 0;
	size_t buf = 0;

	fflush(stdout);
	input = NULL;
	getline(&input, &buf, stdin);

	while ((argval[argcount] = strsep(&input, " \t\n")) != NULL && argcount < ARGMAX - 1)
	{
		if (strcmp(argval[argcount], "") != 0)
		{
			argcount++;
		}
	}

	free(input);
	return argcount;
}

// Displays welcoming message
void screenFetch()
{
	char *welcomestr = "\n |￣￣￣￣￣￣￣￣|\n |  BIENVENUE     |\n | SUR MYSHELL    | \n |＿＿＿＿＿＿＿＿| \n(\\__/) ||\n(•ㅅ•) || \n/ 　 づ \n (saisir help afin de consulter le manuel) \n";
	printf("%s", welcomestr);
}

// Displays all the commands and how to use them
void help()
{
	char *manstr = "\n |￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣|\n |  MAN:                                                      |\n |     mkdir <nomrepertoire> : creer un repertoire            |\n |     rmdir <nomrepertoire> : supprimer un repertoire        |\n |     touch <nomfichier> : creer un fichier                  |\n |     cd <nomrepertoire> : changer le rep courant            |\n |     ls : liste des fichers                                 |\n |     lsall : liste tous les fichiers + date + droits        |\n |     lsR <fichier> : consulter les droits d'un fichier      |\n |     chmod <fichier> (+/-)droit (droit = r/w/rw)            |\n |     mv source destination : déplacer un fichier/repertoire |\n |     rm <nomfichier> : supprimer un fichier                 |\n |     cp <source> <destination> : copier un fichier          |\n |     echo texte > fichier : ecrire d:ans un fichier         |\n |     cat <fichier> : afficher le contenu d'un fichier       |\n |     df: consulter l'etat du disque                         |\n |     exit                                                   |\n |＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿| \n(\\__/) ||\n(•ㅅ•) || \n/ 　 づ \n ";
	printf("%s", manstr);
}

// Displays exit message
void exitScreen()
{
	printf("A bientot ! \n");
	printf("  _   _\n \\_/-\\_/\n  )   (\n (     )\n  )   (\n /     \\ \n(   9   ) A BIENTOT !!\n(_\\_____/_)`by Josh, Sarra, Imane, Amine, Mariem\n");
}

// Launches the shell
void launchShell()
{
	int exitflag = 0;
	int i;
	int repcount = 1;
	char *prompt = malloc((100) * sizeof(char));

	char **folder = NULL;
	folder = malloc(10 * sizeof(char *)); // 10 folders

	for (i = 0; i < 10; i++)
	{
		folder[i] = malloc(MAX_FILENAME_CHARS * sizeof(char));
		folder[i] = "";
	}

	folder[0] = MAIN_FOLDER;

	strcpy(prompt, "");
	strcat(prompt, "[ShellLite]:~$ ");
	strcat(prompt, folder[0]);
	strcat(prompt, "\0");

	int funcArgCount = 0;

	screenFetch();
	while (exitflag == 0)
	{
		printf("%s", prompt);
		funcArgCount = getInput();
		if (strcmp(argval[0], "exit") == 0 || strcmp(argval[0], "z") == 0)
		{
			exitScreen();
			break;
		}
		else if (strcmp(argval[0], "help") == 0)
		{
			help();
		}
		else if (strcmp(argval[0], "mv") == 0)
		{
			if ((sizeof(argval[2]) == 0) || funcArgCount != 3)
			{
				nstdError("Saisir mv <source> <destination> ou consulter le manuel via \"help\".");
			}
			else
			{
				move(argval[1], argval[2]);
			}
		}
		else if (strcmp(argval[0], "cd") == 0)
		{
			if (funcArgCount == 1) {
				nstdError("Saisir cd <répertoire> ou consulter le manuel via \"help\".");
			}
			else {
				int var;
				var = changeDirectory(argval[1]);
				if (var != -1 && funcArgCount == 2)
				{
					if (strcmp(argval[1], "..") != 0)
					{
						folder[repcount] = argval[1];
						strcat(prompt, folder[repcount]);
						strcat(prompt, "/");
						repcount++;
					}
					else
					{
						remove_string(prompt, folder[repcount - 1]);
						remove_char(prompt, strlen(prompt) - 1);
						repcount--;
					}
				}
			}
		}
		else if (strcmp(argval[0], "ls") == 0)
		{
			myls();
		}
		else if (strcmp(argval[0], "lsall") == 0)
		{
			mylsAll();
		}
		else if (strcmp(argval[0], "lsR") == 0)
		{
			lsRights(argval[1]);
		}
		else if (strcmp(argval[0], "chmod") == 0)
		{
			if (funcArgCount == 3) chmod(argval[1], argval[2]);
			else nstdError("Saisir chmod <fichier> <droits> ou consulter le manuel via \"help\".");
		}
		else if (strcmp(argval[0], "mkdir") == 0)
		{
			if (funcArgCount != 2)
			{
				nstdError("Saisir mkdir <nom_repertoire> ou consulter le manuel via \"help\".");
			}
			else
			{
				createFile(argval[1], 'd');
			}
		}
		else if (strcmp(argval[0], "rmdir") == 0)
		{
			if (funcArgCount != 2)
			{
				nstdError("Saisir rmdir <nom_repertoire> ou consulter le manuel via \"help\".");
			}
			else
			{
				removeFolder(argval[1]);
			}
		}
		else if (strcmp(argval[0], "touch") == 0)
		{
			if (funcArgCount != 2)
			{
				nstdError("Saisir touch <nom_fichier> ou consulter le manuel via \"help\".");
			}
			else
			{
				createFile(argval[1], '-');
			}
		}
		else if (strcmp(argval[0], "rm") == 0)
		{
			if (funcArgCount != 2)
			{
				nstdError("Saisir rm <nom_fichier> ou consulter le manuel via \"help\".");
			}
			else
			{
				removeFile(argval[1]);
			}
		}
		else if (strcmp(argval[0], "cp") == 0)
		{
			if (funcArgCount == 3) {
				char *file1 = argval[1];
				char *file2 = argval[2];

				if (strlen(file1) > 0 && strlen(file2) > 0)
				{
					copy(argval[1], argval[2]);
				}
			}
			else
			{
				nstdError("Saisir cp <source> <destination> ou consulter le manuel via \"help\".");
			}
		}
		else if (strcmp(argval[0], "echo") == 0)
		{
			if (funcArgCount >= 4)
			{
				char *chaine = malloc((300) * sizeof(char));
				strcpy(chaine, "");
				char *destination = argval[i + 1];
				for (i = 1; i < funcArgCount; i++)
				{
					if (strcmp(argval[i], ">") == 0)
					{
						destination = argval[i + 1];
					}
					strcat(chaine, argval[i]);
					strcat(chaine, " ");
				}
				remove_string(chaine, destination);
				if (strchr(chaine, '>') != NULL)
				{
					remove_string(chaine, ">");
					echoToFile(chaine, destination);
				}
				else
				{
					nstdError("Saisis echo <texte> > <destination> ou consulter le manuel via \"help\".");
				}
			}
			else
			{
				nstdError("Saisis echo <texte> > <destination> ou consulter le manuel via \"help\".");
			}
		}
		else if (strcmp(argval[0], "cat") == 0)
		{
			if (funcArgCount == 2) {
				cat(argval[1]);
			}
			else {
				nstdError("Saisir cat <fichier> ou consulter le manuel via \"help\".");
			}
		}
		else if (strcmp(argval[0], "df") == 0)
		{
			diskFree();
		}
		else
		{
			nstdError("Commande inconnue. Saisir \"help\" afin de consulter le manuel.");
		}
	}
}
