/*
 * Mengtian Li
 * 11/11/2014
 * main.c that execute UM
 */
#include <stdio.h>
#include <stdlib.h>
#include <bitpack.h>
#include "um.h"

int main(int argc, char *argv[])
{
	assert(argc == 2);
	FILE* fp = fopen(argv[1], "rb");
	if(fp){
	        /* execute UM */
		UM_T um = new_UM(fp);
		fclose(fp);
		execute_UM(um);
		free_UM(&um);
	} else {
		fprintf(stderr, "fail to open file.\n");
		fclose(fp);
		exit(1);
	}
	exit(0);
}
