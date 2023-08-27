#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "ssd1306.h"
#include "button.h"

#define SDA 2
#define SCL 3
#define BUTTON_0 13
#define BUTTON_1 14
#define BUTTON_2 15
#define ROTARY_ENC_CLK 16
#define ROTARY_ENC_DT 17

int led_value = true;

uint32_t globalCurrentTime;

ssd1306_t disp;

bool repeating_timer_callback(struct repeating_timer *t)
{
    led_value = !led_value;
    gpio_put(PICO_DEFAULT_LED_PIN, led_value);

    printf("repeating timer callback\n");
    return true;
}

void init_pins();
void init_display();
void draw_splash();

void set_repeating_timer_fn()
{
    struct repeating_timer timer;
    add_repeating_timer_ms(500, repeating_timer_callback, NULL, &timer);
}
void decode_rotary_encoder(button_t *button_p)
{
    button_t *button = (button_t *)button_p;
    static bool ccw_fall = 0; // bool used when falling edge is triggered
    static bool cw_fall = 0;

    if (button->pin == ROTARY_ENC_DT)
    {
        if ((!cw_fall) && button->state == 1) // cw_fall is set to TRUE when phase A interrupt is triggered
            cw_fall = 1;

        if ((ccw_fall) && button->state == 0) // if ccw_fall is already set to true from a previous B phase trigger, the ccw event will be triggered
        {
            cw_fall = 0;
            ccw_fall = 0;
            // do something here,  for now it is just printing out CW or CCW
            printf("CCW \r\n");
        }
    }

    if (button->pin == ROTARY_ENC_CLK)
    {
        if ((!ccw_fall) && button->state == 1) // ccw leading edge is true
            ccw_fall = 1;

        if ((cw_fall) && button->state == 0) // cw trigger
        {
            cw_fall = 0;
            ccw_fall = 0;
            // do something here,  for now it is just printing out CW or CCW
            printf("CW \r\n");
        }
    }
}

// From button header
void button_callback(button_t *button_p)
{
    button_t *button = (button_t *)button_p;
    printf("Button on pin %d changed its state to %d\n", button->pin, button->state);

    if (button->state && button->pin != ROTARY_ENC_CLK && button->pin != ROTARY_ENC_DT)
        return; // Ignore button release. Invert the logic if using
                // a pullup (internal or external).

    switch (button->pin)
    {
    case BUTTON_0:
        printf("Button 0 is pressed\n");
        break;
    case BUTTON_1:
        printf("Button 1 is pressed\n");
        break;
    case BUTTON_2:
        printf("Button 2 is pressed\n");
        break;
    case ROTARY_ENC_CLK:
    case ROTARY_ENC_DT:
        decode_rotary_encoder(button);
        break;
    }
}

int main()
{
    stdio_init_all();

    init_pins();
    init_display();
    draw_splash();

    // set_repeating_timer_fn();
    create_button(BUTTON_0, button_callback);
    create_button(BUTTON_1, button_callback);
    create_button(BUTTON_2, button_callback);

    create_button(ROTARY_ENC_CLK, button_callback);
    create_button(ROTARY_ENC_DT, button_callback);

    while (true)
    {
        globalCurrentTime = to_ms_since_boot(get_absolute_time());
        tight_loop_contents();
    }

    return 0;
}

void init_pins()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_0);
    gpio_set_dir(BUTTON_0, GPIO_IN);
    gpio_pull_up(BUTTON_0);

    gpio_init(BUTTON_1);
    gpio_set_dir(BUTTON_1, GPIO_IN);
    gpio_pull_up(BUTTON_1);

    gpio_init(BUTTON_2);
    gpio_set_dir(BUTTON_2, GPIO_IN);
    gpio_pull_up(BUTTON_2);

    gpio_init(ROTARY_ENC_CLK);
    gpio_set_dir(ROTARY_ENC_CLK, GPIO_IN);
    // gpio_pull_up(ROTARY_ENC_CLK);

    gpio_init(ROTARY_ENC_DT);
    gpio_set_dir(ROTARY_ENC_DT, GPIO_IN);
    // gpio_pull_up(ROTARY_ENC_DT);

    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_pull_up(SDA);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SCL);
}

void init_display()
{
    i2c_init(i2c1, 400000);
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&disp);
}

void display_updates(u_int8_t freq)
{
}

void draw_splash()
{
    const char *words = "Avinash's FM RADIO!";
    ssd1306_draw_string(&disp, 0, 0, 1, words);
    ssd1306_show(&disp);
    sleep_ms(800);
    ssd1306_clear(&disp);
}