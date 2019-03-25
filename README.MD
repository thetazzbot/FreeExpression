# Open Source firmware replacement for Cricut(tm) Expression and Cake stencil cutter machines. #

----------

**IMPORTANT: REPROGRAMMING THE CPU WILL PERMANENTLY ERASE YOUR ORIGINAL FIRMWARE
THERE IS NO WAY TO GET THE ORIGINAL FIRMWARE BACK OTHER THAN BUYING A NEW MACHINE.** 

*******************************************************************************

1/13/2019
Updated project to Atmel Studio 7

6/11/2017 
Merged pull request from Gary Stofer to improve HGPL reliability

Previous notes:

Original firmware cannot be copied from another machine (it has been
locked and cannot be read from the CPU)

Original firmware cannot be restored through any PC application used for
normal firmware upgrades.

This firmware has only been tested on the machine that I have. It
may not work on your machine.  It may cause permanent damage, and it
will certainly void your warranty.

This firmware cannot read your original cartridges ever.

Cricut is a trademark of PROVO CRAFT and this firmware is **not** a PROVO CRAFT product. This firmware  was developed completely independently, without any documentation of PROVO CRAFT products, and without any and all original firmware. If you have problems with this firmware, do not consult PROVO CRAFT. 



This work was forked from https://github.com/thetazzbot/FreeExpression which originally was forked from https://github.com/Arlet/Freecut  


This fork uses a Standard 4x20 LCD with a SPI interface in place of the graphics LCD that comes on the expression machine. See the hack_notes file for how this was added to the machine. *I did not want to spend the time and effort to figure out how to control the original LCD.*



If you still want to proceed:


You will need a ATMEL programmer device and an Atmel Studio 6.2 or newer installation. I'm using an ATMEL JTAG ICE MKII pod and the JTAG interface of the AVR. See picture and detailing connection and file Connector pinout.txt for details.


Use Inkscape to draw and send a file to the cutter.  Inkscape 0.91 has an export to Plot feature (under Extensions) which sends a HPGL langauge file to the COM port where the cutter is connected to. Use 9600 baud and  make sure XON/XOFF is selected for flow control. Inkscape works good on WIndows and Linux machines. 


----------


## Keypad key functions: ##

- On/Off : Powers the machine and executes carriage homing sequence.
- Move buttons : Moves the carriage and the media freely.
- Home (ResetAll) : Re-homes the carriage, media is assumed to be unloaded.
- Load Paper		 : Pulls in media or cutting mat about 1/2".
- Unload Paper	 : Moves the mat/media back to the loading point. 
- X,Y Offset (BackSpace): Sets the current X and Y position as location 0,0 for following cuts
- Cutter Down (Numeral 1) : Drops the cutter down. This only works when the media is loaded. Useful  for measuring actual cutting force with a scale.
- Cutter Up (Numeral 2)  : Moves the cutter away from the media.
- Cutting Speed	(Xtra2) : In conjunction with the +/keys the cutting speed can be chosen in 9 steps.
- Cutting Pressure (Xtra1) : In conjunction with the +/keys the cutting pressure can be adjusted in 9 steps.
- Letter H : HPGL language selection, (default)
- Letter G: G-code language selection, see file g-code.c for supported g-code tokens

- Stop: Aborts the currently cached cutting operations, but new data arriving from the PC will still trigger subsequent motion.

## Dials ##

- Dial Speed: 	Also adjusts the cutting speed and is used to read the cutting speed after power up. Only the mid range of the speed choices can be selected, use the +/keys to get all the way to the end.

- Dial Pressure: Also adjusts the cutting pressure and is used to read the initial pressure on power-up. Only the mid range of the pressure choices can be selected, use the +/keys for the full range.

- Dial Size:  Not used. The machine can cut up to 80"x12"wide.

## Multi-cut and media lock ##

After the machine moves the media/carriage there is a time of about 1 minute (depending on speed setting) where the motors stay engaged and you can not move the media/carriage by hand. After this timeout the motors go into standby and you can move carriage and media by hand. This timeout is useful so that one can cut the same shape multiple times and not loose registration - for thick material multi-cut.

# Command language #


Currently supports HPGL -- HP Graphic Language. Cutting speed and pressure is NOT taken from the language input, only from keyboard and associated dials.

After the machine moves the media/carriage there is a time of about 1 minute at speed 5 where the motors stay engaged and you can not move the media/carriage by hand. After this timeout the motors go into standby and you can move carriage and media by hand. This timeout is useful so that one can cut the same shape multiple times and not loose registration - for thick material multi-cut.

Hope to add an option for GPGL (Graphtec)  in the future. 

# CAD and Cutting #
From the CAD system export via DXF format, then open that file in Inkscape. On the input dialog choose manual scale factor of 25.4 if the original CAD file was done in inch scale, otherwise set to 1.0. 
To cut, select all objects then use PATH/Object to Path, the Extensions/Export/Plot.  Plotter resolution set to 400 dpi.  Pen number, force and speed are ignored. Rotation and Mirror as desired. 
Connection Settings are 9600/XON/XOFF and HPGL for language. Com port number as per device manger.   To execute press Apply. 


 

