/* CSci4061 F2016 Assignment 1
* login: baran109, id2, id3
* date: 10/05/16
* name: Makar Baranov, full name2, full name3
* id: 5319040, id for second name, id for third name */

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
//3. Deal with the main testcase which has all the .c .o files
//4. Find out about input, do we need this show_target or can we comment it out? also why does their solution echo out the commands even with no -n option set
//5. Test options


//recursive implementation of make function
void make(target_t targets[], target_t* current, int nTargetCount, int* FLAG_B, int* FLAG_n)
{
	//update the status of the current target
	current->nStatus = RUNNING;

	//[Recursive case]
	//check if the target has any dependencies; more than 1 -> recur with those dependencies as new targets if needed
	//otherwise, fork-exec-wait


	if (current->nDependencyCount > 0)
	{	
		//traverse every dependency of the current target
		for (int i = 0; i < current->nDependencyCount; i++)
		{
			
			//find dependency in the array of targets
			int index = find_target(current->szDependencies[i], targets, nTargetCount);
			target_t* dependency = &targets[index];


			//if dependency does not exist as a target and a file
			if ((index == -1) && ((is_file_exist(dependency->szTarget)) == -1))
			{
				//update the status of the current target and print the error
				current->nStatus = INELIGIBLE;
				perror("the dependent target file doesn't exist!");
				exit(-1);
			}


			//check if the dependency was already done or not
			if(dependency->nStatus==FINISHED) continue;


			//check if target exists
			//if it does -> check conditions; otherwise, recurse
			if (is_file_exist(current->szTarget)!=-1)                     
			{

				//check if the -B option is set and compare the timestamps of both files
				//if -B is set or file.c is more recent than file.o -> recurse; otherwise, go to the next dependency/fork-exec-wait
				if ((*FLAG_B) || ((compare_modification_time(current->szTarget, dependency->szTarget)) == 2))
				{
					make(targets, dependency, nTargetCount, FLAG_B, FLAG_n);
				}
				else
					continue;
			}
			else
			{
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
		fprintf(stderr, "pidar%s\n", current->szCommand);
		fprintf(stderr, "status: %d", *FLAG_n);
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

	show_targets(targets, nTargetCount);

	//Now, the file has been parsed and the targets have been named. 
	//You'll now want to check all dependencies (whether they are available targets or files) and 
	//then execute the target that was specified on the command line, along with their dependencies, etc.
	//Else if no target is mentioned then build the first target found in Makefile.

	make(targets, &targets[0], nTargetCount, &FLAG_B, &FLAG_n);

	return EXIT_SUCCESS;
}


