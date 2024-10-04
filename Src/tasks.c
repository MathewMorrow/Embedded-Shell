#include "tasks.h"
#include "main.h"
#include "shell.h"

static uint32_t ledToggleLastMillis = 0;
static uint32_t ledToggleMillis = 50;
static volatile float floatValue = 0;

// Program main loop task handler
void tasks()
{

    if (HAL_GetTick() - ledToggleLastMillis > ledToggleMillis) {
        ledToggleLastMillis = HAL_GetTick();
        // HAL_GPIO_TogglePin(PIN_LED_GPIO_Port, PIN_LED_Pin);
        (PIN_LED_GPIO_Port->ODR & (PIN_LED_Pin)) ? (PIN_LED_GPIO_Port->BRR = (PIN_LED_Pin)) : (PIN_LED_GPIO_Port->BSRR = (PIN_LED_Pin));
    }

    // Asynchronous handling of shell commands in non-blocking mode
    shell_task_handler();

}

// Proof of concept function for shell to adjust runtime settings
void set_led_period_milliseconds(uint32_t periodMilliseconds)
{
    ledToggleMillis = periodMilliseconds >> 2;
}