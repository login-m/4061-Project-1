/* CSci4061 F2016 Assignment 1
* login: baran109, id2, trinh020
* date: 10/05/16
* name: Makar Baranov, Drew Ketelsen, Thanh Trinh
* id: 5319040, 4976382, 4578339 */

To compile:
Inlcuded with program files is a make4061 that allows for easy compilation from shell
From the terminal, cd down into the correct directory that contains make4061. Then enter "gcc -o make4061 main.c util.c" in the terminal. Then you can run the program wiht different flag and otpion below:

1. -f filename: filename will be the name of the makefile, otherwise the default name 'makefile' 
2. -n: Only displays the commands that would be run, doesn’t actually execute them
3. -B: Do not check timestamps for target and input 
Example syntax:
1. ./make4061: This will build the first target found in makefile
2. ./make4061 specificTarget : This will build only the specific target
3. ./make4061 -f yourownmakefile
4. ./make4061 -n


We use a recursive make function to check search for dependencies
then check if it needs to be builts (Flag)
check if have any child, if it does wait for child to execute

We work together on coding, error handling, testing and documentation: Drew  Makar Thanh


Other instruction:
	if any problem consist, try -B flag
