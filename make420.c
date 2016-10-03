/* CSci4061 F2016 Assignment 1
* login: baran109, id2, id3
* date: 10/05/16
* name: Makar Baranov, Drew Ketelsen, Thanh Trinh
* id: 5319040, 4976382, id for third name */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"

void show_error_message(char * lpszFileName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
	fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
	exit(0);
}

//Things to do:
//1. Check against #3 and #4 (Project1.pdf)

//recursive implementation of make function
void make(target_t targets[], target_t* current, int* nTargetCount,  const char* execName, int* FLAG_B, int* FLAG_n, int* disp_updated)
{
	
	//checks if it needs to be built
	int build = 1;

	//update the status of the current target
	current->nStatus = RUNNING;

	//[Recursive case]
	//check if the target has any dependencies; more than 0 -> recur with those dependencies as new targets if needed
	//otherwise, fork-exec-wait
	if (current->nDependencyCount > 0)
	{

		//traverse every dependency of the current target
		for (int i = 0; i < current->nDependencyCount; i++)
		{
			
			//try to find dependency in the array of targets
			int index = find_target(current->szDependencies[i], targets, *nTargetCount);

			//if dependency does not exist as a target
			if(index==-1)
			{
				//check if it exists as a file
				if (is_file_exist(current->szDependencies[i]) == -1)
				{
					//if it doesnt, update the status of the current target and print the error
					current->nStatus = INELIGIBLE;
					fprintf(stderr, "%s: *** The dependent file '%s' doesn't exist. Stop.\n",execName, current->szDependencies[i]);
					exit(-1);
				}

				//go to next dependency
				continue;
			}
			//if dependency does exist as a target
			else
			{
				//initialize dependency pointer for clarity
				target_t* dependency = &targets[index];

				//check if the dependency was already processed
				if(dependency->nStatus==FINISHED) 
					continue;

				//make the dependency a new target
				make(targets, dependency, nTargetCount, execName, FLAG_B, FLAG_n, disp_updated);
		                   
			}
			
		}
	}

	//[Base case]
	//check if you need to compare modification times and update the build flag if needed
	if ((!*(FLAG_B)) && is_file_exist(current->szTarget)!=-1)
	{
		//checks if any of the dependencies is more recent than the current target
		int status, count = 0;
		for (int i=0; i<current->nDependencyCount;++i)
		{
			status = compare_modification_time(current->szTarget,current->szDependencies[i]);
 			if(status == 1 || status == 0)
				count++;			
					
		}
		
		//if some of the dependencies need to be built, set the build flag
		build = (count == current->nDependencyCount ? 0 : 1);
	}

	//Display up-to-date info if needed
	if(!build && *disp_updated) 
		fprintf(stderr,"%s: '%s' is up to date.\n", execName, current->szTarget);
	
	//forx-exec-wait 
	if (build && (!(*FLAG_n)))
	{
		//print out the command		
		fprintf(stderr, "%s\n", current->szCommand);
		
		pid_t childpid;
		childpid = fork();

		//handle errors
		if (childpid == -1) 
		{
			current->nStatus = INELIGIBLE;
			fprintf(stderr,"%s: *** Failed to fork. Stop.",execName);
			exit(-1);
		}

		//child's code - execute the command
		if (childpid == 0)
		{
			execvp(current->prog_args[0], current->prog_args);  
		}           
		

		//parent's code - wait for the child
		else
		{
			//handle errors
			int status = 0;
			wait(&status);
			if (WEXITSTATUS(status) != 0)
			{
				current->nStatus = INELIGIBLE;
				fprintf(stderr, "%s: *** Child exited with error code=%d. Stop.\n", execName, WEXITSTATUS(status));
				exit(-1);
			}
			
			//update status
			current->nStatus = FINISHED;
		}
	}

	//Print the commands that would be built, don't execute
	else if (*FLAG_n && build)
	{
		fprintf(stderr, "%s\n", current->szCommand);
	}
	
}

int main(int argc, char **argv)
{
	target_t targets[MAX_NODES]; //List of all the targets. Check structure target_t in util.h to understand what each target will contain.
	int nTargetCount = 0;
	
	// Declarations for getopt./tes
	extern int optind;
	extern char * optarg;
	int ch;
	char * format = "f:hnB";

	// Variables you'll want to use
	char szMakefile[64] = "Makefile";
	char szTarget[64] = "";
	char* execName = argv[0]+2;
	int i = 0;
	
	//Boolean options
	int FLAG_B = 0, FLAG_n = 0;
	int disp_updated = 0;

	//init Targets 
	for (i = 0; i < MAX_NODES; i++)
	{
		targets[i].pid = 0;
		targets[i].nDependencyCount = 0;
		strcpy(targets[i].szTarget, "");
		strcpy(targets[i].szCommand, "");
		targets[i].nStatus = READY;
	}

	while ((ch = getopt(argc, argv, format)) != -1)
	{
		switch (ch)
		{
		case 'f':
			strcpy(szMakefile, strdup(optarg));
			break;
		case 'n':
			FLAG_n = 1;
			break;
		case 'B':
			FLAG_B = 1;
			break;
		case 'h':                                                  
		default:
			show_error_message(argv[0]);
			exit(1);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 1)
	{
		show_error_message(argv[0]);
		return EXIT_FAILURE;
	}

	/* Parse graph file or die */
	if ((nTargetCount = parse(szMakefile, targets)) == -1)
	{
		return EXIT_FAILURE;
	}

	//Setting Targetname
	//if target is not set, set it to default (first target from makefile)
	if (argc == 1)
	{
		strcpy(szTarget, argv[0]);
		disp_updated = 1;
	}
	else
	{
		strcpy(szTarget, targets[0].szTarget);
	}

	//show_targets(targets, nTargetCount);

	//Now, the file has been parsed and the targets have been named. 
	//You'll now want to check all dependencies (whether they are available targets or files) and 
	//then execute the target that was specified on the command line, along with their dependencies, etc.
	//Else if no target is mentioned then build the first target found in Makefile.

	if(targets)
	{
		//find the specified target and start the process
		int index = find_target(szTarget, targets, nTargetCount);
		if(index == -1) 
		{
			fprintf(stderr,"%s: *** No rule to make target '%s'. Stop.\n",execName,szTarget);
			exit(-1);
		}
		target_t* start = &targets[index];
		make(targets, start, &nTargetCount, execName, &FLAG_B, &FLAG_n, &disp_updated);
	}

	return EXIT_SUCCESS;
}


