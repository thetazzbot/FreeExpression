/*
 * display.h
 *
 * Created: 2/15/2015 12:03:50 PM
 *  Author: Mark
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

void display_init(void);

void display_firstpage(void);
int display_nextpage(void);
int display_putc( char c );
void display_puts(char *s) ;
void display_println(char *s) ;

#endif /* DISPLAY_H_ */