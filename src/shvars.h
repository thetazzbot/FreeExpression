#ifndef _SHVARS_H
#define _SHVARS_H


///< Absolute coordinates used for stepper motion.
///< Negative means invalid.
typedef int16_t STEPPER_COORD;

///< User coordinates used in input, arc calculation etc.
typedef double USER_COORD;


typedef struct _stepper_xy {
	STEPPER_COORD x;
	STEPPER_COORD y;
} STEPPER_POINT;

typedef struct _user_xy {
	USER_COORD x;
	USER_COORD y;
} USER_POINT;

/// Acceleration modes.
/// See configs.h for marginal values.
typedef enum _acceleration_modes {
	ACCEL_FIXSLOW = 0,		///< Move slowly, fixed speed
	ACCEL_FIXFAST,			///< Move fast, fastest possible speed
	ACCEL_FIXMEDIUM,		///< Fixed average speed
	ACCEL_ACCEL,			///< Ramp speed up
	ACCEL_DECEL,			///< Ramp speed down
} ACCEL_MODE;

extern char scratchpad[];		///< character scratchpad used by scanner
extern double numpad[];		///< stored parameters of AA and similar commands
extern int32_t ip_pad[];		///< stored parameters of IP command (4)
extern int32_t sc_pad[];		///< stored parameters of SC command (4)

extern STEPPER_POINT stepper_loc;      ///< absolute stepper coordinates

extern USER_POINT user_loc;		///< rounded location in user coordinate system (used for arc calculation)

extern uint8_t borderflags;	///< Margin flags: MSB [0 0  ZINF ZSUP YINF YSUP XINF XSUP] LSB

#endif