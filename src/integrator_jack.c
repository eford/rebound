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

void  solve_universal_kepler(double r0, double beta, double eta, double zeta, double h, double *G1, double *G2)
{
 double b;
  double x, g1, g2, g3;
  double a, s2, c2, g, xnew, x1;
  double err;

  b = sqrt(beta);
  xnew = (h/r0)*(1.0 - 0.5*eta*h/(r0*r0));
  err = 1.e-9*xnew;
  int NN = 1000;
  do {
    x = xnew;
    a = b*x/2.0;
    s2 = sin(a);
    c2 = cos(a);
    g1 = 2.0*s2*c2/b;
    g2 = 2.0*s2*s2/beta;
    g3 = (x - g1)/beta;
    g = eta*g1 + zeta*g2;
    xnew = (x*g - eta*g2 - zeta*g3 + h)/(r0 + g);
    NN--;
  } while(fabs(x - xnew) > err && NN>0);
  if (NN<=0){
    *G1 = 2.;
    *G2 = 2.;
    return;
  }
  
  x = xnew;
  a = b*x/2.0;
  s2 = sin(a);
  c2 = cos(a);
  g1 = 2.0*s2*c2/b;
  g2 = 2.0*s2*s2/beta;

  *G1 = g1;
  *G2 = g2;

}

void integrator_jack_part1(void){
  double kc = G;
  double r0, r, v2, eta, beta, zeta;
  double f, g, fdot, gdot;
  double X, G1, G2, G3;

  r0 = sqrt(particles[1].x*particles[1].x + particles[1].y*particles[1].y + particles[1].z*particles[1].z);
  v2 = particles[1].vx*particles[1].vx + particles[1].vy*particles[1].vy + particles[1].vz*particles[1].vz;
  eta = particles[1].x*particles[1].vx + particles[1].y*particles[1].vy + particles[1].z*particles[1].vz;
  beta = 2.0*kc/r0 - v2;
  if(beta<0.0) {
	  usleep(5);
    return;
  }
  zeta = kc - beta*r0;

  solve_universal_kepler(r0, beta, eta, zeta, dt, &G1, &G2);

  r = r0 + eta*G1 + zeta*G2;
  f = 1.0 - kc*G2/r0;
  g = eta*G2 + r0*G1;
  fdot = -kc*G1/(r0*r);
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
