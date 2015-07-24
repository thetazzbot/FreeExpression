
#include "display.h"
#include "lcd.h"

void display_init(void)
{
	lcd_init();
}

/**

*/
void display_puts(char *s) 
{
	fprintf( &lcd, "\n%s",s );

}

void display_print(char *s) 
{
	fprintf( &lcd, "%s",s );

}




void display_println(char *s) 
{

	fputs(s, &lcd );
}


