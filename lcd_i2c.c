#include "lcd_i2c.h"
#include "ti_msp_dl_config.h"
#include <stdint.h>

// Function to send data to the LCD
void lcd_write(uint8_t data) {
    while (!(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE)); // Wait until the bus is idle
    DL_I2C_startControllerTransfer(I2C_INST, LCD_ADDR, DL_I2C_CONTROLLER_DIRECTION_TX, 1);
    DL_I2C_fillControllerTXFIFO(I2C_INST, &data, 1);
    while (DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS); // Wait until the bus is free
}

// Function to send a command to the LCD
void lcd_send_command(uint8_t cmd) {
    uint8_t high = (cmd & 0xF0) | 0x08;  // Prepare high nibble with RS = 0 (command mode)
    uint8_t low = ((cmd << 4) & 0xF0) | 0x08;  // Prepare low nibble with RS = 0 (command mode)

    // Send high nibble with En = 1
    lcd_write(high | 0x04);  // En = 1
    delay_ms(1);  // Delay for the LCD to process the data
    lcd_write(high);  // En = 0

    // Send low nibble with En = 1
    lcd_write(low | 0x04);  // En = 1
    delay_ms(1);  // Delay for the LCD to process the data
    lcd_write(low);  // En = 0
}

// Function to send data to the LCD
void lcd_send_data(uint8_t data) {
    uint8_t high = (data & 0xF0) | 0x09;  // Prepare high nibble with RS = 1 (data mode)
    uint8_t low = ((data << 4) & 0xF0) | 0x09;  // Prepare low nibble with RS = 1 (data mode)

    // Send high nibble with En = 1
    lcd_write(high | 0x04);  // En = 1
    delay_ms(1);  // Delay for the LCD to process the data
    lcd_write(high);  // En = 0

    // Send low nibble with En = 1
    lcd_write(low | 0x04);  // En = 1
    delay_ms(1);  // Delay for the LCD to process the data
    lcd_write(low);  // En = 0
}

// Function to initialize the LCD
void lcd_init() {
    delay_ms(50);  // Wait for LCD to power up

    lcd_send_command(0x33);  // Initialize LCD in 8-bit mode
    delay_ms(5);  // Wait for the LCD to process the command

    lcd_send_command(0x32);  // Switch to 4-bit mode
    delay_ms(5);  // Wait for the LCD to process the command

    lcd_send_command(0x28);  // 4-bit, 2-line, 5x8 font
    delay_ms(5);  // Wait for the LCD to process the command

    lcd_send_command(0x0C);  // Display on, cursor off
    delay_ms(5);  // Wait for the LCD to process the command

    lcd_send_command(0x06);  // Entry mode
    delay_ms(5);  // Wait for the LCD to process the command

    lcd_send_command(0x01);  // Clear display
    delay_ms(2);  // Wait for the LCD to clear the display
}

// Function to clear the LCD display
void lcd_clear() {
    lcd_send_command(0x01);  // Send clear command
    delay_ms(2);  // Wait for the LCD to process the command
}

// Function to set the cursor position on the LCD
void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};  // Row offsets for 2-line LCDs
    lcd_send_command(0x80 | (col + row_offsets[row]));  // Set DDRAM address for cursor position
    delay_ms(1);  // Wait for the LCD to process the command
}

// Function to print a string to the LCD
void lcd_print(const char *str) {
    while (*str) {
        lcd_send_data(*str++);  // Send each character to the LCD
        delay_ms(1);  // Delay for each character to be displayed
    }
}

// Function to add a delay (for simulation, replace with actual delay function)
void delay_ms(uint32_t ms) {
    volatile uint32_t i;
    for (i = 0; i < ms * 1000; i++) {
        // Busy-wait loop for delay, you can replace this with a real delay function
    }
}

