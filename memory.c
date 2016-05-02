// Standard includes
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <assert.h>
using namespace std;

#include "memory.h"


void initialize_memory(dynamic_mem* mem)
{
	mem->link = (int*) malloc(30*sizeof(int));
	if (mem->link == NULL)
		printf("Memory allocation error! \n");
	mem->capacity = 30;
	mem->utilization = 0;
}

void destroy_memory(dynamic_mem* mem)
{
	free(mem->link);
	mem->capacity = 0;
	mem->utilization = 0;
}

void insert_memory(dynamic_mem* mem, int element)
{
	int util = mem->utilization;
	if(util == mem->capacity)
	{
		mem->capacity = mem->capacity * 2;
		mem->link = (int*) realloc(mem->link, mem->capacity * sizeof(int));
		if (mem->link == NULL)
			printf("Memory allocation error! \n");

		printf("Speicher wird verdoppelt! \n");
	}
	mem->link[util] = element;
	mem->utilization++;
}

