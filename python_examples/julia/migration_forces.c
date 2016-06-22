/**
 * Planetary migration in the GJ876 system
 *
 * This example applies dissipative forces to two
 * bodies orbiting a central object. The forces are specified
 * in terms of damping timescales for the semi-major axis and
 * eccentricity. This mimics planetary migration in a protostellar disc. 
 * The example reproduces the study of Lee & Peale (2002) on the 
 * formation of the planetary system GJ876. For a comparison, 
 * see figure 4 in their paper. The IAS15 or WHFAST integrators
 * can be used. Note that the forces are velocity dependent.
 * Special thanks goes to Willy Kley for helping me to implement
 * the damping terms as actual forces. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "../../src/rebound.h"

double* tau_a; 	/**< Migration timescale in years for all particles */
double* tau_e; 	/**< Eccentricity damping timescale in years for all particles */

void set_heartbeat(struct reb_simulation* r, void* func_ptr);
void migration_forces(struct reb_simulation* r);
void activate_migration_forces(struct reb_simulation* r);
void deactivate_migration_forces(struct reb_simulation* r);
void set_tau_a(int id, double val);
void set_tau_e(int id, double val);
double get_tau_a(int id);
double get_tau_e(int id);
void print_args(int id, double val);


void set_heartbeat(struct reb_simulation* r, void* func_ptr) {
	r->heartbeat = func_ptr;
}

void activate_migration_forces(struct reb_simulation* r) {
  printf("# calling activate_migration_forces... w/ N=%d N_active=%d force_is_velocity_dependent=%d t=%g\n",r->N,r->N_active,r->force_is_velocity_dependent,r->t);
  	r->additional_forces = migration_forces;
	r->force_is_velocity_dependent = 1;
	printf("# ...now N=%d force_is_velocity_dependent=%d\n",r->N,r->force_is_velocity_dependent);
  if(r->N>0) {
	tau_a = calloc(sizeof(double),r->N);
        tau_e = calloc(sizeof(double),r->N);
  }
}

void deactivate_migration_forces(struct reb_simulation* r) {
	r->additional_forces = NULL;
	r->force_is_velocity_dependent = 0;
	free(tau_a);
	free(tau_e);
}

void set_tau_a(int id, double val) {
	tau_a[id] = val;	
}
void set_tau_e(int id, double val) {
	tau_e[id] = val;	
}

void print_args(int id, double val) {
  printf("id=%d val=%g\n",id,val);	
}

double get_tau_a(int id) {
	return tau_a[id];
}
double get_tau_e(int id) {
	return tau_e[id];
}

void migration_forces(struct reb_simulation* r){
	const double G = r->G;
	const int N = r->N;
	struct reb_particle* const particles = r->particles;
	struct reb_particle com = particles[0]; // calculate migration forces with respect to center of mass;
	for(int i=1;i<N;i++){
		if (tau_e[i]!=0||tau_a[i]!=0){
			struct reb_particle* p = &(particles[i]);
			const double dvx = p->vx-com.vx;
			const double dvy = p->vy-com.vy;
			const double dvz = p->vz-com.vz;

			if (tau_a[i]!=0){ 	// Migration
				p->ax -=  dvx/(2.*tau_a[i]);
				p->ay -=  dvy/(2.*tau_a[i]);
				p->az -=  dvz/(2.*tau_a[i]);
			}
			if (tau_e[i]!=0){ 	// Eccentricity damping
				const double mu = G*(com.m + p->m);
				const double dx = p->x-com.x;
				const double dy = p->y-com.y;
				const double dz = p->z-com.z;

				const double hx = dy*dvz - dz*dvy; 
				const double hy = dz*dvx - dx*dvz;
				const double hz = dx*dvy - dy*dvx;
				const double h = sqrt ( hx*hx + hy*hy + hz*hz );
				const double v = sqrt ( dvx*dvx + dvy*dvy + dvz*dvz );
				const double r = sqrt ( dx*dx + dy*dy + dz*dz );
				const double vr = (dx*dvx + dy*dvy + dz*dvz)/r;
				const double ex = 1./mu*( (v*v-mu/r)*dx - r*vr*dvx );
				const double ey = 1./mu*( (v*v-mu/r)*dy - r*vr*dvy );
				const double ez = 1./mu*( (v*v-mu/r)*dz - r*vr*dvz );
				const double e = sqrt( ex*ex + ey*ey + ez*ez );		// eccentricity
				const double a = -mu/( v*v - 2.*mu/r );			// semi major axis
				const double prefac1 = 1./(1.-e*e) /tau_e[i]/1.5;
				const double prefac2 = 1./(r*h) * sqrt(mu/a/(1.-e*e))  /tau_e[i]/1.5;
				p->ax += -dvx*prefac1 + (hy*dz-hz*dy)*prefac2;
				p->ay += -dvy*prefac1 + (hz*dx-hx*dz)*prefac2;
				p->az += -dvz*prefac1 + (hx*dy-hy*dx)*prefac2;
			}
		}
		com = reb_get_com_of_pair(com,particles[i]);
	}
}

