#include "esp_sntp.h"
#include "esp_log.h"
#include <time.h>

#define TAG "RTC_UPDATE"
#define NTP_URL "a.st1.ntp.br"

void update_time(void)
{
    struct tm timeinfo;
    time_t now;
    struct timeval tv;
    ESP_LOGI(TAG, "Updating time from NTP server...");
    sntp_sync_time(&tv);
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "Time synchronized: %s", asctime(&timeinfo));
}

void update_time_task(void *pvParameter)
{
    while (1) 
    {
        update_time();
        vTaskDelay(3600000 / portTICK_PERIOD_MS);
    }
}

void init_rtc(void) 
{
    ESP_LOGI(TAG, "Setting up RTC NTP server...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, NTP_URL);
    esp_sntp_init();
    time_t now = 0;
    struct tm timeinfo = { 0 };
    while (timeinfo.tm_year < (1970 - 1900))
    {
        ESP_LOGI(TAG, "Waiting for system time to be set...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    ESP_LOGI(TAG, "First time synchronization: %s", asctime(&timeinfo));
    xTaskCreate(&update_time_task, "update_time_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Time update task started");
}
