#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// #define ESP32C6

#ifdef ESP32C6
    #define GREEN_LED GPIO_NUM_12
    #define BLUE_LED  GPIO_NUM_10
    #define RED_LED   GPIO_NUM_1
#else
    #define GREEN_LED GPIO_NUM_12
    #define BLUE_LED  GPIO_NUM_27
    #define RED_LED   GPIO_NUM_26
#endif

void configure_leds(void)
{
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << GREEN_LED),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << BLUE_LED);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << RED_LED);
    gpio_config(&io_conf);
}

void configure_input_btn(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_13),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    while (1)
    {
        ESP_LOGI("GPIO", "%d", gpio_get_level(GPIO_NUM_13));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void green_led_on(void)
{
    gpio_set_level(GREEN_LED, 1);
}

void green_led_off(void)
{
    gpio_set_level(GREEN_LED, 0);
}

void blue_led_on(void)
{
    gpio_set_level(BLUE_LED, 1);
}

void blue_led_off(void)
{
    gpio_set_level(BLUE_LED, 0);    
}

void red_led_on(void)
{
    gpio_set_level(RED_LED, 1);    
}

void red_led_off(void)
{
    gpio_set_level(RED_LED, 0);    
}