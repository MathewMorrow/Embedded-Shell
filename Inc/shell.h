#pragma once

#include <stddef.h>

// HOW TO USE THE EMBEDDED SHELL COMMAND LINE
// 1. Setup the Shell:
//    Use the 'shellImpl_t' structure to configure the shell and pass to the shell_init function at boot
//
//    a. **Provide a Pointer to the Transmit Function:**
//       You need to pass a function that transmits a single character over the UART or other communication interface.
//
//    b. **Provide a Pointer to the Transmit-Ready Check Function:**
//       This function should return whether the transmit hardware (UART, etc.) is ready to send more data.
//
// 2. **Receiving Data:**
//    Place the 'shell_receive_char(char c)' function inside your receive interrupt handler to pass incoming characters 
//    directly to the shell for processing.
//    
//    Example for use in Rx IRQ handler:
//    void USARTx_IRQHandler(void) {
//        if (rx interrupt condition) {
//            char c = receive_character_from_uart();
//            shell_receive_char(c);
//        }
//    }
//
// 3. **Non-Blocking Transmission:**
//    Ensure that the `SHELL_NON_BLOCKING` is defined in the source file
//    Place the shell_task_handler() to be run periodically to handle shell commands asynchronously

// Definition for each shell command
// {command, handler function, help string}
typedef struct shellCommand_s {
    const char *command;
    int (*handler)(int argc, char *argv[]);
    const char *help;
} shellCommand_t;

// Implimentation structure for the device/hardware
typedef struct shellImpl_s {
  // Function to call whenever a character needs to be sent out.
  int (*send_char)(char c);
  // Function to check if the Tx hardware is ready to send
  int (*is_tx_hardware_ready)(void);
} shellImpl_t;

// Reference for the internal shell commands - defined as an array
extern const shellCommand_t *const g_shellCommandsArray;
// Reference to the number of shell commands registered
extern const size_t g_numShellCommands;

// Initializes the shell with the device implimentation - called during boot
void shell_init(const shellImpl_t *impl);

// Call this when a character is received.
// Place in the Rx IRQ for non-blocking/polling implimentation
void shell_receive_char(char c);

// Send Tx data if ready
// Place in the Tx IRQ handler for non-blocking implimentation
void prv_send_char_buf();

// If SHELL_NON_BLOCKING is define this function must be called periodically to handle data
void shell_task_handler();

// Write a string appended with a '\n'
void shell_put_line(const char *str);

// Help handler in case of input error
int shell_help_handler(int argc, char *argv[]);