// External inclusions
#include "esp_event.h"
#include "freertos/task.h"

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
void init_gpio(void);
void init_rtc(void);
void configure_input_btn(void);

void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_wifi_event_group = xEventGroupCreate();
    mqtt_event_group = xEventGroupCreate();
    
    init_storage();
    init_wifi_sta();
    init_rtc();
    init_mqtt();

    xEventGroupWaitBits(
        mqtt_event_group,
        MQTT_PROVISIONED,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    init_gpio();
}
