/*
 * stepper.h
 *
 * stepper X/Y/Z movement
 *
 		This source original developped by  https://github.com/Arlet/Freecut
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

#ifndef STEPPER_H
#define STEPPER_H

void stepper_init( void );
void stepper_tick( void );
void stepper_move( int x, int y );
void stepper_draw( int x, int y );
void stepper_speed( int delay );
void stepper_pressure( int pressure );
void stepper_home( void );
void stepper_set_origin00(void );
void stepper_unload_paper( void );
void stepper_load_paper( void );
void pen_up(void);
void pen_down(void);
void stepper_jog_manual(int direction, int dist);
void stepper_off( void );
enum state do_next_command( void );

// These values are opposite of their named meaning
// 1023 is "no pressure applied" and represents a very long
// delay on the PWM of the pen's MOSFET port
// 25 translates to "maximum voltage/current allowed" and
// pushes close to 12 volts through the MOSFET to the pen's solenoid
#define MAX_PEN_PWM 500			// MAX PWM for stepper solenoid i.e. nearly no pressure  ~ 10Gram
#define MIN_PEN_PWM 100			// MIN PWM for stepper solenoid i.e max pressure it can do ~ 300gram when cold, 275 gram when hot.
#define MAX_CUTTER_P_RANGES 10	// The number of pressure choices  
#define MAX_STEPPER_SPEED_RANGES 9	// the number of speed choices
#endif
