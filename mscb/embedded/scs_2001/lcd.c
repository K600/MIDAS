/********************************************************************\

  Name:         lcd.c
  Created by:   Stefan Ritt

  Contents:     LCD routines

  $Id: mscbutil.c 5022 2011-05-04 14:22:33Z ritt $

\********************************************************************/

#include <intrins.h>
#include <string.h>
#include <stdio.h>
#include "mscbemb.h"
#include "lcd.h"

bit lcd_present;

/********************************************************************\

  Routine: lcd_setup, lcd_clear, lcd_goto, lcd_puts, putchar

  Purpose: LCD functions for HD44780/KS0066 compatible LCD displays

           Since putchar is used by printf, this function puts its
           output to the LCD as well
           
           **edited by Lukas Kuenzi; only support for SCS2000 / SCS2001**

\********************************************************************/

#define LCD P2                  // LCD display connected to port2

sbit LCD_RS  = P1 ^ 4;
sbit LCD_R_W = P1 ^ 5;
sbit LCD_E   = P1 ^ 6;
sbit LCD_1D  = P1 ^ 0;
sbit LCD_2D  = P1 ^ 1;
sbit LCD_RESET = P1 ^ 7;

sbit LCD_DB7 = LCD ^ 7;

/*------------------------------------------------------------------*/

void lcd_out( unsigned char d, bit RS )
{
  unsigned char timeout;
  
  SFRPAGE = CONFIG_PAGE;
  P2MDOUT = 0x00;              // Port 2: input / open drain
  LCD_1D = 0;                  // b to a for data: LCD to MCU

  LCD_RS = 0;                  // select BF
  LCD_R_W = 1;
  
   
  LCD_E = 1;
  delay_us(1);
  LCD_E = 0;
  
  for( timeout = 0 ; timeout < 200 ; timeout++ ) 
  {
    delay_us(10);              // let signals settle
    if (!LCD_DB7)              // loop if busy
      break;
  }

   SFRPAGE = CONFIG_PAGE;
   P2MDOUT = 0xFF;              // data output, push pull
   LCD_1D = 1;                  // a to b for data: MCU to LCD
   
   LCD_R_W = 0;
   LCD_RS = RS;  
   LCD = d;

   delay_us(1);

   LCD_E = 1;
   delay_us(1);
   LCD_E = 0;
   delay_us(1);
}

/*------------------------------------------------------------------*/

void lcd_setup()
{  
  lcd_present = 1;
  
  SFRPAGE = CONFIG_PAGE;
  P1MDOUT |= 0x80;              // P1.7: reset pin for lcd, push pull
  
  // rest the lcd
  LCD_RESET = 0;
  delay_ms( 1 );
  LCD_RESET = 1;
  
  // starup delay
  delay_ms(50);

  LCD_E   = 0;
  LCD_R_W = 0;
  LCD_RS  = 0;

  SFRPAGE = CONFIG_PAGE;
  P2MDOUT = 0xFF;             // LCD all push pull
  
  P1MDOUT |= 0x03;            // push pull for P1.0 and P1.1, direction control pins of 74ALVC164245
  LCD_1D = 1;                 // a to b for data
  LCD_2D = 1;                 // a to b for control
  
  // initialize the lcd
  lcd_out(0x34, 0);   // function set: 8-bit, RE=1
  lcd_out(0x0B, 0);   // ext function set: 4-line display inverting cursor
  lcd_out(0x30, 0);   // function set: 4-bit, RE=0
  lcd_out(0x02, 0);   // return home
  lcd_out(0x0C, 0);   // display on, cursor off, blink off
  lcd_out(0x01, 0);   // clear display
  lcd_out(0x34, 0);   // function set: 8bit, RE=1
  lcd_out(0x06, 0);   // entry mode set: increment, entire shift off
  lcd_out(0x30, 0);   // function set: 8bit, RE=0
}

/*------------------------------------------------------------------*/

void lcd_clear()
{
   lcd_out( 0x01, 0 );
   lcd_goto( 0, 0 );
}

void lcd_cursor(unsigned char flag)
{
   if (flag)
      lcd_out(0x0F, 0); // display on, curson on, blink on
   else
      lcd_out(0x0C, 0); // display on, cursor off, blink off
}

/*------------------------------------------------------------------*/

void lcd_goto(char x, char y)
{
  lcd_out((x & 0x1F) | (0x80) | ((y & 0x03) << 5), 0);

}

/*------------------------------------------------------------------*/

char putchar(char c)
{
   if (c != '\r' && c != '\n')
      lcd_out(c, 1);
   return c;
}

/*------------------------------------------------------------------*/

void lcd_puts(char *str)
{            
   while (*str)
      lcd_out(*str++, 1);
}
