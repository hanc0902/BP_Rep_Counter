#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <stdint.h>

#define LCD_ADDR 0x27 // Change if your LCD uses a different address (0x3F is common too)

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET 0x20
#define LCD_SETDDRAMADDR 0x80

#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

#define LCD_2LINE 0x08
#define LCD_5x8DOTS 0x00
#define LCD_4BITMODE 0x00

#define En 0x04  // Enable bit
#define Rs 0x01  // Register select bit
#define BACKLIGHT 0x08

void lcd_init(void);
void lcd_clear(void);
void lcd_send_command(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);
void delay_ms(uint32_t ms);

#endif
