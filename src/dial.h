/*
 * dial.h
 *
 * Interface for front panel dials.
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

#ifndef DIALS_H
#define DIALS_H

enum
{ 
    DIAL_SIZE,
    DIAL_SPEED,
    DIAL_PRESSURE,
    MAX_DIALS
};

int dial_get_speed( void );
int dial_get_pressure( void );
int dial_get_size( void );
void dial_poll( void );
void dial_init( void );

#endif
