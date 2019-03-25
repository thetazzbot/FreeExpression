#ifndef _SCALE_H
#define _SCALE_H

#include "shvars.h"

/// Initialize translation and scale.
///
/// Translation is not implemented.
void translate_init(void);

/// Use IP and SC data to calculate the scale and translation.
///
/// Translation is not implemented. 
void translate_scale(void);

/// Transform user coordinates (fx,fy) into plotter coordinates (x,y) according to 
/// the transform defined by IP/SC. Then do reverse transform, round and assign
/// resulting values to (ox,oy).
/// @param	fx 	user coordinates x
/// @param	fy	user coordinates y
/// @param  *x 	(output) absolute stepper x
/// @param  *y 	(output) absolute stepper y
/// @param	*ox (output) corrected fx
/// @param	*oy (output) corrected fy
///
/// @see STEPSCALE_X
/// @see STEPSCALE_Y
void userscale(double fx, double fy, int16_t* x, int16_t* y, double* ox, double* oy);


/// Something that shouldn't be used 
void userprescale(double absx, double absy, double *ox, double *oy);

/// Something else that should not be used
USER_POINT scale_P1P2(void);

#endif