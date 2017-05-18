/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_alink.h"

#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "alink_factory_reset";
static xQueueHandle gpio_evt_queue = NULL;

void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void alink_key_init(uint32_t key_gpio_pin)
{
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1 << key_gpio_pin;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(key_gpio_pin, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(2, sizeof(uint32_t));

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(key_gpio_pin, gpio_isr_handler, (void*) key_gpio_pin);
}

typedef enum {
    ALINK_KEY_SHORT_PRESS = 1,
    ALINK_KEY_MEDIUM_PRESS,
    ALINK_KEY_LONG_PRESS,
} alink_key_t;

alink_err_t alink_key_scan(TickType_t ticks_to_wait)
{
    uint32_t io_num;
    alink_err_t ret;
    BaseType_t press_key = pdFALSE;
    BaseType_t lift_key = pdFALSE;

    int backup_time = 0;
    for (;;) {
        ret = xQueueReceive(gpio_evt_queue, &io_num, ticks_to_wait);
        ALINK_ERROR_CHECK(ret != pdTRUE, ALINK_ERR, "xQueueReceive, ret:%d", ret);

        if (gpio_get_level(io_num) == 0) {
            press_key = pdTRUE;
            backup_time = system_get_time();
        } else if (press_key) {
            lift_key = pdTRUE;
            backup_time = system_get_time() - backup_time;
        }
        if (press_key & lift_key) {
            press_key = pdFALSE;
            lift_key = pdFALSE;
            if (backup_time > 5000000) {
                    return ALINK_KEY_LONG_PRESS;
            } else if (backup_time > 1000000) {
                return ALINK_KEY_MEDIUM_PRESS;
            } else {
                return ALINK_KEY_SHORT_PRESS;
            }
        }
    }
}

void alink_key_event(void* arg)
{
    alink_err_t ret = 0;
    alink_key_init(ALINK_RESET_KEY_IO);
    for (;;) {
        ret = alink_key_scan(portMAX_DELAY);
        ALINK_ERROR_CHECK(ret == ALINK_ERR, vTaskDelete(NULL), "alink_key_scan ret:%d", ret);

        switch (ret) {
        case ALINK_KEY_SHORT_PRESS:
            alink_event_send(ALINK_EVENT_ACTIVATE_DEVICE);
            break;

        case ALINK_KEY_MEDIUM_PRESS:
            alink_event_send(ALINK_EVENT_UPDATE_ROUTER);
            break;

        case ALINK_KEY_LONG_PRESS:
            alink_event_send(ALINK_EVENT_FACTORY_RESET);
            break;

        default:
            break;
        }
    }

    vTaskDelete(NULL);
}

alink_err_t alink_erase_wifi_config();
alink_err_t esp_alink_factory_reset()
{
    /* clear ota data  */
    ALINK_LOGI("*********************************");
    ALINK_LOGI("*          FACTORY RESET        *");
    ALINK_LOGI("*********************************");
    ALINK_LOGI("clear wifi config");
    alink_err_t err;
    err = alink_erase_wifi_config();
    ALINK_ERROR_CHECK(err != 0, ALINK_ERR, "alink_erase_wifi_config");

    esp_partition_t find_partition;
    memset(&find_partition, 0, sizeof(esp_partition_t));
    find_partition.type = ESP_PARTITION_TYPE_DATA;
    find_partition.subtype = ESP_PARTITION_SUBTYPE_DATA_OTA;

    const esp_partition_t *partition = esp_partition_find_first(find_partition.type, find_partition.subtype, NULL);
    ALINK_ERROR_CHECK(partition == NULL, ALINK_ERR, "nvs_erase_key partition:%p", partition);

    err = esp_partition_erase_range(partition, 0, partition->size);
    ALINK_ERROR_CHECK(partition == NULL, ALINK_ERR, "nvs_erase_key partition:%p", partition);

    if (err != ALINK_OK) {
        ALINK_LOGE("esp_partition_erase_range ret:%d", err);
        vTaskDelete(NULL);
    }
    ALINK_LOGI("reset user account binding");
    alink_factory_reset();

    ALINK_LOGI("The system is about to be restarted");
    esp_restart();
}
