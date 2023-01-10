# Example: Generate Engine

这个程序的原版是 D:\Users\86177\source\ESP32Works\examples\peripherals\gpio\generic_gpio
原程序的意义不完全明确。

## function
列举出几个能控制电机的函数

| GPIO     | Direction | Configuration                                          |
| -------- | --------- | ------------------------------------------------------ |
| GPIO1    | output    | 50Hz信号 高电平部分脉宽不同                              |

| 高电平脉宽 / ms | Configuration |
| -------------- | ------------- |
| 0.5            | 0 degree      |
| 1.0            | 45 degree     |
| 1.5            | 90 degree     |
| 2.0            | 135 degree    |
| 2.5            | 180 degree    |



