#include <memory.h>
#include <math.h>
#include <assert.h>
#include <iostream>

#include <stdio.h>  // For debugging only

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
using namespace glm;
using namespace std;

#include "memory.h"
#include "common.h"
#include "physics.h"

#define RADIUS 0.03f

#define R_LEN 4
#define R_HEIGHT 13


// Definition of constants
#define SMOOTHING_LENGTH 0.05 // Ein wenig länger als der initiale Teilchenabstand.
#define VISCOSITY 0.1
#define MASS 0.1
#define STIFF 1.0
#define SEARCH_RADIUS (SMOOTHING_LENGTH)
#define TIME_STEP 0.0001  // TIME_STEP has influence on the stability of the program.


#define SCAL_LEN 0.1
#define N_PARTICLES (CUBE_LEN_X*CUBE_LEN_Y*CUBE_LEN_Z)
#define EPSILON 0.05





const float PI = 3.1415926535f;





void get_pos(vec3* pos, sph_struc* sph)
{
	memcpy(pos, sph->pos, sph->n_particles*sizeof(vec3));
}

void create_sph_instance(sph_struc* sph, int size, vec3* pos, vec3* vel)
{
	sph->viscosity = VISCOSITY;
	sph->n_particles = size;
	sph->smoothlen = SMOOTHING_LENGTH;
	sph->stiff = STIFF;

	sph->timestep = TIME_STEP;

	sph->r_search = SEARCH_RADIUS;

	// Allocate memory for attributes of particles
	sph->mass = MASS;
	sph->density = new float[size];
	sph->pos = new vec3[size];
	sph->vel = new vec3[size];
	sph->force = new vec3[size];
	sph->normal = new vec3[size];
	sph->curvature = new float[size];

	memcpy(sph->pos, pos, sph->n_particles*sizeof(vec3));
	memcpy(sph->vel, vel, sph->n_particles*sizeof(vec3));

	// Just to make sure that everything is neat
	memset(sph->density, 0, sph->n_particles*sizeof(float));
	memset(sph->force, 0, sph->n_particles*sizeof(vec3));
	memset(sph->normal, 0, sph->n_particles*sizeof(vec3));
	memset(sph->curvature, 0, sph->n_particles*sizeof(float));
}



float W_poly6(float lengthsq, float h)
{
	float h2_r2 = h*h - lengthsq;

	if (h2_r2 > 0.0f)
		return 315.0f/(64.0f*PI*pow(h, 9)) * pow(h2_r2,3);
	return 0.0f;
}


vec3 gradient_W_poly6(vec3 vec_r, float h)
{
	float r = length(vec_r);



	if (h-r > 0.0f)
		return mat3(-945.0f/(32*PI*pow(h,9))*pow(h*h-r*r,2))*vec_r;
	return vec3(0.0f);
}

float laplacian_W_poly_6(vec3 vec_r, float h)
{
	float r = length(vec_r);

	if (h-r > 0.0f)
		return 945.0f/(8.0f*PI*pow(h,9))*(h*h-r*r)*(r*r-3.0f/4.0f*(h*h-r*r));
	return 0.0f;
}



vec3 gradient_W_spiky(vec3 vec_r, float h)
{
	float r = length(vec_r);
	float diff_h_r = (h-r)*(h-r);

	if (h-r > 0.0f)
		return mat3(-45.0f/(PI*pow(h,6)) * diff_h_r) * vec_r;
	return vec3(0.0f);
}


float laplacian_W_viscosity(vec3 vec_r, float h)
{
	float quotient;

	quotient = 1.0f - length(vec_r)/h;

	if (h-length(vec_r) > 0.0f)
		return 45.0f/(PI*pow(h,5)) * quotient;
	return 0.0f;
}




void compute_density(sph_struc* sph, neighbor_struc* nbr_list)
{
	int i,j;

	memset(sph->density, 0, sph->n_particles*sizeof(float));

	for (i = 0; i < sph->n_particles; i++)
	{
		for (j = 0; j < nbr_list->utilization[i]; j++)
		{
			float distsq = nbr_list->n_matrix[sph->n_particles*i+j].distsq;

			sph->density[i] += sph->mass * W_poly6(distsq, sph->r_search);
		}
	}
}


void compute_force(sph_struc* sph, neighbor_struc* nbr_list)
{
	int i, j;
	float h = sph->smoothlen;

	for (i = 0; i < sph->n_particles; i++)
	{
		vec3 pressure_force(0.0), viscosity_force(0.0), surface_tension(0.0), color_field_gradient(0.0);
		float color_field_laplacian;

		for (j = 0; j < nbr_list->utilization[i]; j++)
		{
			int nbr_index;
			float p_pressure, n_pressure;
			vec3 vec_r;

			nbr_index = nbr_list->n_matrix[sph->n_particles*i+j].index;
			vec_r = sph->pos[i] - sph->pos[nbr_index];

			//cout << "p: " << i << "\t n: " << j << endl;

			p_pressure = sph->stiff*(sph->density[i] - 0.0f);
			n_pressure = sph->stiff*(sph->density[nbr_index] - 0.0f);

			// Pressure-force of particle
			if (sph->density[nbr_index] != 0.0f)
				pressure_force = sph->mass * (p_pressure + n_pressure)/(2.0f*sph->density[nbr_index]) * gradient_W_spiky(vec_r,h);

			// Viscosity-force of particle
			viscosity_force = - sph->viscosity*sph->mass / sph->density[nbr_index]*(sph->vel[i] - sph->vel[nbr_index]) * laplacian_W_viscosity(vec_r,h);

			// Color-field-gradient of particle
			color_field_gradient = mat3(sph->mass/sph->density[nbr_index]) * gradient_W_poly6(vec_r,h);

			// Color-field-laplacian of particle
			color_field_laplacian = sph->mass / sph->density[nbr_index] * laplacian_W_poly_6(vec_r,h);

			// Surface tension ('sigma' fehlt)
			surface_tension = mat3(-color_field_laplacian/length(color_field_gradient)) * color_field_gradient;
			
			sph->force[i] -= mat3(-0.5e4)*vec_r; // experiment with the plus or minus-sign

		}

		//sph->force[i] = mat3(1.0) * (pressure_force + mat3(1.0) * viscosity_force + surface_tension);
	}
}







vec3 compute_collision(vec3 normal)
{
	return mat3(1.0f)*normal;
}



// Collision for a glass cube
void process_collision(sph_struc* sph)
{
	float delta_x; // Distance between object and fluid particle
	vec3 normal; // the inward normal
	
	for (int i = 0; i < sph->n_particles; i++)
	{
		float edge_length = 1.0; 	// half-length of box: 0.5
		mat3 wall_parameter = mat3(1.0e3);

		vec3 pos_pred = sph->pos[i] + sph->timestep * sph->vel[i];

		// Bottom plate
		normal = vec3( 0.0f, 1.0f, 0.0f);
		delta_x = dot(pos_pred, normal);

		if (delta_x < -edge_length)
		{
			sph->force[i] += wall_parameter*compute_collision(normal);
		}

		// Top plate
		normal = vec3( 0.0f, -1.0f, 0.0f);
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->force[i] += wall_parameter*compute_collision(normal);
		}

		normal = vec3( -1.0f, 0.0f, 0.0f);
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->force[i] += wall_parameter*compute_collision(normal);
		}

		normal = vec3(1.0f, 0.0f, 0.0f);
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->force[i] += wall_parameter*compute_collision(normal);
		}

		normal = vec3(0.0f, 0.0f, -1.0f );
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->force[i] += wall_parameter*compute_collision(normal);
		}

		normal = vec3( 0.0f, 0.0f, 1.0f );
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->force[i] += wall_parameter*compute_collision(normal);
		}
	}
	return;
}

// Collision for a glass cube
void process_reflection(sph_struc* sph)
{
	float delta_x; // Distance between object and fluid particle
	vec3 normal; // the inward normal
	
	for (int i = 0; i < sph->n_particles; i++)
	{
		float edge_length = 1.0; 	// half-length of box: 0.5

		vec3 pos_pred = sph->pos[i] + sph->timestep * sph->vel[i];

		// Bottom plate
		normal = vec3( 0.0f, 1.0f, 0.0f);
		delta_x = dot(pos_pred, normal);

		if (delta_x < -edge_length)
		{
			sph->vel[i] -= normal*dot(normal,sph->vel[i]);
		}

		// Top plate
		normal = vec3( 0.0f, -1.0f, 0.0f);
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->vel[i] -= normal*dot(normal,sph->vel[i]);
		}

		normal = vec3( -1.0f, 0.0f, 0.0f);
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->vel[i] -= normal*dot(normal,sph->vel[i]);
		}

		normal = vec3(1.0f, 0.0f, 0.0f);
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->vel[i] -= normal*dot(normal,sph->vel[i]);
		}

		normal = vec3(0.0f, 0.0f, -1.0f );
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->vel[i] -= normal*dot(normal,sph->vel[i]);
		}

		normal = vec3( 0.0f, 0.0f, 1.0f );
		delta_x = dot(pos_pred, normal);
		if (delta_x < -edge_length)
		{
			sph->vel[i] -= normal*dot(normal,sph->vel[i]);
		}
	}
	return;
}

void elapse_water(sph_struc* sph, grid_struc* g, neighbor_struc* nbr_list)
{
	alloc_search_grid(g, sph->pos, sph->n_particles, sph->r_search);
	set_neighbors(g, nbr_list, sph->pos, sph->n_particles);

	memset(sph->force, 0, sph->n_particles * sizeof(vec3));
	//compute_density(sph, nbr_list);
	compute_force(sph, nbr_list);
	//process_collision(sph);
	process_reflection(sph);
	
	//float scale = 1.0e-12; // for debugging purposes only
	vec3 gravity = vec3(0.0, -1e3, 0.0);
	//vec3 gravity = vec3(0.0);


	// Time integration (Euler method)
	for (int i = 0; i < sph->n_particles; i++)
	{
		sph->force[i] += sph->mass * gravity;
		/*cout << "Force[i].x: " << sph->force[i].x << endl;
		cout << "Force[i].y: " << sph->force[i].y << endl;
		cout << "Force[i].z: " << sph->force[i].z << endl;*/
		sph->vel[i] += mat3(sph->timestep / sph->mass) * sph->force[i];
		sph->pos[i] += mat3(sph->timestep)*sph->vel[i];// + scale * mat3(0.5f*pow(sph->timestep, 2)/sph->mass) * sph->force[i];
		
	}
}


