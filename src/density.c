
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

using namespace glm;

#include "common.h"
#include "physics.h"
#include "density.h"



int get_density_index(density_grid* d, glm::vec3 v)
{
	int dx = (int) ((v.x - d->minx ) * d->inv_grid_len);
	int dy = (int) ((v.y - d->miny ) * d->inv_grid_len);
	int dz = (int) ((v.z - d->minz ) * d->inv_grid_len);
	return dx + dy * d->width_x + dz * d->width_x * d->width_y;
}

// Still buggy
void alloc_density_stamp(density_stamp* s, uint length, uint width, uint height, float len, float radius)
{
	s->grid_len = len;
	s->inv_grid_len = 1.0/len;

	s->width_x = length;
	s->width_y = width;
	s->width_z = height;
	s->n_cells = length*width*height;

	int l_half = (int) length/2.;
	int w_half = (int) width/2.;
	int h_half = (int) height/2.;

	s->density = (float*) calloc(s->width_x * s->width_y * s->width_z, sizeof(float));

	for (int gx=0; gx < s->width_x; gx++)
	{
		for (int gy=0; gy < s->width_y; gy++)
		{
			for (int gz=0; gz < s->width_z; gz++)
			{
				int index = gx + gy * s->width_x + gz * s->width_x*s->width_y;

				float distsq = s->grid_len * sqrt( std::pow(gx-l_half, 2)+std::pow(gy-w_half, 2)+std::pow(gz-h_half, 2) );

				if (distsq < radius)
					s->density[index] = 0.1;
				else
					s->density[index] = 0.0;

				//std::cout << s->density[index] << std::endl;
			}
		}
	}
}


// The density grid should be larger...
void assign_density_to_grid(density_grid* d, density_stamp* s, sph_struc* sph)
{
	for (int i=0; i<sph->n_particles; i++)
	{
		int dindex = get_density_index(d, sph->pos[i]);

		//std::cout << dindex << std::endl;

		for (int gx=0; gx < s->width_x; gx++)
		{
			for (int gy=0; gy < s->width_y; gy++)
			{
				for (int gz=0; gz < s->width_z; gz++)
				{
					// here is still a bug:
					int offset = gx + gy*d->width_x + gz*d->width_x*d->width_y;
					int stamp_index = gx + gy*s->width_x + gz*s->width_x*s->width_y;

					if ( (dindex+offset < d->N_cells) && (stamp_index < s->n_cells) )
					{ 
						d->density[dindex+offset] += s->density[stamp_index];
					}
					//if ( d->density[dindex+offset] != 0. )
					//	std::cout << d->density[dindex+offset] << std::endl;
				}
			}
		}
	}
}




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

	// debugging purposes only
	const int collar = 1;

	d->minx = fmin_x - collar*d->grid_len;
	d->miny = fmin_y - collar*d->grid_len;
	d->minz = fmin_z - collar*d->grid_len;

	d->width_x = (fmax_x-fmin_x)*d->inv_grid_len + 1 + 2*collar;
	d->width_y = (fmax_y-fmin_y)*d->inv_grid_len + 1 + 2*collar;
	d->width_z = (fmax_z-fmin_z)*d->inv_grid_len + 1 + 2*collar;

	d->N_cells = d->width_x * d->width_y * d->width_z;
	d->density = (float*) malloc(d->N_cells * sizeof(float));

	if (d->density != NULL)
	{
		free(d->density);
	}

	d->density = (float*) calloc(d->N_cells*sizeof(float), sizeof(float));

	//std::cout << "d->N_cells: " << d->N_cells << std::endl;
}
