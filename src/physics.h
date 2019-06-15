
typedef struct
{
	int n_particles;	// Number of particles;
	float viscosity;	// Viscosity of fluid
	float smoothlen;	// Smoothing length
	float timestep;		// Timestep
	float stiff;		// Stiffness
	float r_search;		// Search radius of neighbor list

	vec3* normal;		// Normal
	vec3* pos;			// Position
	vec3* vel;			// Velocity
	vec3* force;		// Experienced Force
	float* curvature;  	// Curvature at particle location
	float* density;    	// Density (and its reciprocal?) at particle location
	float mass;       	// Mass of particle
} sph_struc;



void create_sph_instance(sph_struc* sph, int size, vec3* pos, vec3* vel);
void create_sph_instance(sph_struc* sph, int size, vec3* pos, vec3* vel,
						 float smoothlen, float viscosity, float mass, float stiff, float search_radius, float timesteps);
void get_pos(vec3* pos, sph_struc* sph);
void elapse_water(sph_struc* sph, grid_struc* g, neighbor_struc* nbr_list);
