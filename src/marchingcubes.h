
struct cellvertex
{
	vec3 vertex;
	vec3 normal;
	GLfloat density;
};


struct gridcell
{
	cellvertex v[8];
};


/*struct isosurface
{
	GLfloat* density;
	gridcell g;
};*/

void get_cellvertices(gridcell& cell, density_grid& density, int xn, int yn, int zn);

void polygonize_cell(gridcell* grid, std::vector<vec3>& vertex_data, std::vector<vec3>& normal_data, float isolevel);

void polygonize_density(density_grid& density, std::vector<vec3>& vertex_data, std::vector<vec3>& normal_data, float isolevel);

