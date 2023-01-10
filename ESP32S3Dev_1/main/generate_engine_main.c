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

#define GPIO_OUTPUT_IO_0    1
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0
#define DUTY_NUM 16383

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

    ledc_set_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel, (uint32_t)(DUTY_NUM * 1.3 / 20));
    esp_err_t updateDutyCond180 = ledc_update_duty(LEDC_LOW_SPEED_MODE, engineChannel.channel);
    if (updateDutyCond180 == ESP_ERR_INVALID_ARG)
    {
        printf("Update Duty Error: Parameter error\n");
        return;
    }
    
    while (1)
    {
        engine(& engineChannel);
    }
    
}

