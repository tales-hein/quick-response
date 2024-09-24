#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/projdefs.h"
#include "hal/adc_types.h"
#include <time.h>

#define TAG       "GPIO"
#define GREEN_LED GPIO_NUM_12
#define BLUE_LED  GPIO_NUM_10
#define RED_LED   GPIO_NUM_1

static adc_oneshot_unit_handle_t adc1_handle;
static int adc_raw[2];
static int voltage[2];
static adc_oneshot_unit_init_cfg_t init_config1 = {.unit_id = ADC_UNIT_1};
static bool do_calibration1_chan0               = false;
static bool do_calibration1_chan1               = false;
static adc_cali_handle_t adc1_cali_chan0_handle = NULL;
static adc_cali_handle_t adc1_cali_chan1_handle = NULL;

extern QueueHandle_t send_queue;
BaseType_t sent;

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

static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_13,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

void configure_analog_pin(void)
{
    adc_oneshot_new_unit(&init_config1, &adc1_handle);
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_13,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_5, &config);
    do_calibration1_chan0 = example_adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_4, ADC_ATTEN_DB_12, &adc1_cali_chan0_handle);
    do_calibration1_chan1 = example_adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_5, ADC_ATTEN_DB_12, &adc1_cali_chan1_handle);
}

void read_analog_callback(void *arg)
{
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &adc_raw[0]));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC_CHANNEL_4, adc_raw[0]);
    if (do_calibration1_chan0) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0], &voltage[0]));
        ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC_CHANNEL_4, voltage[0]);
    }

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_5, &adc_raw[1]));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC_CHANNEL_5, adc_raw[1]);
    if (do_calibration1_chan1) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan1_handle, adc_raw[1], &voltage[1]));
        ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC_CHANNEL_5, voltage[1]);
    }

    time_t now;
    time(&now);
    char payload[128];
    
    snprintf(
        payload,
        sizeof(payload),
        "{\"read_1\": %i, \"read_2\": %i, \"timestamp\": %lld}",
        voltage[0], voltage[1], now
    );

    do {
        sent = xQueueSend(send_queue, payload, 0);
    } while (sent == errQUEUE_FULL);
}

void init_gpio(void)
{
    configure_leds();
    configure_analog_pin();
    green_led_on();
    red_led_on();
    blue_led_on();
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &read_analog_callback,
        .name = "read_analog_callback"      
    };
    esp_timer_handle_t periodic_timer;
    esp_timer_create(&periodic_timer_args, &periodic_timer);
    esp_timer_start_periodic(periodic_timer, 100);
    ESP_LOGI(TAG, "GPIO initialized.");
}
