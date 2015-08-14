#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "shvars.h"
#include "hpgl.h"		// the enum in this file should really be in cli.h 
#include "avrlibtypes.h"

#define DPI 400
// TODO: DPI should be in stepper.h 

// Internal scanner state. 
/// @see hpgl_char()
typedef enum _en_gcode_state 
{
	WAIT_FOR_CMD =1,
	WAIT_FOR_G,
	WAIT_FOR_M,
	WAIT_FOR_X,
	WAIT_FOR_Y,
	WAIT_FOR_Z,
	SKIP_COMMENT,
	PROCESS_LINE
	
} en_gcode_state;


STEPPER_COORD  dx = 0;
STEPPER_COORD  dy = 0;

static  BOOL
is_numeric ( char c)
{
	if ( c >= '0' && c <= '9' )
		return  TRUE;
	
	if ( c == '-' || c == '+' || c =='.')
		return TRUE;
	
	return FALSE;

}



// TODO: could use a init function and a struct to keep all the state machine state info together instead of static inside function
void gcode_init() {

}

int8_t gcode_char(char c, STEPPER_COORD* x, STEPPER_COORD* y) 
{
	static unsigned char tmp_ndx;
	static en_gcode_state State = WAIT_FOR_CMD; 
	static BOOL pen_down = FALSE;
	static BOOL Scale_Inch = TRUE;
	
	static int new_pos = 0;			// keeps track whether or not the last line of input had a new X or Y position command
	
	float tmp_f;
	int tmp_i;
	int8_t ret = CMD_CONT;			// default return is to continue parsing
	
	
// the goto recheck's are there so that the newline handling can be done in one single place 	
recheck:	
	
	// A overrun of the temp scratchpad is about to happen due to a mis formed argument 
	if (tmp_ndx>= SCRATCHPAD_SIZE)
	{
		tmp_ndx = 0;
		return CMD_ERR;
	}
	
	

	switch (State)
	{
		case WAIT_FOR_CMD:
			// if c == one of the commands G,M,X,Y,Z,;,(
			switch (c)
			{
				case 'G':
					State = WAIT_FOR_G;
					tmp_ndx=0;
				break;
				
				case 'M':
					State = WAIT_FOR_M;
					tmp_ndx=0;
				break;
				
				case 'X':
					State = WAIT_FOR_X;
					tmp_ndx=0;
				break;
				
				case 'Y':
					State = WAIT_FOR_Y;
					tmp_ndx=0;
				break;
				
				case 'Z':
					State = WAIT_FOR_Z;
					tmp_ndx=0;
				break;
				
				case ';':  // sometimes used as a comment to end of line
				case '(':  // General comment
					State = SKIP_COMMENT;
				break;
				
				case '\r':
					State = PROCESS_LINE;
					goto recheck;
				break;
	
			}			
		break;
			
			
		case SKIP_COMMENT:
			if ( c == ')' )		// close comment 
				State = WAIT_FOR_CMD;
				
			if ( c == '\r'  )
			{
				State = PROCESS_LINE;
				goto recheck;
			}
			
		break;
			
		case WAIT_FOR_G:	
		// there is not much we need to know from the G code -- except if the positions are in mm or inch, G20 or G21
		// the speed is set by the dials on the cutter and we don't do radius cuts 
			if ( is_numeric(c) )
				scratchpad[tmp_ndx++]=c;	// collect the string of numbers 
			else
			{
				scratchpad[tmp_ndx]=0;
				tmp_i = atoi(scratchpad);
				
				if (tmp_i == 20 )
				{
					if (Scale_Inch == FALSE)	// switch from mm to inch 
					{
//TODO: must keep floating point var for X and Y around so we can convert from mm to inch and back if program switches midstream --
// This is not likely however and therefore we don't convert here						dx /=25.4
						Scale_Inch = TRUE;
					}
				}
					
					
				if (tmp_i == 21 )
				{
					if (Scale_Inch == TRUE)	// switch from mm to inch 
					{
						Scale_Inch = FALSE;
					}
				}
					
				State = WAIT_FOR_CMD;
				goto recheck;
				
			}
		break;
		
		case WAIT_FOR_M:// There is not much we need to know from the Mcodes except M30 maybe -- end of program -- go home again  
			if ( is_numeric(c) )
				scratchpad[tmp_ndx++]=c;	// collect the string of numbers 
			else
			{
				scratchpad[tmp_ndx]=0;
				tmp_i = atoi(scratchpad);
				
				if (tmp_i == 30 )		// This is program end
				{ 
					dx=dy=0;
					*x = dx;
					*y = dy;
					ret = CMD_SEEK0;
				}
				State = WAIT_FOR_CMD;
				goto recheck;
				
			}
		break;

			
		case WAIT_FOR_Z:
		// collect the number into the scratchpad -- terminate on non numeric and set pen_up from the Z number -- neg number is down else up
		// TODO:if there is a space between the command and the number we get 0, not sure if that's valid syntax or not
		// same for X,and Y

			if ( is_numeric(c) )
				scratchpad[tmp_ndx++]=c;	// collect the string of numbers 
			else
			{
				scratchpad[tmp_ndx]=0;
				tmp_f = atof( scratchpad);
				
				if (tmp_f < 0 )
					pen_down = TRUE;
				else
					pen_down = FALSE;
					
				State = WAIT_FOR_CMD;
				goto recheck;
				
			}
		break;
			
		case WAIT_FOR_X:
			if ( is_numeric(c) )
				scratchpad[tmp_ndx++]=c;	// collect the string of numbers 
			else
			{
				scratchpad[tmp_ndx]=0;			// This now contains the ASCII number string, i.e. "1.232" or "-1.234" 
				tmp_f = atof( scratchpad);		// Convert to float --  

				
				if (!Scale_Inch)
					tmp_f /= 25.4; // MM to inch 
					
				dx= tmp_f *DPI;
				new_pos++;			// indicates that we got a new position in this line of program
				State = WAIT_FOR_CMD;
				goto recheck;
			}
		break;
			
		case  WAIT_FOR_Y:
			if ( is_numeric(c) )
				scratchpad[tmp_ndx++]=c;	// collect the string of numbers 
			else
			{
				scratchpad[tmp_ndx]=0;
				tmp_f = atof( scratchpad);
				
				if (!Scale_Inch)
					tmp_f /= 25.4; // MM to inch
					
				dy= tmp_f *DPI;
				new_pos++;			// indicates that we got a new position on this line of program
				State = WAIT_FOR_CMD;
				goto recheck;
			}
		break;
		
		case PROCESS_LINE:
			State = WAIT_FOR_CMD;	// next character 
				
			if (new_pos)
			{
				*x = dx;		// send new coordinates back to move/cut to
				*y = dy;
					
				if ( pen_down )
				ret = CMD_PD;
				else
				ret = CMD_PU;
			}
				
			new_pos = 0;
		break;
			
			
		// This should not happen except if missing a case or not initialized
		default:
			State = WAIT_FOR_CMD;		
		break;
			
	
	}
	

	return ret;
}		