#define F_CPU 11059200UL

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdint.h>

#include "lcd.h"

#define RX_BUFFER_SIZE 40

#define JOY_X_CHANNEL 2
#define JOY_Y_CHANNEL 3
#define JOY_SWITCH_PIN PD5
#define SERVO_PIN PD4

typedef enum
{
    MODE_MENU = 0,
    MODE_ABOUT,
    MODE_APP,
    MODE_BT_TEXT,
    MODE_BT_CTRL,
    MODE_JOY_MONITOR,
    MODE_SERVO
} AppMode;

static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_index = 0;

static uint8_t bt_stream = 0;
static uint16_t bt_stream_div = 0;

static uint8_t servo_level = 5;
static AppMode current_mode = MODE_MENU;
static uint8_t menu_index = 0;

// UART
static void uart_init(void);
static void uart_send_char(char c);
static void uart_send_string(const char *str);
static void uart_send_number(uint16_t num);
static uint8_t uart_available(void);
static char uart_receive_char(void);

// ADC and joystick
static void adc_init(void);
static uint16_t adc_read(uint8_t channel);
static const char *joystick_direction(uint16_t x, uint16_t y);
static uint8_t sw_pressed_event(void);

// Screen flow
static void show_menu(uint8_t index);
static void about_mode_loop(void);
static void app_mode_loop(void);
static void bt_text_mode_loop(void);
static void bt_ctrl_mode_loop(void);
static void joy_monitor_mode_loop(void);
static void servo_mode_loop(void);

// Bluetooth commands
static void bt_read_line(void);
static void bt_text_read_line(void);
static void bt_handle_ctrl(char *line);
static void send_joystick_report(void);
static void send_bt_text_help(void);
static void send_bt_ctrl_help(void);

// Servo
static void servo_init(void);
static void servo_set_us(uint16_t pulse_us);
static void servo_set_level(uint8_t level);
static uint16_t servo_level_to_us(uint8_t level);

// Helpers
static void clear_buffer(char *buf, uint8_t len);
static void lcd_print_text_2x16(const char *text);
static void lcd_print_text_with_title(const char *title, const char *text);

int main(void)
{
    uart_init();
    lcd_init();
    adc_init();
    servo_init();

    DDRD &= ~(1 << JOY_SWITCH_PIN);
    PORTD |= (1 << JOY_SWITCH_PIN);

    clear_buffer(rx_buffer, RX_BUFFER_SIZE);

    lcd_print_two_lines("System start", "LCD+JOY+BT");
    _delay_ms(1000);

    uart_send_string("\r\nREADY\r\n");
    uart_send_string("System OK\r\n");

    show_menu(menu_index);

    while (1)
    {
        switch (current_mode)
        {
            case MODE_MENU:
            {
                uint16_t y = adc_read(JOY_Y_CHANNEL);

                if (y < 300)
                {
                    if (menu_index > 0)
                    {
                        menu_index--;
                        show_menu(menu_index);
                        _delay_ms(220);
                    }
                }

                if (y > 700)
                {
                    if (menu_index < 5)
                    {
                        menu_index++;
                        show_menu(menu_index);
                        _delay_ms(220);
                    }
                }

                if (sw_pressed_event())
                {
                    clear_buffer(rx_buffer, RX_BUFFER_SIZE);
                    rx_index = 0;

                    switch (menu_index)
                    {
                        case 0:
                            current_mode = MODE_ABOUT;
                            lcd_print_two_lines("Jaroslaw Rybak", "275773");
                            break;

                        case 1:
                            current_mode = MODE_APP;
                            lcd_print_two_lines("Aplikacja:", "BT Controller");
                            break;

                        case 2:
                            current_mode = MODE_BT_TEXT;
                            lcd_print_two_lines("BT TEXT", "pisz z tel.");
                            send_bt_text_help();
                            break;

                        case 3:
                            current_mode = MODE_BT_CTRL;
                            lcd_print_two_lines("BT CTRL", "1=HELP");
                            send_bt_ctrl_help();
                            break;

                        case 4:
                            current_mode = MODE_JOY_MONITOR;
                            lcd_print_two_lines("JOY MONITOR", "SW=powrot");
                            break;

                        case 5:
                            current_mode = MODE_SERVO;
                            lcd_print_two_lines("SERVO TEST", "JOY steruje");
                            break;
                    }
                }

                break;
            }

            case MODE_ABOUT:
                about_mode_loop();
                break;

            case MODE_APP:
                app_mode_loop();
                break;

            case MODE_BT_TEXT:
                bt_text_mode_loop();
                break;

            case MODE_BT_CTRL:
                bt_ctrl_mode_loop();
                break;

            case MODE_JOY_MONITOR:
                joy_monitor_mode_loop();
                break;

            case MODE_SERVO:
                servo_mode_loop();
                break;
        }

        _delay_ms(10);
    }
}

// Menu and mode handlers
static void show_menu(uint8_t index)
{
    lcd_clear();

    switch (index)
    {
        case 0:
            lcd_set_cursor(0, 0);
            lcd_send_string(">O mnie");
            lcd_set_cursor(1, 0);
            lcd_send_string(" Aplikacja");
            break;

        case 1:
            lcd_set_cursor(0, 0);
            lcd_send_string(">Aplikacja");
            lcd_set_cursor(1, 0);
            lcd_send_string(" BT TEXT");
            break;

        case 2:
            lcd_set_cursor(0, 0);
            lcd_send_string(">BT TEXT");
            lcd_set_cursor(1, 0);
            lcd_send_string(" BT CTRL");
            break;

        case 3:
            lcd_set_cursor(0, 0);
            lcd_send_string(">BT CTRL");
            lcd_set_cursor(1, 0);
            lcd_send_string(" JOY MON");
            break;

        case 4:
            lcd_set_cursor(0, 0);
            lcd_send_string(">JOY MON");
            lcd_set_cursor(1, 0);
            lcd_send_string(" SERVO");
            break;

        case 5:
            lcd_set_cursor(0, 0);
            lcd_send_string(">SERVO");
            lcd_set_cursor(1, 0);
            lcd_send_string(" SW=enter");
            break;
    }
}

static void about_mode_loop(void)
{
    if (sw_pressed_event())
    {
        current_mode = MODE_MENU;
        show_menu(menu_index);
    }
}

static void app_mode_loop(void)
{
    if (sw_pressed_event())
    {
        current_mode = MODE_MENU;
        show_menu(menu_index);
    }
}

static void bt_text_mode_loop(void)
{
    bt_text_read_line();

    if (sw_pressed_event())
    {
        current_mode = MODE_MENU;
        show_menu(menu_index);
    }
}

static void bt_ctrl_mode_loop(void)
{
    bt_read_line();

    if (bt_stream)
    {
        bt_stream_div++;
        if (bt_stream_div >= 50)
        {
            bt_stream_div = 0;
            send_joystick_report();
        }
    }

    if (sw_pressed_event())
    {
        bt_stream = 0;
        current_mode = MODE_MENU;
        show_menu(menu_index);
    }
}

static void joy_monitor_mode_loop(void)
{
    uint16_t x = adc_read(JOY_X_CHANNEL);
    uint16_t y = adc_read(JOY_Y_CHANNEL);

    lcd_clear();

    lcd_set_cursor(0, 0);
    lcd_send_string("X:");
    lcd_send_number(x);

    lcd_set_cursor(0, 9);
    lcd_send_string("Y:");
    lcd_send_number(y);

    lcd_set_cursor(1, 0);
    lcd_send_string(joystick_direction(x, y));

    if (sw_pressed_event())
    {
        current_mode = MODE_MENU;
        show_menu(menu_index);
    }

    _delay_ms(150);
}

static void servo_mode_loop(void)
{
    uint16_t x = adc_read(JOY_X_CHANNEL);

    if (x < 80)
    {
        servo_level = 1;
    }
    else if (x < 180)
    {
        servo_level = 2;
    }
    else if (x < 300)
    {
        servo_level = 3;
    }
    else if (x < 430)
    {
        servo_level = 4;
    }
    else if (x < 590)
    {
        servo_level = 5;
    }
    else if (x < 720)
    {
        servo_level = 6;
    }
    else if (x < 840)
    {
        servo_level = 7;
    }
    else if (x < 940)
    {
        servo_level = 8;
    }
    else
    {
        servo_level = 9;
    }

    servo_set_level(servo_level);

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("Servo level:");
    lcd_send_number(servo_level);

    lcd_set_cursor(1, 0);
    lcd_send_string("X:");
    lcd_send_number(x);

    if (sw_pressed_event())
    {
        current_mode = MODE_MENU;
        show_menu(menu_index);
    }

    _delay_ms(100);
}

// UART
static void uart_init(void)
{
    uint16_t ubrr = 35;

    UBRRH = (uint8_t)(ubrr >> 8);
    UBRRL = (uint8_t)ubrr;

    UCSRB = (1 << RXEN) | (1 << TXEN);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

static void uart_send_char(char c)
{
    while (!(UCSRA & (1 << UDRE)))
    {
    }
    UDR = c;
}

static void uart_send_string(const char *str)
{
    while (*str)
    {
        uart_send_char(*str);
        str++;
    }
}

static void uart_send_number(uint16_t num)
{
    char buffer[6];
    uint8_t length = 0;

    if (num == 0)
    {
        uart_send_char('0');
        return;
    }

    while (num > 0)
    {
        buffer[length++] = (num % 10U) + '0';
        num /= 10U;
    }

    while (length > 0)
    {
        uart_send_char(buffer[--length]);
    }
}

static uint8_t uart_available(void)
{
    return (UCSRA & (1 << RXC));
}

static char uart_receive_char(void)
{
    return UDR;
}

// ADC and joystick
static void adc_init(void)
{
    ADMUX = (1 << REFS0);

    ADCSRA =
        (1 << ADEN) |
        (1 << ADPS2) |
        (1 << ADPS1) |
        (1 << ADPS0);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
    {
    }
}

static uint16_t adc_read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xE0) | (channel & 0x07);

    _delay_us(20);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
    {
    }

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
    {
    }

    return ADC;
}

static const char *joystick_direction(uint16_t x, uint16_t y)
{
    if (y < 300)
    {
        return "GORA";
    }
    if (y > 700)
    {
        return "DOL";
    }
    if (x < 300)
    {
        return "LEWO";
    }
    if (x > 700)
    {
        return "PRAWO";
    }
    return "SRODEK";
}

static uint8_t sw_pressed_event(void)
{
    static uint8_t last_state = 1;
    uint8_t current = (PIND & (1 << JOY_SWITCH_PIN)) ? 1 : 0;

    if ((last_state == 1) && (current == 0))
    {
        _delay_ms(20);
        current = (PIND & (1 << JOY_SWITCH_PIN)) ? 1 : 0;

        if (current == 0)
        {
            while (!(PIND & (1 << JOY_SWITCH_PIN)))
            {
            }
            _delay_ms(20);
            last_state = 1;
            return 1;
        }
    }

    last_state = current;
    return 0;
}

// Bluetooth text and control
static void send_bt_text_help(void)
{
    uart_send_string("\r\nBT TEXT MODE\r\n");
    uart_send_string("Wpisz tekst i ENTER.\r\n");
    uart_send_string("Tekst pojawi sie na LCD.\r\n");
    uart_send_string("SW -> powrot do menu\r\n");
}

static void send_bt_ctrl_help(void)
{
    uart_send_string("\r\nBT CTRL MODE\r\n");
    uart_send_string("1  -> help\r\n");
    uart_send_string("2tekst -> pokaz tekst na LCD\r\n");
    uart_send_string("3  -> jednorazowy odczyt joysticka\r\n");
    uart_send_string("4  -> start stream joysticka\r\n");
    uart_send_string("5  -> stop stream joysticka\r\n");
    uart_send_string("6  -> pokaz dane autora\r\n");
    uart_send_string("7  -> clear LCD\r\n");
    uart_send_string("81..89 -> servo poziom 1..9\r\n");
    uart_send_string("90 -> servo center\r\n");
    uart_send_string("SW -> powrot do menu\r\n");
}

static void bt_text_read_line(void)
{
    while (uart_available())
    {
        char c = uart_receive_char();

        if (c == '\r' || c == '\n')
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0';

                lcd_print_text_with_title("BT TEXT:", rx_buffer);

                uart_send_string("TXT OK: ");
                uart_send_string(rx_buffer);
                uart_send_string("\r\n");

                rx_index = 0;
                clear_buffer(rx_buffer, RX_BUFFER_SIZE);
            }
        }
        else if (rx_index < (RX_BUFFER_SIZE - 1))
        {
            rx_buffer[rx_index++] = c;
        }
    }
}

static void bt_read_line(void)
{
    while (uart_available())
    {
        char c = uart_receive_char();

        if (c == '\r' || c == '\n')
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0';
                bt_handle_ctrl(rx_buffer);
                rx_index = 0;
                clear_buffer(rx_buffer, RX_BUFFER_SIZE);
            }
        }
        else if (rx_index < (RX_BUFFER_SIZE - 1))
        {
            rx_buffer[rx_index++] = c;
        }
    }
}

static void bt_handle_ctrl(char *line)
{
    uart_send_string("CMD: ");
    uart_send_string(line);
    uart_send_string("\r\n");

    if (strcmp(line, "1") == 0)
    {
        send_bt_ctrl_help();
        lcd_print_two_lines("BT CTRL", "HELP SENT");
    }
    else if (line[0] == '2' && line[1] != '\0')
    {
        lcd_print_text_2x16(line + 1);
        uart_send_string("OK TXT\r\n");
    }
    else if (strcmp(line, "3") == 0)
    {
        send_joystick_report();
    }
    else if (strcmp(line, "4") == 0)
    {
        bt_stream = 1;
        uart_send_string("STREAM ON\r\n");
        lcd_print_two_lines("BT STREAM", "ON");
    }
    else if (strcmp(line, "5") == 0)
    {
        bt_stream = 0;
        uart_send_string("STREAM OFF\r\n");
        lcd_print_two_lines("BT STREAM", "OFF");
    }
    else if (strcmp(line, "6") == 0)
    {
        lcd_print_two_lines("Jaroslaw Rybak", "275773");
        uart_send_string("OK ABOUT\r\n");
    }
    else if (strcmp(line, "7") == 0)
    {
        lcd_clear();
        uart_send_string("OK CLEAR\r\n");
    }
    else if ((strlen(line) == 2) && (line[0] == '8') && (line[1] >= '1') && (line[1] <= '9'))
    {
        servo_level = line[1] - '0';
        servo_set_level(servo_level);

        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_send_string("SERVO LEVEL");
        lcd_set_cursor(1, 0);
        lcd_send_number(servo_level);

        uart_send_string("OK SERVO ");
        uart_send_char(line[1]);
        uart_send_string("\r\n");
    }
    else if (strcmp(line, "90") == 0)
    {
        servo_level = 5;
        servo_set_level(servo_level);
        lcd_print_two_lines("SERVO", "CENTER");
        uart_send_string("OK SERVO CENTER\r\n");
    }
    else
    {
        uart_send_string("ERR\r\n");
        lcd_print_two_lines("Unknown cmd", line);
    }
}

static void send_joystick_report(void)
{
    uint16_t x = adc_read(JOY_X_CHANNEL);
    uint16_t y = adc_read(JOY_Y_CHANNEL);

    uart_send_string("JOY X=");
    uart_send_number(x);
    uart_send_string(" Y=");
    uart_send_number(y);
    uart_send_string(" DIR=");
    uart_send_string(joystick_direction(x, y));
    uart_send_string(" SW=");

    if ((PIND & (1 << JOY_SWITCH_PIN)) == 0)
    {
        uart_send_string("PRESSED");
    }
    else
    {
        uart_send_string("RELEASED");
    }

    uart_send_string("\r\n");
}

// Servo control with Timer1
static void servo_init(void)
{
    DDRD |= (1 << SERVO_PIN);

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    TCCR1A = (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);

    ICR1 = 27647;

    servo_set_level(5);
}

static void servo_set_us(uint16_t pulse_us)
{
    uint32_t ticks = ((uint32_t)pulse_us * 1382UL) / 1000UL;
    OCR1B = (uint16_t)ticks;
}

static uint16_t servo_level_to_us(uint8_t level)
{
    switch (level)
    {
        case 1: return 700;
        case 2: return 900;
        case 3: return 1100;
        case 4: return 1300;
        case 5: return 1500;
        case 6: return 1700;
        case 7: return 1900;
        case 8: return 2100;
        case 9: return 2300;
        default: return 1500;
    }
}

static void servo_set_level(uint8_t level)
{
    servo_set_us(servo_level_to_us(level));
}

// Shared helpers
static void clear_buffer(char *buf, uint8_t len)
{
    uint8_t index;

    for (index = 0; index < len; index++)
    {
        buf[index] = 0;
    }
}

static void lcd_print_text_2x16(const char *text)
{
    uint8_t index;

    lcd_clear();

    lcd_set_cursor(0, 0);
    for (index = 0; (index < 16) && (text[index] != '\0'); index++)
    {
        lcd_send_data(text[index]);
    }

    lcd_set_cursor(1, 0);
    for (index = 16; (index < 32) && (text[index] != '\0'); index++)
    {
        lcd_send_data(text[index]);
    }
}

static void lcd_print_text_with_title(const char *title, const char *text)
{
    uint8_t index;

    lcd_clear();

    lcd_set_cursor(0, 0);
    for (index = 0; (index < 16) && (title[index] != '\0'); index++)
    {
        lcd_send_data(title[index]);
    }

    lcd_set_cursor(1, 0);
    for (index = 0; (index < 16) && (text[index] != '\0'); index++)
    {
        lcd_send_data(text[index]);
    }
}
