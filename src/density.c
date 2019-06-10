
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

using namespace glm;

#include "density.h"



/*int get_density_index(density_grid* d, int i, int j, int k)
{
	int index;

	index = i + j * d->width_y + k * d->width_y * d->width_z;

	return index;
}


void create_density_stamp()
{
	r->stamp = (float*) calloc(width*width*width);

	for (gx=-stamp_width; gx <= stamp_width; gx++)
	{
		for (gy=-stamp_width; gy <= stamp_width; gy++)
		{
			for (gz=-stamp_width; gz <= stamp_width; gz++)
			{
				if (distsq < hsq)
					stamp[] = 1.0f;
				else
					stamp[] = 0.0f;
			}
		}
	}
}




float add_density_stap(int gindex, vec3 pos)
{
	int = (int) (pos.x - minx)*r->inv_res;
	int = (int) (pos.y - miny)*r->inv_res;
	int = (int) (pos.z - minz)*r->inv_res;

	for (int i=0; i < N_stamp; i++)
	{
		vec3 tmp = stamp_pos[i] - pos;
		distsq = dot(tmp,tmp);
		if (distsq < h*h)
			density += ;
	}
}




void density_stamp(density_grid* d, int gindex, int gx, int gy, int gz)
{
	d->stamp

	for (int i=0; i< sph->n_particles; i++)
	{
		for (gx=-stamp_width; gx <= stamp_width; gx++)
		{
			for (gy=-stamp_width; gy <= stamp_width; gy++)
			{
				for (gz=-stamp_width; gz <= stamp_width; gz++)
				{
					if( (gindex < 0)|| (gindex < r->MC_cells))
						r->density[gindex] += add_density_stamp(gindex,pos[i]);
				}
			}
		}
	}
}*/


void alloc_density_grid(density_grid* d, vec3* pos, int n_particles, float len)
{
	float fmin_x;
	float fmax_x;
	float fmin_y;
	float fmax_y;
	float fmin_z;
	float fmax_z;
	
	fmin_x = fmin_y = fmin_z = 1000;
	fmax_x = fmax_y = fmax_z = -1000;

	d->grid_len = len;
	d->inv_grid_len = 1.0/len;

	for (int i = 0; i < n_particles; i++)
	{
		vec3* p = &pos[i];

		if (fmin_x > p->x)
			fmin_x = p->x;
		if (fmax_x < p->x)
			fmax_x = p->x;
		if (fmin_y > p->y)
			fmin_y = p->y;
		if (fmax_y < p->y)
			fmax_y = p->y;
		if (fmin_z > p->z)
			fmin_z = p->z;
		if (fmax_z < p->z)
			fmax_z = p->z;
	}

	d->minx = fmin_x;
	d->miny = fmin_y;
	d->minz = fmin_z;

	d->width_x = (fmax_x-fmin_x)*d->inv_grid_len + 1;
	d->width_y = (fmax_y-fmin_y)*d->inv_grid_len + 1;
	d->width_z = (fmax_z-fmin_z)*d->inv_grid_len + 1;

	d->N_cells = d->width_x * d->width_y * d->width_z;
	d->density = (float*) malloc(d->N_cells * sizeof(float));

	if (d->density != NULL)
	{
		free(d->density);
	}

	d->density = (float*) malloc(d->N_cells*sizeof(float));

	std::cout << "d->N_cells: " << d->N_cells << std::endl;

	/*int gx, gy, gz;

	for (gx=-stamp_width; gx <= stamp_width; gx++)
	{
		for (gy=-stamp_width; gy <= stamp_width; gy++)
		{
			for (gz=-stamp_width; gz <= stamp_width; gz++)
			{
				index = get_density_index(gx,gy,gz);
				density[index] += density_stamp( );
			}
		}
	}*/
}
