// External inclusions
#include "esp_system.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Definitions

#define MQTT_PROVISIONED BIT4

// Global variables

char ssid[32] = {0};
char pass[64] = {0};
bool has_wifi = 0;
EventGroupHandle_t sta_wifi_event_group;
EventGroupHandle_t mqtt_event_group;

// Function declarations

void init_mqtt(void);
void init_wifi_sta(void);
void init_storage(void);
void configure_leds(void);
void green_led_on(void);
void red_led_on(void);
void blue_led_on(void);
void publish_payload(char* payload_content, char* topic);
void configure_input_btn(void);

// void _task_send_data(void* pvParameters)
// {
//     while (1)
//     {
        
//     }
// }

void _task_process_data(void* pvParameters)
{

}

void app_main(void)
{
    configure_leds();
    configure_input_btn();
    green_led_on();
    red_led_on();
    blue_led_on();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_wifi_event_group = xEventGroupCreate();
    mqtt_event_group = xEventGroupCreate();
    
    init_storage();
    init_wifi_sta();
    init_mqtt();

    xEventGroupWaitBits(
        mqtt_event_group,
        MQTT_PROVISIONED,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    while (1)
    {
        /* code */
    }
    

    // xTaskCreate(_task_send_data, "send data", 1024, NULL, 10, NULL);
    // xTaskCreate(_task_process_data, "process data", 1024, NULL, 10, NULL);
}
