
#include "display.h"
#ifdef MACHINE_CAKE
#include "display_lcd.h"
#endif
#ifdef MACHINE_EXPRESSION
#include "display_oled.h"
#endif


void display_init(void)
{
#ifdef  MACHINE_CAKE
	lcd_init();
#endif
#ifdef  MACHINE_EXPRESSION
	oled_display_init();
#endif
}

/**

*/
void display_puts(char *s) 
{
#ifdef  MACHINE_CAKE
	fprintf( &lcd, "\n%s",s );
#endif
#ifdef  MACHINE_EXPRESSION
	oled_display_puts(s);
#endif

}

void display_print(char *s) 
{
#ifdef  MACHINE_CAKE
	fprintf( &lcd, "%s",s );
#endif
#ifdef  MACHINE_EXPRESSION
	oled_display_println(s);
#endif


}




void display_println(char *s) 
{
#ifdef  MACHINE_CAKE
	fputs(s, &lcd );
#endif
#ifdef  MACHINE_EXPRESSION
	oled_display_println(s);
#endif

}


