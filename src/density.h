

typedef struct 
{
	float grid_len;
	float inv_grid_len;

	float* density;

	int width;
	int height;
	int depth;
	int N_cells; // superfluous?

	float minx;
	float miny;
	float minz;

} density_grid;


void alloc_density_grid(density_grid* d, vec3* pos, float len);
