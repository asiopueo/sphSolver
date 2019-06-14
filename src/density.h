

struct density_grid
{
	float grid_len;
	float inv_grid_len;

	float* density;

	uint width_x;
	uint width_y;
	uint width_z;
	uint N_cells; // superfluous?

	float minx;
	float miny;
	float minz;
};



// Besser als Klasse definieren?
struct density_stamp
{
	float grid_len;
	float inv_grid_len;

	float* density;

	uint width_x;
	uint width_y;
	uint width_z;
	uint n_cells;
};




int get_density_index(density_grid* d, uint i, uint j, uint k);

void alloc_density_stamp(density_stamp* s, uint length, uint width, uint height, float len, float radius);

void assign_density_to_grid(density_grid* d, density_stamp* stmp, sph_struc* sph);

void alloc_density_grid(density_grid* d, vec3* pos, int n_particles, float len);
