typedef struct
{
	int utilization;
	int capacity;
	int* link;
} dynamic_mem;


void initialize_memory(dynamic_mem* mem);
void destroy_memory(dynamic_mem* mem);
void insert_memory(dynamic_mem* mem, int element);
