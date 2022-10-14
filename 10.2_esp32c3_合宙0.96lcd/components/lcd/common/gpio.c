#include "gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"

#define TAG     "gpio"

// #define GPIO_OUT_DATA	(*(volatile unsigned int*)(GPIO_OUT_REG))

// static uint8_t uart_isr_sta = 0;

// extern xQueueHandle gpio_evt_queue;
// static void IRAM_ATTR gpio_isr_handler(void *arg)
// {
//     uint32_t gpio_num = (uint32_t)arg;
//     xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
// }

void gpio_mode(int pin, int mode, int pull, int initOutput) {
    if (pin == 255) return;
    gpio_t conf = {0};
    conf.pin = pin;
    conf.mode = mode == GPIO_INPUT ? GPIO_INPUT : GPIO_OUTPUT; // 只能是输入/输出, 不能是中断.
    conf.pull = pull;
    conf.irq = initOutput;
    conf.lua_ref = 0;
    conf.irq_cb = 0;
    gpio_setup(&conf);
    if (conf.mode == GPIO_OUTPUT)
        gpio_set(pin, initOutput);
}

int gpio_exist(int pin)
{
    if (pin >= 0 && pin < GPIO_NUM_MAX)
        return 1;
    else
        return 0;
}

int gpio_setup(gpio_t *gpio)
{
    ESP_LOGI(TAG, "gpio_setup pin=%d, mode=%d", gpio->pin, gpio->mode);
    if (!gpio_exist(gpio->pin))
        return -1;

    // 先去初始化gpio(esp32c3的io18和19必须做这一步)
    gpio_reset_pin(gpio->pin);

    //设置输入输出模式
    if (gpio->mode == GPIO_OUTPUT)
    {
        gpio_set_direction(gpio->pin, GPIO_MODE_OUTPUT);
    }
    else if (gpio->mode == GPIO_INPUT)
    {
        gpio_set_direction(gpio->pin, GPIO_MODE_INPUT);
    }
    else if (gpio->mode == GPIO_IRQ)
    {
        gpio_set_direction(gpio->pin, GPIO_MODE_INPUT);
        // //设置中断
        switch (gpio->irq)
        {
        case GPIO_RISING:
            gpio_set_intr_type(gpio->pin, GPIO_INTR_POSEDGE);
            break;
        case GPIO_FALLING:
            gpio_set_intr_type(gpio->pin, GPIO_INTR_NEGEDGE);
            break;
        case GPIO_BOTH:
        default:
            gpio_set_intr_type(gpio->pin, GPIO_INTR_ANYEDGE);
            break;
        }
        // if (uart_isr_sta == 0)
        // {
        //     gpio_install_isr_service(0);
        //     uart_isr_sta = 1;
        // }
        // gpio_isr_handler_add(gpio->pin, gpio_isr_handler, (void *)gpio->pin);
    }

    //设置上下拉
    switch (gpio->pull)
    {
    case GPIO_DEFAULT:
        gpio_set_pull_mode(gpio->pin, GPIO_FLOATING);
        break;
    case GPIO_PULLUP:
        gpio_set_pull_mode(gpio->pin, GPIO_PULLUP_ONLY);
        break;
    case GPIO_PULLDOWN:
        gpio_set_pull_mode(gpio->pin, GPIO_PULLDOWN_ONLY);
        break;
    default:
        break;
    }
    return 0;
}

int gpio_set(int pin, int level)
{
    if (gpio_exist(pin))
    {
        gpio_set_level(pin, level);
        return 0;
    }
    else
        return -1;
}

int gpio_get(int pin)
{
    if (gpio_exist(pin))
    {
        int level = gpio_get_level(pin);
        return level;
    }
    return -1;
}

void gpio_close(int pin)
{
    if (gpio_exist(pin))
    {
        gpio_reset_pin(pin);
    }
}

// void IRAM_ATTR gpio_pulse(int pin, uint8_t *level, uint16_t len, uint16_t delay_ns) {
// #if CONFIG_IDF_TARGET_ESP32C3
//     volatile uint32_t del=delay_ns;
//     vPortEnterCritical();
//     for(int i=0; i<len; i++){
//         if(level[i/8]&(0x80>>(i%8)))
//             GPIO_OUT_DATA |= (0x00000001<<pin);
//         else 
//             GPIO_OUT_DATA &= ~(0x00000001<<pin);
//         del = delay_ns;
//         while(del--);
//     }
//     vPortExitCritical();
// #endif
// }
