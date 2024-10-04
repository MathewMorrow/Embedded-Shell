#include "shell.h"
#include "tasks.h"

#include "stm32l4xx_hal.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// Commands
int cli_cmd_hello(int argc, char *argv[]);
int cli_configure_led(int argc, char *argv[]);
int cli_write_flash(int argc, char *argv[]);

// Array of all shell commands
// Made accessible via a const *const pointer via a global declared below
static const shellCommand_t s_shell_commands_array[] = {
    {"help", shell_help_handler, "Lists all commands"},
    {"hello", cli_cmd_hello, "Say hello"},
    {"led", cli_configure_led, "led config"},
    {"flash", cli_write_flash, "write to flash"},
};

// Create a const *const pointer to the shell commands array and number of elements of the array
const shellCommand_t *const g_shellCommandsArray = s_shell_commands_array;
const size_t g_numShellCommands = ARRAY_SIZE(s_shell_commands_array);

// Test command to say hello
int cli_cmd_hello(int argc, char *argv[])
{
    shell_put_line("Hello World!");
    for(int i = 0; i < argc; i++) {
        shell_put_line(argv[i]);
    }
    return 0;
}

// Set the LED toggle period
int cli_configure_led(int argc, char *argv[])
{
    // Return value
    int rslt = 1;

    for(int i = 0; i < argc; i++) {
        shell_put_line(argv[i]);
    }

    if(strcmp(argv[1], "set") == 0) {
        if(strcmp(argv[2], "period") == 0) {
            uint32_t commandPeriodMillis;
            commandPeriodMillis = (uint32_t)atoi(argv[3]);
            // If a valid number
            if(commandPeriodMillis) {
                set_led_period_milliseconds(commandPeriodMillis);
                char buffer[100];
                snprintf(buffer, sizeof(buffer), "LED period set to: ", argv[3]);
                shell_put_line(buffer);
                rslt = 0;
            } else {
                shell_put_line("Invalid period");
            }
        }
    }
    
    return rslt;
}

int cli_write_flash(int argc, char *argv[])
{
    int rslt = 1;

    // if(strcmp(argv[1], "write") == 0) {

    //     // int num_words = (32 + sizeof(argv[2]) - 1) / strlen(argv[2]);

    //     uint32_t data[] = {'h','e','l','l','o'};
    //     // Calculate the number of 32-bit words using sizeof
    //     size_t size_in_bytes = sizeof(data);  // Includes null terminator
    //     uint16_t num_words = (size_in_bytes + 3) / 4;  // Rounds up to the nearest 32-bit word
    //     uint32_t sector = 0x080FF800;
    //     uint32_t dataToSend = sector;
    //     // num_words = 1;
    //     flash_write_data(sector, &data, num_words);
    // }

    return rslt;
}