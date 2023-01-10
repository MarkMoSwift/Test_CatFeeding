/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <sys/param.h>

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/adc_types.h"
#include "esp_err.h"


#define GPIO_OUTPUT_IO_0    1
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0
#define DUTY_NUM 16383

ledc_channel_config_t *engineChannel_ = NULL;

static const char *TAG = "MQTT_EXAMPLE";
int countNum(int a);
void engine(ledc_channel_config_t* engineChannel);
void engineOC(ledc_channel_config_t* engineChannel)

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */



//事件处理函数
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        engineOC(engineChannel_);
        
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://192.168.4.1", 
        //.uri = CONFIG_BROKER_URL,
        //.client_id="mqttx_6011fa14",
        .username="espressDev",
        .password="asdfghjkl",
        .port=1883,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    //注册MQTT事件
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    //开启MQTT功能
    esp_mqtt_client_start(client);
}

int countNum(int a)
{
    int cnt = -1;
    while (a != 0)
    {
        a = a >> 1;
        cnt ++;
    }
    return cnt;
}

void engine(ledc_channel_config_t* engineChannel)
{
    
    static uint32_t flag = 0;
    
    int highTime = 2.2;

    if (flag == 0)
    {
        highTime = 2.2;
        printf("Engine turns 0 degrees\n");
        printf("Close the door!\n");
    }
    else if (flag == 1)
    {
        highTime = 1.3;
        printf("Engine turns 90 degrees\n");
        printf("Open the door!\n");
    }

    ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel, (uint32_t)(DUTY_NUM * highTime / 20));
    esp_err_t updateDutyCond = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel);
    /* printf("duty: ");
    printf( ledc_get_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel) );
    printf(' \n'); */
    if (updateDutyCond == ESP_ERR_INVALID_ARG)
    {
        printf("Update Duty Error: Parameter error\n");
        return;
    }

    vTaskDelay(500 / portTICK_RATE_MS);

    //flag = (flag ? 0 : 1);
        
}

void engineOC(ledc_channel_config_t* engineChannel)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel, (uint32_t)(DUTY_NUM * 2.2 / 20));
    esp_err_t updateDutyCond = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel);
    if (updateDutyCond == ESP_ERR_INVALID_ARG)
    {
        printf("Update Duty Error: Parameter error\n");
        return;
    }

    vTaskDelay(1000 / portTICK_RATE_MS);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel, (uint32_t)(DUTY_NUM * 1.3 / 20));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel);

    //flag = (flag ? 0 : 1);
        
}


void app_main(void)
{
    /**************************************************************/
    
    ledc_timer_config_t engineTimer1;
    //speed; 低速模式
    engineTimer1.speed_mode = LEDC_LOW_SPEED_MODE;
    //LEDC channel duty resolution 12; 占空比12位有效数字，即0-4095
    engineTimer1.duty_resolution = countNum(DUTY_NUM + 1);
    //The timer source of channel 0;
    engineTimer1.timer_num = 0;
    //frequency 50 Hz
    engineTimer1.freq_hz = 50;
    //Configure LEDC source clock;
    engineTimer1.clk_cfg = LEDC_AUTO_CLK;
    
    //
    esp_err_t timerConfigCond = ledc_timer_config(&engineTimer1);
    if (timerConfigCond == ESP_ERR_INVALID_ARG)
    {
        printf("Timer Error: Parameter error\n");
        return;
    }
    else if (timerConfigCond == ESP_FAIL)
    {
        printf("Timer Error: Can not find a proper pre-divider number base on the given frequency and the current duty_resolution.\n");
        return;
    }
    
    ledc_channel_config_t engineChannel;
    //output gpio_num;
    engineChannel.gpio_num = GPIO_OUTPUT_IO_0;
    //speed; 低速模式
    engineChannel.speed_mode = LEDC_LOW_SPEED_MODE;
    //LEDC channel;
    engineChannel.channel = 0;
    //interrupt disable; 
    engineChannel.intr_type = LEDC_INTR_DISABLE;
    //timer source of channel
    engineChannel.timer_sel = 0;
    //duty setting; 占空比
    engineChannel.duty = (uint32_t)(DUTY_NUM * 0.5 / 20);
    //LEDC channel hpoint value; 不懂
    engineChannel.hpoint = 1;
    //LEDC flags; 不懂
    engineChannel.flags.output_invert = 0;

    engineChannel_ = &engineChannel;


    esp_err_t channelConfigCond = ledc_channel_config(&engineChannel);
    if (channelConfigCond == ESP_ERR_INVALID_ARG)
    {
        printf("Channel Error: Parameter error\n");
        return;
    }
    /****************************************************************************/

    //
    vTaskDelay(1000 / portTICK_RATE_MS);
    esp_err_t setDutyCond = ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel, (uint32_t)(DUTY_NUM * 1.5 / 20));
    if (setDutyCond == ESP_ERR_INVALID_ARG)
    {
        printf("Task Delay Error: Parameter error\n");
        return;
    }

    esp_err_t updateDutyCond = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel);
    if (updateDutyCond == ESP_ERR_INVALID_ARG)
    {
        printf("Update Duty Error: Parameter error\n");
        return;
    }

    ledc_bind_channel_timer(LEDC_LOW_SPEED_MODE, 0, 0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel, (uint32_t)(DUTY_NUM * 2.2 / 20));
    esp_err_t updateDutyCond180 = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel);
    if (updateDutyCond180 == ESP_ERR_INVALID_ARG)
    {
        printf("Update Duty Error: Parameter error\n");
        return;
    }

    /**************************************************************************/


    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();

  /*  while(1)
        {
        //printf ("I am gear long\r\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        }*/

    
    


}
