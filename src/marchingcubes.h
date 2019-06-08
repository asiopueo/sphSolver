
struct mcvertex
{
	vec3 vertex;
	vec3 normal;
	GLfloat density;
};


struct gridcell
{
	cellvertex* v[8];
};


struct isosurface
{
	GLfloat* density;
	gridcell g;
};



void renderMarchingCubes(implicit_surface* v, float threshold);


