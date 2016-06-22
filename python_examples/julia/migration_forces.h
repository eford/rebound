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

extern double* tau_a; 	/**< Migration timescale in years for all particles */
extern double* tau_e; 	/**< Eccentricity damping timescale in years for all particles */

void migration_forces(struct reb_simulation* r);
void activate_migration_forces(struct reb_simulation* r);
void deactivate_migration_forces(struct reb_simulation* r);
void set_tau_a(int id, double val);
void set_tau_e(int id, double val);

