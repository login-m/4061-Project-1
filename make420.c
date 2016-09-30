/* CSci4061 F2016 Assignment 1
* login: baran109, id2, id3
* date: 10/05/16
* name: Makar Baranov, Drew Ketelsen, full name3
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
//2. Check the way of execution (had to change some code to pass simple test cases, so might more change)
//4. Find out about input, do we need this show_target or can we comment it out? also why does their solution echo out the commands even with no -n option set
//5. fix the original ./test [options] [target] --- somehow you can't execute specific target 


//recursive implementation of make function
void make(target_t targets[], target_t* current, int nTargetCount, int* FLAG_B, int* FLAG_n)
{
	//update the status of the current target
	current->nStatus = RUNNING;

	//[Recursive case]
	//check if the target has any dependencies; more than 0 -> recur with those dependencies as new targets if needed
	//otherwise, fork-exec-wait

	//debug
	fprintf(stderr, "Target: %s\n", current->szTarget);

	if (current->nDependencyCount > 0)
	{	
		//traverse every dependency of the current target
		for (int i = 0; i < current->nDependencyCount; i++)
		{
			
			//try to find dependency in the array of targets
			int index = find_target(current->szDependencies[i], targets, nTargetCount);

			//if dependency does not exist as a target
			if(index==-1)
			{
				//check if it exists as a file
				if (is_file_exist(current->szDependencies[i]) == -1)
				{
					//if it doesnt, update the status of the current target and print the error
					current->nStatus = INELIGIBLE;
					fprintf(stderr, "the dependent file doesn't exist : %s\n",current->szDependencies[i]);
					exit(-1);
				}
				//if it does, go to the next dependency
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

				//check if it exists as a file
				if(is_file_exist(current->szTarget)!=-1)
				{
					//if it does, compare the timestamps and check -B flag -> true, recurse ; false, go to the next dependency/fork-exec-wait
					if ((*FLAG_B) || (compare_modification_time(current->szTarget, dependency->szTarget) == 2))
						make(targets, dependency, nTargetCount, FLAG_B, FLAG_n);
					else
						continue;
									
				}
				else
					make(targets, dependency, nTargetCount, FLAG_B, FLAG_n);

				                   
			}
			
		}
	}


	//[Base case]
	//Check if -n option is set.
	//It is -> execute; otherwise, just print the command to stdout
	if (!(*FLAG_n)) 
	{

		//fork-exec-wait
		pid_t childpid;
		childpid = fork();

		//handle errors
		if (childpid == -1) 
		{
			current->nStatus = INELIGIBLE;
			perror("failed to fork");
			exit(-1);
		}

		//child's code - execture args
		if (childpid == 0)
		{
			execvp(current->prog_args[0], current->prog_args);  
		}           
		

		//parent's code - wait for a child
		else
		{
			//handle errors
			int status = 0;
			wait(&status);
			if (WEXITSTATUS(status) != 0)
			{
				current->nStatus = INELIGIBLE;
				fprintf(stderr, "child exited with error code=%d\n", WEXITSTATUS(status));
				exit(-1);
			}
			
			//update status
			current->nStatus = FINISHED;
		}
	}

	else
	{
		//Print commands, don't execute
		fprintf(stderr, "%s\n", current->szCommand);
	}
	
}

int main(int argc, char **argv)
{
	target_t targets[MAX_NODES]; //List of all the targets. Check structure target_t in util.h to understand what each target will contain.
	int nTargetCount = 0;

	// Declarations for getopt
	extern int optind;
	extern char * optarg;
	int ch;
	char * format = "f:hnB";

	// Variables you'll want to use
	char szMakefile[64] = "Makefile";
	char szTarget[64] = "";
	int i = 0;

	//Boolean options
	int FLAG_B = 0, FLAG_n = 0;

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
			//Set flag which can be used later to handle this case.
			FLAG_n = 1;
			break;
		case 'B':
			//Set flag which can be used later to handle this case.
			FLAG_B = 1;
			break;
		case 'h':                                                   //case h???
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

	//#####check for empty file
	//#####probably need a more efficient checking - is_file_exist or smth..

	//#####also ask TA if ./make [option] [target] should still execute a default "Makefile" as a file name argument if -f is ommited

	if(targets)
	{
		//find the specified target and start the process
		int index = find_target(szTarget, targets, nTargetCount);
		
		target_t* start = &targets[(index == -1 ? 0 : index)];
		make(targets, start, nTargetCount, &FLAG_B, &FLAG_n);
	}

	return EXIT_SUCCESS;
}


