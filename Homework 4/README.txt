/***************************************************************************************************************************************************************/
/* 
	CS450 Operating System - Fall 21
	Programming Assignment 4
	Hassan Alamri 		& 	Timur Gaimakov 
	halamri@hawk.iit.edu		tgaimakov@hawk.iit.edu
	A20473047			A20415319
          
             
 */
/***************************************************************************************************************************************************************/

The purpose from this Assignment is Developing a set of tools that can help to recover a damaged file system. For many reasons, the information in a directory 
file, in an inode etc. may get damaged and cannot be read. As a result, a user program will not be able to access some parts of the file system. In this exercise, 
we ask you to develop a set of tools and show that you can use them to recover a file system if one or more of its directory inode is damaged.

So there will be five programs in this assignment:
1- A program, called the directoryWalker, that prints out the names of each file and directory in a file system tree, starting at a given point in 
	the tree. The program will also write the inodes associated with each file and directory.
2- A program, called the inodeTBWalker, that prints out all the allocated inodes. You obtain this set of inodes from the inode table.
3- A program that will compare the inode related output from the two Walkers.
4- A program that can erase the information (including the block pointers) in a directory inode so that you can test and demonstrate your results.
5- A program that will fix the damaged file system.

So we need to add the new system calls in xv6 "it is acctually xv6-public that we use, but we change the folder name to xv6". When a user program calls them, 
they will satisfy and implement these programs requirments.
/***************************************************************************************************************************************************************/

HOW TO BUILD AND RUN THE CODE
- First of all you should to have and type of linux or unix OS.
- Then follow installation instructions step by step to install toolchain, patched QEMU, and basic tools from this link:
https://www.cs.ucr.edu/~nael/cs153/labs.html

- Open the terimnal and install xv6 repository using this command:
git clone https://github.com/mit-pdos/xv6-public.git xv6

- Then write this statment:
$ make

- After it builds, make and start the emulator by typing in:
$ make qemu-nox

- This should now open the shell starting with $ prompt to type in commands.
- Verify the 5 test programs show up when you type ls
- Before going to the steps please check the screenshots output of the directory with inodes

$ dirWalker
prints out the names of each file and directory in a file system tree.

$ inTBWalker
prints out all the allocated inodes.

$ comWalker
Comparing the inode related output from the two Walkers
Make sure you have run the dirWalker and inTBWalker before running this otherwise it will show no changes

$ damInode [add inode Number]
for checking Damaging the FS by Inode number

$ recInode
Repairing Files

The following sequence will repair any inodes that are damaged:
$ dirWalker
$ inTBWalker
$ comWalker
$ recInode
NOTE: RUN ALL THE COMMANDS IN ORDER

TEST PROGRAM
Here is a sample program that can be used to test the walkers and programs

$ mkdir CS450
$ mkdir CS450/PA4
$ dirWalker
$ inTBWalker
$ comWalker
$ damInode [add CS450's inode]

Repair using:
$ dirWalker
$ inTBWalker
$ comWalker
$ recInode

Verify results:
$ dirWalker
$ inTBWalker
$ comWalker