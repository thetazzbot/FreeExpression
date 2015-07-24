#ifndef _GCODE_H
#define _GCODE_H
#include "shvars.h"

/// Initialize the scanner.
void gcode_init(void);

/// Handle next character from the input. When action is determined, return one of the _hpgl_command values.
/// Pass target coordinates in x and y.
/// @param c	input char
/// @param x	output: destination x (returns -1 if no data)
/// @param y	output: destination y (returns -1 if no data)
/// @param lb	output: next label character (see CMD_LB)
/// @returns	_hpgl_command
int8_t gcode_char(char c, STEPPER_COORD* x, STEPPER_COORD* y);

#endif
