/*
 * display.h
 *
 * Created: 2/15/2015 12:03:50 PM
 *  Author: Mark
 */ 

#ifdef MACHINE_EXPRESSION
#ifndef DISPLAY_OLED_H_
#define DISPLAY_OLED_H_

void oled_display_init(void);
void oled_display_firstpage(void);
int oled_display_nextpage(void);
int oled_display_putc( char c );
void oled_display_puts(char *s) ;
void oled_display_println(char *s) ;
#endif
#endif //#ifdef MACHINE_EXPRESSION