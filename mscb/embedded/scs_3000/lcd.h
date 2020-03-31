/********************************************************************\

  Header file for LCD functions for four line text LCD

\********************************************************************/

void lcd_setup();
void lcd_clear();
void lcd_line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2);
void lcd_goto(unsigned short x, unsigned short y);
char putchar(char c);
void lcd_puts(char *str);
void lcd_cursor(unsigned char flag);
void lcd_font(unsigned char f);
void lcd_fontcolor(unsigned char fg, unsigned char bg);
void lcd_fontzoom(unsigned char zx, unsigned char zy);
void lcd_text(unsigned short x, unsigned short y, char align, char *str);

#define BLACK 1
#define BLUE  2
#define RED   3
#define GREEN 4
#define WHITE 8