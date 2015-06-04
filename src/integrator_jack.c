/**
 * @file 	integrator.c
 * @brief 	Mikkola integration scheme.
 * @author 	Hanno Rein <hanno@hanno-rein.de>
 * @detail	This file implements the WHFast integration scheme.  
 * 
 * @section 	LICENSE
 * Copyright (c) 2015 Hanno Rein, Daniel Tamayo
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
#include "particle.h"
#include "main.h"
#include "tools.h"
#include "gravity.h"
#include "boundaries.h"
#include "integrator.h"
#include "integrator_jack.h"


void integrator_jack_part1(void){
  double kc = G;
  
  double r0, r, v2, eta, beta, zeta;
  double b, c2, s2;
  double f, g, fdot, gdot;
  double X, G1, G2, G3;
  double a, n, x, esinE, ecosE;
  int flag;

  r0 = sqrt(particles[1].x*particles[1].x + particles[1].y*particles[1].y + particles[1].z*particles[1].z);
  v2 = particles[1].vx*particles[1].vx + particles[1].vy*particles[1].vy + particles[1].vz*particles[1].vz;
  eta = particles[1].x*particles[1].vx + particles[1].y*particles[1].vy + particles[1].z*particles[1].vz;
  beta = 2.0*kc/r0 - v2;
  zeta = kc - beta*r0;

  // THIS NEEDS TO BE COMPLETED....

  a = kc/beta;
  r = r0 + eta*G1 + zeta*G2;
  f = 1.0 - kc*G2/r0;
  g = eta*G2 + r0*G1;
  fdot = -(a/r)*(b/r0)*2.0*s2*c2;
  gdot = 1.0 - kc*G2/r;


  double xo = particles[1].x;
  double yo = particles[1].y;
  double zo = particles[1].z;
  particles[1].x = (f*particles[1].x + g*particles[1].vx);
  particles[1].y = (f*particles[1].y + g*particles[1].vy);
  particles[1].z = (f*particles[1].z + g*particles[1].vz);
  particles[1].vx = (fdot*xo + gdot*particles[1].vx);
  particles[1].vy = (fdot*yo + gdot*particles[1].vy);
  particles[1].vz = (fdot*zo + gdot*particles[1].vz);

}

void integrator_jack_synchronize(void){
}

void integrator_jack_part2(void){
	t+=dt;
}
	

void integrator_jack_reset(void){
}
