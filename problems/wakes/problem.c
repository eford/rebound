/**
 * @file 	problem.c
 * @brief 	Example problem: shearing sheet.
 * @author 	Hanno Rein <hanno@hanno-rein.de>
 * @detail 	This problem uses shearing sheet boundary
 * conditions. Particle properties resemble those found in 
 * Saturn's rings. 
 * 
 * @section 	LICENSE
 * Copyright (c) 2011 Hanno Rein, Shangfei Liu
 *
 * This file is part of rebound.
 *
 * rebound is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rebound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rebound.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#ifdef OPENMP
#include <omp.h>
#endif
#include "main.h"
#include "particle.h"
#include "boundaries.h"
#include "output.h"
#include "communication_mpi.h"
#include "tree.h"
#include "collisions.h"
#include "tools.h"
#include "input.h"

extern double OMEGA;
extern double coefficient_of_restitution;
extern double minimum_collision_velocity;

extern double (*coefficient_of_restitution_for_velocity)(double); 
double coefficient_of_restitution_bridges(double v); 
void input_append_input_arguments_with_int(const char* argument, int value);
void problem_find_restartfile();
int position_id=0;	// output number
int check_transparency;


extern double opening_angle2;
extern int output_logfile_first;

double sun_B;
double sun_phi;

void problem_init(int argc, char* argv[]){
	// Setup constants
#ifdef GRAVITY_TREE
	opening_angle2	= .5;
#endif
	G 				= 6.67428e-11;		// N m^2 / kg^2 
	double a			= 391e3;		// m 
	double chariklo_m		= 1e19; 		// kg
	OMEGA 				= sqrt(G*chariklo_m/(a*a*a));	// 1/s
	softening 			= 0.1;			// m
	dt 				= 1e-3*2.*M_PI/OMEGA;	// s
	tmax				= 10.*2.*M_PI/OMEGA+dt;
	check_transparency		= input_get_int(argc,argv,"check_transparency",1);

	// Setup domain dimensions
	int _root_n = 1;
#ifdef MPI
	do{
		_root_n++;
	}while(_root_n*_root_n <= mpi_num);
	_root_n--;
	if (_root_n*_root_n!=mpi_num){
		printf("\n\nError. mpi_num must be square of integer.\n");
		exit(-1);
	}
#endif // MPI

	root_nx = _root_n;
	root_ny = _root_n;
	root_nz = 1;
	nghostx = 2;
	nghosty = 2;
	nghostz = 0;
	boxsize = input_get_double(argc,argv,"boxsize",25)/(double)root_nx;
	init_box();
	//
	sun_B 			= input_get_double(argc,argv,"sun_B",M_PI/2.); 			// Sun elevation angle, default: normal to ring plane
	sun_phi 			= input_get_double(argc,argv,"sun_phi",0.); 			// Sun azimuthal angle

	
	// Setup particle and disk properties
	double sigma 			= input_get_double(argc,argv,"sigma",400); 			// kg/m^2
	double rho			= input_get_double(argc,argv,"rho",250);			// kg/m^3
	double particle_radius_min 	= input_get_double(argc,argv,"rmin",.3);				// m
	double particle_radius_max 	= input_get_double(argc,argv,"rmax",3);				// m
	double particle_radius_slope 	= -3;	
	minimum_collision_velocity 	= 0.05*particle_radius_min*OMEGA;
	coefficient_of_restitution_for_velocity	= coefficient_of_restitution_bridges;

	
	struct 	aabb bb	= {	.xmin = -boxsize_x/2., 
				.xmax =  boxsize_x/2., 
				.ymin = -boxsize_y/2., 
				.ymax =  boxsize_y/2., 
				.zmin = -boxsize_z/2., 
				.zmax =  boxsize_z/2.	};
	long	_N;
	if (particle_radius_min==particle_radius_max){
		_N = round(sigma*boxsize_x*boxsize_y/(4./3.*M_PI*rho*  pow(particle_radius_max,3.)));
	}else{
		_N = round(sigma*boxsize_x*boxsize_y/(4./3.*M_PI*rho* (pow(particle_radius_max,4.+particle_radius_slope) - pow(particle_radius_min,4.+particle_radius_slope)) / (pow(particle_radius_max,1.+particle_radius_slope) - pow(particle_radius_min,1.+particle_radius_slope)) * (1.+particle_radius_slope)/(4.+particle_radius_slope)));
	}

#ifdef OPENMP
	input_append_input_arguments_with_int("openmp",omp_get_max_threads());
#endif // OPENMP

#ifdef MPI
	input_append_input_arguments_with_int("mpinum",mpi_num);
#endif // MPI
	if (output_check_directory()==1){
		// Restart simulation
		problem_find_restartfile();
		output_logfile_first = 0;
#ifdef MPI
		if (mpi_id==0){
#endif // MPI
		output_double("Restarted at time",t);
#ifdef MPI
		}
#endif // MPI
	}else{
		// Start from scratch and prepare directory
		output_prepare_directory();
#ifdef MPI
		bb = communication_boundingbox_for_proc(mpi_id);
		_N   /= mpi_num;
#endif // MPI
		double mass = 0;
		double area = 0;
		for(int i=0;i<_N;i++){
			struct particle pt;
			do{
				pt.z 		= particle_radius_max*tools_normal(2.);	
			}while(fabs(pt.z)>=boxsize_z/2.);
#ifdef MPI
			int proc_id;
			do{
#endif // MPI
				pt.x 		= tools_uniform(bb.xmin,bb.xmax);
				pt.y 		= tools_uniform(bb.ymin,bb.ymax);
#ifdef MPI
				int rootbox = particles_get_rootbox_for_particle(pt);
				int root_n_per_node = root_n/mpi_num;
				proc_id = rootbox/root_n_per_node;
			}while(proc_id != mpi_id );
#endif // MPI
			
			pt.vx 		= 0;
			pt.vy 		= -1.5*pt.x*OMEGA;
			pt.vz 		= 0;
			pt.ax 		= 0;
			pt.ay 		= 0;
			pt.az 		= 0;
			pt.r 		= tools_powerlaw(particle_radius_min,particle_radius_max,particle_radius_slope);
			pt.m 		= rho*4./3.*M_PI*pt.r*pt.r*pt.r;
			mass		+= pt.m;
			area		+= M_PI*pt.r*pt.r;

			
			particles_add(pt);
		}
#ifdef MPI
		if (mpi_id==0){
#endif // MPI
			output_double("boxsize [m]",boxsize);
			output_int("root_nx",root_nx);
			output_int("root_ny",root_ny);
			output_int("root_nz",root_nz);
			output_double("tau_geom",area/(boxsize*boxsize));
			output_double("sigma [kg/m^2]",sigma);
			output_double("rho [kg/m^3]",rho);
			output_double("particle_radius_min [m]",particle_radius_min);
			output_double("particle_radius_max [m]",particle_radius_max);
			output_double("particle_radius_slope",particle_radius_slope);
			output_double("OMEGA [1/s]",OMEGA);
			output_double("lambda_crit [m]",4.*M_PI*M_PI*G*sigma/OMEGA/OMEGA);
			output_double("length [m]",boxsize*(double)root_nx);
			output_double("length/lambda_crit",boxsize*(double)root_nx/(4.*M_PI*M_PI*G*sigma/OMEGA/OMEGA));
			output_int("N",_N);
#ifdef MPI
			output_int("N_total",_N*mpi_num);
			output_int("mpi_num",mpi_num);
#endif // MPI
			output_double("tmax [orbits]",tmax/(2.*M_PI/OMEGA));
			output_int("number of timesteps",ceil(tmax/dt));
			system("cat config.log");
#ifdef MPI
		}
#endif // MPI
	}
#ifdef MPI
	MPI_Barrier(MPI_COMM_WORLD);
#endif // MPI
}

void problem_find_restartfile(){
#ifdef MPI
	if( access( "binary_0.txt", F_OK ) == -1 ) {
#else // MPI
	if( access( "binary.txt", F_OK ) == -1 ) {
#endif // MPI
		printf("Cannot find a restart file in directory.\n");
		exit(-1);
	}
	input_binary("binary.txt");	
}

double coefficient_of_restitution_bridges(double v){
	// assumes v in units of [m/s]
	double eps = 0.32*pow(fabs(v)*100.,-0.234);
	if (eps>1) eps=1;
	if (eps<0) eps=0;
	return eps;
}

void problem_inloop(){
}

void output_ascii_mod(char* filename){
#ifdef MPI
	char filename_mpi[1024];
	sprintf(filename_mpi,"%s_%d",filename,mpi_id);
	FILE* of = fopen(filename_mpi,"w"); 
#else // MPI
	FILE* of = fopen(filename,"w"); 
#endif // MPI
	if (of==NULL){
		printf("\n\nError while opening file '%s'.\n",filename);
		return;
	}
	for (int i=0;i<N;i++){
		struct particle p = particles[i];
		fprintf(of,"%e\t%e\t%e\t%e\n",p.x,p.y,p.z,p.r);
	}
	fclose(of);
}
	
double sun_b[3];

double  tree_does_ray_hit_particle_above(struct cell* c, double a[3], double b[3],int pr){
	double t1 = b[1]*(a[2]-c->z)-b[2]*(a[1]-c->y);
	double t2 = b[2]*(a[0]-c->x)-b[0]*(a[2]-c->z);
	double t3 = b[0]*(a[1]-c->y)-b[1]*(a[0]-c->x);
	double bot2 = b[0]*b[0]+b[1]*b[1]+b[2]*b[2];
	double distance2 = (t1*t1+t2*t2+t3*t3)/bot2;
	double width=c->w*0.86602540378+collisions_max_r;
	if (distance2<width*width){
		if (c->pt<0){
			for(int i=0;i<8;i++){
				struct cell* o = c->oct[i];
				if(o){
					double F = tree_does_ray_hit_particle_above(o,a,b,pr);
					if (F>0) return F;
				}
			}
		}else{
			struct particle p = particles[c->pt];
			double u  = b[0]*(a[0]-p.x) + b[1]*(a[1]-p.y) + b[2]*(a[2]-p.z);
			double v  = (p.x-a[0])*(p.x-a[0]) + (p.y-a[1])*(p.y-a[1]) + (p.z-a[2])*(p.z-a[2]);
			double s  = u*u - v + p.r*p.r;
			if (s>0){ // line intersects sphere
				double d  = -u + sqrt(s); // other intersection at -u-sqrt(s). Choosing the one with larger z value.
				//double cx = a[0] + d*b[0];	// intersection point 
				//double cy = a[1] + d*b[1];
				double cz = a[2] + d*b[2];
				if (cz>a[2] && c->pt != pr){
					return 1; 	// ray hits other sphere above scattering sphere
				}
			}
		}
	}
	return 0;
}

void  tree_does_ray_hit_particle(struct cell* c, double a[3], double b[3], double* flux, double* height){
	double t1 = b[1]*(a[2]-c->z)-b[2]*(a[1]-c->y);
	double t2 = b[2]*(a[0]-c->x)-b[0]*(a[2]-c->z);
	double t3 = b[0]*(a[1]-c->y)-b[1]*(a[0]-c->x);
	double bot2 = b[0]*b[0]+b[1]*b[1]+b[2]*b[2];
	double distance2 = (t1*t1+t2*t2+t3*t3)/bot2;
	double width=c->w*0.86602540378+collisions_max_r;
	if (distance2<width*width){
		if (c->pt<0){
			for(int i=0;i<8;i++){
				struct cell* o = c->oct[i];
				if(o){
					tree_does_ray_hit_particle(o,a,b, flux, height);
				}
			}
		}else{
			struct particle p = particles[c->pt];
			double u  = b[0]*(a[0]-p.x) + b[1]*(a[1]-p.y) + b[2]*(a[2]-p.z);
			double v  = (p.x-a[0])*(p.x-a[0]) + (p.y-a[1])*(p.y-a[1]) + (p.z-a[2])*(p.z-a[2]);
			double s  = u*u - v + p.r*p.r;
			if (s>0){ // line intersects sphere
				// TODO: Make a list of all intersections, select the closet one to the pbserver (z sorting)
				double d  = -u + sqrt(s); // other intersection at -u-sqrt(s). Choosing the one with larger z value.
				double cx = a[0] + d*b[0];	// intersection point 
				double cy = a[1] + d*b[1];
				double cz = a[2] + d*b[2];

				if (cz>(*height)){			// intersection happens before previous intersection
					double nx = cx - p.x; 		// normal to sphere
					double ny = cy - p.y;
					double nz = cz - p.z;
					double n  = sqrt(nx*nx + ny*ny + nz*nz);
					nx /= n;
					ny /= n;
					nz /= n;
				
					double cosNB = nx*sun_b[0] + ny*sun_b[1] + nz*sun_b[2];
					if (cosNB>0){ // dayside of particle
						// Check for shadow.
						int nghostray = 1; // check only one ghost box for now. TODO
						int isinshadow = 0;
						for (int gbx=-nghostray; gbx<=nghostray; gbx++){
							for (int gby=-nghostray; gby<=nghostray; gby++){
								struct ghostbox gb = boundaries_get_ghostbox(gbx,gby,0);
								double inters[3]; // ray position in xy plane
								inters[0] = cx+gb.shiftx;
								inters[1] = cy+gb.shifty;
								inters[2] = cz+; 
								for (int i=0;i<root_n;i++){
									struct cell* ce = tree_root[i];
									if (tree_does_ray_hit_particle_above(ce,inters,sun_b,c->pt)){
										isinshadow = 1;
									}
								}
							}
						}
						if (isinshadow){
							(*flux) 	= 0;
						}else{
							(*flux) 	= cosNB;
						}
						(*height)	= cz;
					}else{
						(*flux) 	= 0;
						(*height)	= cz;
					}
				}
			}
		}
	}
	return 0.;
}

void tree_get_transparency(double B, double phi){
	double b[3]; // incoming ray vector
	b[0] 		= cos(phi)*cos(B);
	b[1] 		= sin(phi)*cos(B);
	b[2] 		= sin(B);
	// light ray vector
	sun_b[0] 	= cos(sun_phi)*cos(sun_B);
	sun_b[1] 	= sin(sun_phi)*cos(sun_B);
	sun_b[2] 	= sin(sun_B);
	const double taninv = 1./tan(B);
	double raylength = taninv * boxsize_x/2.+2.*collisions_max_r; 
	int nghostray=0;
	double boxlengthray = 0;
	while(boxlengthray<raylength){
		if (nghostray==0){
			boxlengthray += boxsize_x/2.;
		}else{
			boxlengthray += boxsize_x;
		}
		nghostray++;
	}

	const int N_rays 		= 1000;

	double*   _F = calloc(N_rays,sizeof(double));


	int N_ray_hit_particle 	= 0;	// Number of rays that hit a particle
	double F_reflected 	= 0.;	// Integrated flux. 
#pragma omp parallel for 
	for(int j=0;j<N_rays;j++){
		double a[3]; // ray position in xy plane
		a[0] = tools_uniform(-boxsize_x/2.,boxsize_x/2.);
		a[1] = tools_uniform(-boxsize_y/2.,boxsize_y/2.); // Only really works in square boxes.
		a[2] = 0;
		int transparency = 0;
		double flux = -1;	// -1 means ray passes through ring, 0 means it hits a dark particle, 1 means it hits sees the most iluminated part of a a particle
		double height = -boxsize_z;
		for (int gbx=-nghostray; gbx<=nghostray; gbx++){
			for (int gby=-nghostray; gby<=nghostray; gby++){
				struct ghostbox gb = boundaries_get_ghostbox(gbx,gby,0);
				double as[3]; // ishifted ray position in xy plane
				as[0] = a[0]+gb.shiftx;
				as[1] = a[1]+gb.shifty;
				as[2] = a[2];
				for (int i=0;i<root_n;i++){
					struct cell* c = tree_root[i];
					tree_does_ray_hit_particle(c,as,b, &flux, &height);	
				}
			}
		}
		if (flux>=0.){
			N_ray_hit_particle 	+= 1;
			F_reflected 		+= flux;
		}
	}


	FILE* of = fopen("transparency.txt","a+"); 
	if (of==NULL){
		printf("\n\nError while opening file.\n");
		return;
	}
	fprintf(of,"%e\t%e\t%e\t%0.8f\n",t,B,phi,(double)N_ray_hit_particle/(double)N_rays,(double)F_reflected/(double)N_rays);
	fclose(of);
}

void output_transparency(){
	tree_get_transparency(M_PI/2.,0); //Normal to ring plane
//	for (double phi = 0; phi<M_PI; phi+=M_PI/10.){
//		double lowB = M_PI/8.;
//		double numB = 20.;
//		for (int i=0;i<numB;i++){
//			double B = lowB+(M_PI/2.-lowB)/numB*(double)i;
//			tree_get_transparency(B,phi);
//		}
//
//	}
}


double timing_laststep = 0;
void problem_output(){
	if (output_check(10.*dt)&&check_transparency){
		output_transparency();
	}
	if (output_check(10.*dt)){
		output_timing();
	}
	if (output_check(2.*M_PI/OMEGA)){
		char filename[256];
		sprintf(filename,"position_%08d.txt",position_id);
		output_ascii_mod(filename);
		position_id++;
	}
	if (output_check(.2*M_PI/OMEGA)){
		output_binary("binary.txt");
	}
#ifdef MPI
	if (mpi_id==0){
#endif // MPI
		if (timing_laststep==0){
			timing_laststep = timing_initial;
		}
		struct timeval tim;
		gettimeofday(&tim, NULL);
		double timing_final = tim.tv_sec+(tim.tv_usec/1000000.0);
		FILE* of = fopen("timing.txt","a+"); 
		fprintf(of,"%e\t%.4f\n",t,timing_final-timing_laststep);
		fclose(of);
		timing_laststep = timing_final;
#ifdef MPI
	}
#endif // MPI
}

void problem_finish(){
#ifdef MPI
	if (mpi_id==0){
#endif // MPI
		struct timeval tim;
		gettimeofday(&tim, NULL);
		double timing_final = tim.tv_sec+(tim.tv_usec/1000000.0);
		output_double("runtime [s]",timing_final-timing_initial);
		system("cat config.log");
#ifdef MPI
	}
#endif // MPI
}