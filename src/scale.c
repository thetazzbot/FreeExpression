/* 
 * Scaling functions used by the HPGL state machine
 * 
 * This file is part of FreeExpression.
 *
 * https://github.com/thetazzbot/FreeExpression
 *
 * FreeExpression is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2.
 *
 * FreeExpression is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeExpression. If not, see http://www.gnu.org/licenses/.
 *
 */
#include <inttypes.h>
#include <math.h>

#include <avr/pgmspace.h>
#include <stdio.h>

#include "configs.h"
#include "shvars.h"
#include "scale.h"


static double  user_xscale, user_yscale, user_translate_x, user_translate_y;

void translate_init() {
	user_xscale = user_yscale = 1;
	user_translate_x = user_translate_y = 0;
	ip_pad[0] = 0;
	ip_pad[1] = 0;
	ip_pad[2] = 9500;
	ip_pad[3] = 7000;
	sc_pad[0] = 0;
	sc_pad[1] = 9500;
	sc_pad[2] = 0;
	sc_pad[3] = 7000;
}

// use IP and SC data to calculate the scale
void translate_scale() {
	int32_t ipxrange = ip_pad[2] - ip_pad[0];
	int32_t ipyrange = ip_pad[3] - ip_pad[1];
	int32_t scxrange = sc_pad[1] - sc_pad[0];	// xmax - xmin
	int32_t scyrange = sc_pad[3] - sc_pad[2];	// ymax - ymin
	
	user_xscale = ((double)ipxrange)/((double)scxrange);
	user_yscale = ((double)ipyrange)/((double)scyrange);
	//user_xscale = ((double)scxrange)/((double)ipxrange);
	//user_yscale = ((double)scyrange)/((double)ipyrange);
	user_translate_x = -sc_pad[0]*user_xscale;
	user_translate_y = -sc_pad[2]*user_yscale;
	
	printf_P(PSTR("Scale set to: (%f,%f) translate (%f,%f)"), user_xscale, user_yscale, user_translate_x, user_translate_y);
}

void userprescale(double absx, double absy, double *ox, double *oy) {
	*ox = absx/user_xscale;
	*oy = absy/user_yscale;
}

void userscale(double fx, double fy, int16_t* x, int16_t* y, double* ox, double* oy) {
	*x = (int)round(fx*STEPSCALE_X*user_xscale);
	*y = (int)round(fy*STEPSCALE_Y*user_yscale);
	
	*ox = (*x)/(user_xscale*STEPSCALE_X);
	*oy = (*y)/(user_yscale*STEPSCALE_Y);
}

USER_POINT scale_P1P2() {
	USER_POINT p;
	p.x = ip_pad[2] - ip_pad[0];
	p.y = ip_pad[3] - ip_pad[1];
	
	return p;
}

