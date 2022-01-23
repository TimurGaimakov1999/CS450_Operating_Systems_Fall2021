/*
	Hassan Alamri - (halamri@hawk.iit.edu) -  CWID (A20473047)
	Timur Gaimakov - (tgaimakov@hawk.iit.edu) - CWID (A20415319)
*/

#include <stdio.h>
#include <stdlib.h>

void testCase(){
	const int NUM_HEIGHTS = 3;
	int *heights = malloc(NUM_HEIGHTS * sizeof(*heights));
	for (int i = 0; i < NUM_HEIGHTS; i++){
		heights[i] = i * i;
		printf("%d: %d\n", i, heights[i]);
	}
	free (heights);
	printf("# After free the memory:\n");
	heights[0] = 10 ;
	printf("%d: %d\n", 0, heights[0]);
}

void testSolu(){
	const int NUM_HEIGHTS = 3;
	int *heights = malloc(NUM_HEIGHTS * sizeof(*heights));
	for (int i = 0; i < NUM_HEIGHTS; i++){
		heights[i] = i * i;
		printf("%d: %d\n", i, heights[i]);
	}
	printf("# Before free the memory:\n");
	heights[0] = 10 ;
	printf("%d: %d\n", 0, heights[0]); //first do what ever you want with the array before free it
	free (heights);
} 

int main (int argc, char* args[]) {
	testCase();
	//testSolu();
	return 0;
}

