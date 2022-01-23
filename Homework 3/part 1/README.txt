/***************************************************************************************************************************************************************/
/* 
	CS450 Operating System - Fall 21
	Programming Assignment 3 Part 1
	Hassan Alamri 		& 	Timur Gaimakov 
	halamri@hawk.iit.edu		tgaimakov@hawk.iit.edu
	A20473047			A20415319
          
             
 */
/***************************************************************************************************************************************************************/

Part 1: Memory leaks and tools to find them (xv6 not required)
Memory leaks degrades system performance over time and may eventually lead to system crash. The problem happens often and is difficult to detect and correct. 
The purpose of this exercise is to introduce you to some tools that may help you combat this problem. In this exercise, you will need to use the debugging tools 
gdb and valgrind. valgrind helps you to find memory leaks and other insidious memory problems. Please find the following link to download and install the tool: 
					http://valgrind.org/downloads/current.html

/***************************************************************************************************************************************************************/

HOW TO BUILD AND RUN THE CODE
- First of all you should to have any type of linux or unix OS.
- Then follow installation instructions step by step to install valgrind and gdb tools
- Open the terimnal and install valgrind using this command:
$ sudo apt-get install valgrind

- after that install GDB tool using this command:
$ sudo apt-get install gdb

- Then write this statment:
$ make

- Now, there are steps for creating and compiling c program:
$ touch memLeak.c
$ gedit memLeak.c

- After, taking look at the c code example, compile it:
$ gcc memLeak.c -o valMemLeak

- To apply the valgrind on the compiled file, write this command
$ valgrind --leak-check=full --log-file=valgrind-out.txt ./valMemLeak

- Then, open the report valgrind-out.txt to see the memory leak and errors, using this command:
$ cat valgrind-out.txt


