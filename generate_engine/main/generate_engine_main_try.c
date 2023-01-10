/* Generate Engine


*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/adc_types.h"
#include "esp_err.h"

/**
 * Brief:

 */

#define GPIO_OUTPUT_IO_0    1
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0
#define DUTY_NUM 16383

void deadCircle()
{
    while (1){}
    
}

void sleepOn(ledc_channel_config_t* engineChannel)
{
    printf("engine 180 degrees\n");
    ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel, (uint32_t)(DUTY_NUM));
    esp_err_t updateDutyCond180 = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel->channel);
    /* printf("duty: ");
    printf( ledc_get_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel) );
    printf(' \n'); */
    if (updateDutyCond180 == ESP_ERR_INVALID_ARG)
    {
        printf("Update Duty Error: Parameter error\n");
        return;
    }

    deadCircle();

    vTaskDelay(3000 / portTICK_RATE_MS);
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

void app_main_try(void)
{
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


    esp_err_t channelConfigCond = ledc_channel_config(&engineChannel);
    if (channelConfigCond == ESP_ERR_INVALID_ARG)
    {
        printf("Channel Error: Parameter error\n");
        return;
    }

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

    ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel, (uint32_t)(DUTY_NUM * 1.3 / 20));
    esp_err_t updateDutyCond180 = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel);
    if (updateDutyCond180 == ESP_ERR_INVALID_ARG)
    {
        printf("Update Duty Error: Parameter error\n");
        return;
    }

    //deadCircle();

    /* while (1)
    {
        sleepOn(&engineChannel);
    } */
    
    while (1)
    {
        vTaskDelay(4000 / portTICK_RATE_MS);

        printf("engine 180 degrees\n");
        ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel, (uint32_t)(DUTY_NUM * 2.2 / 20));
        esp_err_t updateDutyCond180 = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel);
        /* printf("duty: ");
        printf( ledc_get_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel) );
        printf(' \n'); */
        if (updateDutyCond180 == ESP_ERR_INVALID_ARG)
        {
            printf("Update Duty Error: Parameter error\n");
            return;
        }

        vTaskDelay(1000 / portTICK_RATE_MS);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel, (uint32_t)(DUTY_NUM * 0 / 20));
        ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel);

        vTaskDelay(4000 / portTICK_RATE_MS);

        printf("engine 90 degrees\n");
        ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel, (uint32_t)(DUTY_NUM * 1.3 / 20));
        esp_err_t updateDutyCond90 = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel);
        /*printf("duty: ");
        printf( ledc_get_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel) );
        printf(' \n'); */
        if (updateDutyCond90 == ESP_ERR_INVALID_ARG)
        {
            printf("Update Duty Error: Parameter error\n");
            return;
        }

        
    }
    
}

/* void app_main(void)
{
    //zero-initialize the config structure. 初始化 gpio_config_t 类型的实例
    gpio_config_t io_conf = {};
    //disable interrupt 不懂
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode 设置成输出引脚
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set 不懂，应该是1 << 1 吧？
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //enable pull-down mode 设置为下拉
    io_conf.pull_down_en = 1;
    //disable pull-up mode 不设置为上拉
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings 
    esp_err_t gpio1SetConf = gpio_config(&io_conf);
    if (gpio1SetConf == ESP_ERR_INVALID_ARG)
    {
        printf("error: gpio1 setting error");
        return 1;
    }
    

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);

    //remove isr handler for gpio number.
    gpio_isr_handler_remove(GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin again
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    int cnt = 0;
    while(1) {
        printf("cnt: %d\n", cnt++);
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_set_level(GPIO_OUTPUT_IO_0, 1);
        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
    }
} */
