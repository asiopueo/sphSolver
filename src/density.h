

typedef struct 
{
	float grid_len;
	float inv_grid_len;

	float* density;

	int width_x;
	int width_y;
	int width_z;
	int N_cells; // superfluous?

	float minx;
	float miny;
	float minz;

} density_grid;


void alloc_density_grid(density_grid* d, vec3* pos, int n_particles, float len);
