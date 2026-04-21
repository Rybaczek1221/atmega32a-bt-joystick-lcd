#define F_CPU 11059200UL

#include "lcd.h"

void lcd_clear(void)
{
    lcd_send_cmd(0x01);
    _delay_ms(2);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr;

    if (row == 0)
    {
        addr = 0x80 + col;
    }
    else
    {
        addr = 0xC0 + col;
    }

    lcd_send_cmd(addr);
}

void lcd_clear_line(uint8_t row)
{
    uint8_t i;

    lcd_set_cursor(row, 0);
    for (i = 0; i < 16; i++)
    {
        lcd_send_data(' ');
    }
    lcd_set_cursor(row, 0);
}

void lcd_print_two_lines(const char *line1, const char *line2)
{
    lcd_clear();

    lcd_set_cursor(0, 0);
    lcd_send_string(line1);

    lcd_set_cursor(1, 0);
    lcd_send_string(line2);
}

void lcd_send_number(uint16_t num)
{
    char buf[6];
    uint8_t i = 0;
    uint8_t j;

    if (num == 0)
    {
        lcd_send_data('0');
        return;
    }

    while (num > 0)
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    for (j = i; j > 0; j--)
    {
        lcd_send_data(buf[j - 1]);
    }
}

void lcd_send_nibble(uint8_t nibble)
{
    PORTA &= ~((1 << PA6) | (1 << PA5) | (1 << PA4) | (1 << PA3));

    if (nibble & (1 << 3)) PORTA |= (1 << PA6); /* D7 */
    if (nibble & (1 << 2)) PORTA |= (1 << PA5); /* D6 */
    if (nibble & (1 << 1)) PORTA |= (1 << PA4); /* D5 */
    if (nibble & (1 << 0)) PORTA |= (1 << PA3); /* D4 */

    PORTA |= (1 << PA1);
    _delay_us(1);
    PORTA &= ~(1 << PA1);
    _delay_us(100);
}

void lcd_send_cmd(uint8_t cmd)
{
    PORTA &= ~(1 << PA0);

    lcd_send_nibble(cmd >> 4);
    lcd_send_nibble(cmd & 0x0F);

    _delay_us(100);
}

void lcd_send_data(uint8_t data)
{
    PORTA |= (1 << PA0);

    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);

    _delay_us(100);
}

void lcd_send_string(const char *str)
{
    while (*str)
    {
        lcd_send_data(*str);
        str++;
    }
}

void lcd_init(void)
{
    DDRA |= (1 << PA6) | (1 << PA5) | (1 << PA4) | (1 << PA3);
    PORTA &= ~((1 << PA6) | (1 << PA5) | (1 << PA4) | (1 << PA3));

    DDRA |= (1 << PA0) | (1 << PA1);

    _delay_ms(15);

    PORTA &= ~((1 << PA0) | (1 << PA1));

    lcd_send_nibble(0x03);
    _delay_ms(5);

    lcd_send_nibble(0x03);
    _delay_us(100);

    lcd_send_nibble(0x03);
    _delay_us(100);

    lcd_send_nibble(0x02);
    _delay_us(100);

    lcd_send_cmd(0x28);
    lcd_send_cmd(0x0C);
    lcd_send_cmd(0x06);
    lcd_send_cmd(0x01);
    _delay_ms(2);
}
