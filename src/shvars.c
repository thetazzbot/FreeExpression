#include <inttypes.h>

#include "shvars.h"




char scratchpad[SCRATCHPAD_SIZE];
double numpad[4];
int32_t ip_pad[4];
int32_t sc_pad[4];

STEPPER_POINT stepper_loc;

USER_POINT user_loc;

uint8_t	borderflags;	///< Margin flags: MSB [0 0  ZINF ZSUP YINF YSUP XINF XSUP] LSB


