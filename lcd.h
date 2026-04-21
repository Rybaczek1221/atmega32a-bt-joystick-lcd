#ifndef LCD_H_
#define LCD_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

void lcd_init(void);
void lcd_send_nibble(uint8_t nibble);
void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_send_string(const char *str);
void lcd_clear(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_clear_line(uint8_t row);
void lcd_print_two_lines(const char *line1, const char *line2);
void lcd_send_number(uint16_t num);

#endif /* LCD_H_ */
