/* CSci4061 F2016 Assignment 1
* login: baran109, ketel011, trinh020
* date: 10/05/16
* name: Makar Baranov, Drew Ketelsen, Thanh Trinh
* id: 5319040, 4976382, 4578339 */

How to compile using Linux "make" function:
1) Open the shell
2) Get to the directory with all the files
3) Enter "make". The executable file make4061 will appear in the same folder.

How to compile using gcc:
1) Open the shell
2) Get to the directory with all the files
3) Enter "gcc -o make4061 main.c util.c" in the terminal. The executable file make4061 will appear in the same folder.

How to use: ./make4061 [options] [target] : only single target is allowed.
-f FILE:	Read FILE as a makefile.
-h		Print this message and exit.
-n		Don't actually execute commands, just print them.
-B 		Don't check files timestamps.


CREDITS: 
The program was developed by CSCI-4061 Group 17: Makar Baranov, Drew Ketelson and Thanh Trinh.
Students worked together on the coding parts, error handling, testing as well as documentation.

[Breaking down to major contribution points:]
[Makar Baranov: coding, error handling      ]
[Drew Ketelson: coding, testing             ]
[Thanh Trinh: coding, documentation         ]


Addional instructions:
1) Targets will be built if it has a dependency that is more recent than the target.
2) Usage options can be combined.



