
typedef struct
{
	float grid_len;             // length of grid edge
	float inv_grid_len;			// inverse grid length

	int* cell_mem;
	int* utilization;

	int width;
	int height;
	int depth;
	int N_cells;

	float minx;
	float miny;
	float minz;
} grid_struc;


typedef struct
{
	int index;
	float distsq;
} nbr_item;


typedef struct
{
	nbr_item* n_matrix;
	int* utilization;
} neighbor_struc;



void alloc_search_grid(grid_struc* g, vec3* pos, int n_particles, float grid_len);
void destroy_search_grid(grid_struc* g);

void create_nbr_list(neighbor_struc* nbr_list);
void add_neighbor(neighbor_struc* nbr_list, int nbr_index, float distsq);
void destroy_nbr_list(neighbor_struc* nbr_list);

void set_neighbors(grid_struc* g, neighbor_struc* nbr_list, vec3* pos, int n_particles);

