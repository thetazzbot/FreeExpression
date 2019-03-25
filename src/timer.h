/*
 * timer.h
 *
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
#ifndef TIMER_H
#define TIMER_H

void timer_init( void );
void usleep( int usecs );
void msleep( unsigned msecs );
void beeper_on( int Hz );
void beeper_off( void );
void timer_set_stepper_speed( int delay );
void timer_set_pen_pressure( int pressure );
int timer_get_pen_pressure(void);
int timer_get_stepper_speed(void);
void beep(void);
extern volatile uint8_t flag_Hz;
extern volatile uint8_t flag_25Hz;

#endif
