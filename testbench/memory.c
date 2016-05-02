// Standard includes
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <assert.h>
using namespace std;



struct dynamic_mem
{
	int utilization;
	int capacity;
	int* link;
};

void initialize_memory(dynamic_mem* mem)
{
	mem->link = (int*) malloc(30*sizeof(int));
	mem->capacity = 30;
	mem->utilization = 0;
}

void destroy_memory(dynamic_mem* mem)
{
	free(mem->link);
	mem->capacity = 0;
	mem->utilization = 0;
}

void addElement(dynamic_mem* mem, int element)
{
	if(mem->utilization == mem->capacity)
	{
		mem->capacity = mem->capacity * 2;
		mem->link = (int*) realloc(mem->link, mem->capacity * sizeof(int));
	}
	mem->link[mem->utilization] = element;
	mem->utilization++;
}

void read_memory(dynamic_mem* mem)
{
	int i;

	for(i=0; i < mem->utilization; i++)
	{
		printf("Element No. %d: %d \n", i, mem->link[i]);
	}
}

void write_memory(dynamic_mem* mem)
{
	int i;

	for(i=3; i<57; i++)
	{
		addElement(mem,i);
	}
}


int main(int argc, char** argv)
{
	dynamic_mem A_test;

	initialize_memory(&A_test);
	write_memory(&A_test);
	read_memory(&A_test);
	destroy_memory(&A_test);

	return 0;
}
