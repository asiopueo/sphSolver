
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

void get_cellvertices(gridcell& cell, float* density, float stride,	int width, int height, int xn, int yn, int zn);

void polygonize_cell(gridcell* grid, std::vector<vec3>& vertex_data, std::vector<vec3>& normal_data, float isolevel);


