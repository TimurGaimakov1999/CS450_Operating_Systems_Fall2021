/*
	Hassan Alamri - (halamri@hawk.iit.edu) -  CWID (A20473047)
	Timur Gaimakov - (tgaimakov@hawk.iit.edu) - CWID (A20415319)
*/

#include <stdio.h>
#include <stdlib.h>

int main () {
	char* string = malloc(8 * sizeof(char));
	// We will not free the memory, so there is a leak
	printf ("\n The Code is Running \n");
	return 0;
}
