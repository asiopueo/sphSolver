
#include <memory.h>
#include <stdio.h>
#include <assert.h>
using namespace std;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include "memory.h"
#include "common.h"


#define CUBE_LEN_X 10
#define CUBE_LEN_Y 10
#define CUBE_LEN_Z 10
#define SCAL_LEN 0.1f
#define N_PARTICLES (CUBE_LEN_X*CUBE_LEN_Y*CUBE_LEN_Z)
#define CELL_CAP 50

int get_grid_index(grid_struc* g, vec3 pos)
{
	int gx, gy, gz;
	int gindex;

	gx = (int)((pos.x - g->minx) * g->inv_grid_len);
	gy = (int)((pos.y - g->miny) * g->inv_grid_len);
	gz = (int)((pos.z - g->minz) * g->inv_grid_len);

	gindex = gx + gy*g->width + gz*g->width*g->height;

	return gindex;
}


float clamp(float min, float x, float max)
{
	if (x <= min)
		return min;
	if (x >= max)
		return max;
	return x;
}


void alloc_search_grid(grid_struc* g, vec3* pos, int n_particles, float grid_len)
{
	int i;
	float fmin_x;
	float fmax_x;
	float fmin_y;
	float fmax_y;
	float fmin_z;
	float fmax_z;

	g->grid_len = grid_len;
	g->inv_grid_len = 1.0f/grid_len;

	fmin_x = fmin_y = fmin_z = 1000.f;
	fmax_x = fmax_y = fmax_z = -1000.f;

	for (i = 0; i < n_particles; i++)
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

	g->minx = fmin_x;
	g->miny = fmin_y;
	g->minz = fmin_z;

	g->width  = (int)((fmax_x - fmin_x) * g->inv_grid_len) + 1;
	g->height = (int)((fmax_y - fmin_y) * g->inv_grid_len) + 1;
	g->depth  = (int)((fmax_z - fmin_z) * g->inv_grid_len) + 1;

	g->N_cells = g->width*g->height*g->depth;

	if (g->N_cells > 50000)
	{
		printf("Abort: N_cells = %d \n", g->N_cells);
		exit(0);
	}

	if (g->cell_mem != NULL)
	{
		free(g->cell_mem);
		free(g->utilization);
	}

	g->cell_mem = (int*) malloc(g->N_cells*CELL_CAP*sizeof(int));

	g->utilization = (int*) calloc(g->N_cells, sizeof(int));

	for (int i=0; i < N_PARTICLES; i++)
	{
		int g_index = get_grid_index(g, pos[i]);
		int util = g->utilization[g_index];
		g->cell_mem[g_index * CELL_CAP + util] = i;
		g->utilization[g_index]++;
	}

}




void destroy_search_grid(grid_struc* g)
{
	free(g->cell_mem);
}




void create_nbr_list(neighbor_struc* nbr_list)
{
	nbr_list->n_matrix = (nbr_item*) malloc(N_PARTICLES*N_PARTICLES*sizeof(nbr_item));
	nbr_list->utilization = (int*) malloc(N_PARTICLES*sizeof(int));
}



/*
void destroy_nbr_list(neighbor_struc* nbr_list)
{
	for(int i=0; i < N_PARTICLES; i++)
	{
		free(nbr_list->n_matrix[i]);
	}
	free(nbr_list->n_matrix);
	free(nbr_list->utilization);
}
*/


void add_neighbor(neighbor_struc* nbr_list, int index_p, int index_n, float distsq)
{
	int util = nbr_list->utilization[index_p];
	//printf("\t index_p=%d, index_n=%d \t distsq=%f \n", index_p, index_n, distsq);
	//printf("\t\t Utilization=%d \n", util);
	nbr_list->n_matrix[N_PARTICLES*index_p + util].distsq = distsq;
	nbr_list->n_matrix[N_PARTICLES*index_p + util].index = index_n;
	nbr_list->utilization[index_p]++;
}


void set_neighbors(grid_struc* g, neighbor_struc* nbr_list, vec3* pos, int n_particles)
{
	int n_grid;
	int p_grid;
	float search_radius2;

	for (int i=0; i < N_PARTICLES; i++)
	{
		nbr_list->utilization[i] = 0;
	}

	search_radius2 = pow(g->grid_len,2);

	for (int p_index = 0; p_index < n_particles; p_index++)
	{
		p_grid = get_grid_index(g,pos[p_index]);

		for (int gz = -1; gz <= 1; gz++)
		{
			for (int gy = -1; gy <= 1; gy++)
			{
				for (int gx = -1; gx <= 1; gx++)
				{
					n_grid = p_grid + gx + g->width*gy + g->width*g->height*gz;

					if ((n_grid < 0)||(n_grid >= g->N_cells))
						continue;

					for (int j = 0; j < g->utilization[n_grid]; j++)
					{

						float distsq;
						int n_index = g->cell_mem[n_grid*CELL_CAP+j];
						/*
						if ((p_index==2) && (n_index==9))
						{
							vec3 tmp_vec = pos[p_index]-pos[n_index];
							distsq = dot(tmp_vec, tmp_vec);
							("p_index=%d, n_index=%d \t distsq=%f \n", p_index, n_index, distsq);
							add_neighbor(nbr_list, p_index, n_index, distsq);
							break;
						}
						*/
						vec3 tmp_vec = pos[p_index]-pos[n_index];
						distsq = dot(tmp_vec, tmp_vec);

						if (distsq<search_radius2)
						{
							add_neighbor(nbr_list, p_index, n_index, distsq);
						}
					}
				}
			}
		}
	}
}

