#include "shell.h"

#include <stddef.h>
#include <string.h>
#include <string.h>
#include <stdint.h>

#define SHELL_NON_BLOCKING
#define SHELL_DEBUG
#define IS_BACKSPACE_OR_DELETE(c) ((c) == '\b' || (c) == 127)
#define SHELL_MAX_BUFFER_SIZE (256)
#define SHELL_MAX_ARGUMENTS (16)
#define SHELL_PROMPT "shell> "

// Macro used for looping through all shell commands until the "command" gets a match
#define SHELL_LOOP_FOR_COMMAND(command) \
    for (const shellCommand_t *command = g_shellCommandsArray; \
        command < &g_shellCommandsArray[g_numShellCommands]; \
        ++command)

// Private function declerations
static void prv_shell_reset_tx_buffer();
static int prv_shell_is_initialized();
static const shellCommand_t *prv_find_command(const char *name);
static void prv_send_char(const char c);
static void prv_echo_str(const char *str);
static void prv_echo(char c);
static char prv_last_char(void);
static int prv_is_rx_buffer_full(void);
static void prv_reset_rx_buffer(void);
static void prv_send_shell_prompt(void);
static void prv_process(void);
static void prv_tx_buffer_error_handler(void);

// Structure for the shell to run within
static struct shellContext_s {
    // Recieve context
    size_t rxSize;
    char rxBuffer[SHELL_MAX_BUFFER_SIZE];
    // Transmit context
    int (*send_char)(char c);
    int (*is_tx_hardware_ready)(void);
    volatile uint8_t txBuffer[SHELL_MAX_BUFFER_SIZE];
    volatile uint16_t txHead;
    volatile uint16_t txTail;
    volatile uint16_t txCount;
} shell;

// Initialize the shell with the given *implimentation
void shell_init(const shellImpl_t *impl)
{
    shell.send_char = impl->send_char;
    shell.is_tx_hardware_ready = impl->is_tx_hardware_ready;
    prv_reset_rx_buffer();
    prv_echo_str("\n" SHELL_PROMPT);
}

// Reset the Tx buffer
void prv_shell_reset_tx_buffer()
{
    shell.txCount = 0;
    shell.txHead = 0;
    shell.txTail = 0;
}

// Check if the shell is initialized
static int prv_shell_is_initialized()
{
    return shell.send_char != NULL;
}

// Find the command of the registered command 
static const shellCommand_t *prv_find_command(const char *name)
{
    SHELL_LOOP_FOR_COMMAND(command) {
            if (strcmp(command->command, name) == 0) {
                return command;
            }
        }
     return NULL;
}

// Send a single char to the CLI
static void prv_send_char(const char c)
{
#if defined(SHELL_NON_BLOCKING)
    // Add data to the transmit buffer
    if (shell.txCount < SHELL_MAX_BUFFER_SIZE) {  // Check if buffer has space
        shell.txBuffer[shell.txHead] = c;
        shell.txHead = (shell.txHead + 1) % SHELL_MAX_BUFFER_SIZE;
        shell.txCount++;
    } else {
        // Buffer overflow, handle error (e.g., drop data or block until space is available)
        prv_tx_buffer_error_handler();
        return;
    }

    // Start the Tx process of buffered chars
    prv_send_char_buf();

#else
    // Send the char to the Tx function pointer
    s_shell.send_char(c);
#endif
}

// Send Tx data if ready
// Place inside the Tx interrupt handler
void prv_send_char_buf()
{
    // If UART is not already transmitting, start transmission
    if (shell.is_tx_hardware_ready() && shell.txCount > 0) {
        // Start sending the first char from the buffer
        volatile uint8_t firstChar = shell.txBuffer[shell.txTail];
        shell.txTail = (shell.txTail + 1) % SHELL_MAX_BUFFER_SIZE;
        shell.txCount--;

        // Send the char to the Tx function pointer
        shell.send_char(firstChar); // Start interrupt-driven transmission
    }
}

// Change name or replace with prv_write
static void prv_echo_str(const char *str)
{
    for (const char *c = str; *c != '\0'; ++c) {
        prv_echo(*c);
    }
}

// Write a string to the terminal followed by a new line '\n'
void shell_put_line(const char *str) {
  prv_echo_str(str);
  prv_echo('\n');
}

// Echo back to the terminal
static void prv_echo(char c)
{
    if ('\n' == c) {
        //prv_echo_str("\r\n"); // TODO - Figure out why this isn't working
        prv_send_char('\r');
        prv_send_char('\n');
    } else if (IS_BACKSPACE_OR_DELETE(c)) {
        //prv_echo_str("\b \b");
        prv_send_char('\b');
        prv_send_char(' ');
        prv_send_char('\b');
    } else {
        prv_send_char(c);
    }
}

// Check the most recent Rx char
static char prv_last_char(void)
{
    return shell.rxBuffer[shell.rxSize - 1];
}

// Check if the rx buffer is full
static int prv_is_rx_buffer_full(void)
{
    return shell.rxSize >= SHELL_MAX_BUFFER_SIZE;
}

// Clear the entire rx buffer
static void prv_reset_rx_buffer(void)
{
    memset(shell.rxBuffer, 0, sizeof(shell.rxBuffer));
    shell.rxSize = 0;
}

// Send the shell prompt for the start of new command
static void prv_send_shell_prompt(void)
{
    prv_echo_str(SHELL_PROMPT);
}

// Place inside the Rx interrupt handler; UART, SPI, USB etc.
void shell_receive_char(char c)
{
    // Handle buffer empty and receiving backspace
    if(shell.rxSize == 0 && IS_BACKSPACE_OR_DELETE(c)) {
        return;
    }

    // Ignore '\r' or return if shell is not booted or rxBuffer is full
    if (c == '\r' || prv_is_rx_buffer_full() || !prv_shell_is_initialized()) {
        return;
    }

    // Valid char is received so echo it it back to the terminal
    prv_echo(c);

    // Handle backspace char
    if (IS_BACKSPACE_OR_DELETE(c)) {
        shell.rxBuffer[--shell.rxSize] = '\0';
        return;
    }

    shell.rxBuffer[shell.rxSize++] = c;

#ifndef SHELL_NON_BLOCKING
    prv_process();
#endif
}

static void prv_process(void)
{
    // Return unless a '\n' or Rx buffer is not full
    if (prv_last_char() != '\n' && !prv_is_rx_buffer_full()) {
        return; 
    }

    // Temporary variable to hold parsed command args and count
    char *argv[SHELL_MAX_ARGUMENTS] = {0};
    int argc = 0;

    // Loop through the received command and parse out arguments
    char *next_arg = NULL;
    for (size_t i = 0; i < shell.rxSize && argc < SHELL_MAX_ARGUMENTS; ++i) {
        char *const c = &shell.rxBuffer[i];
        if (*c == ' ' || *c == '\n' || i == shell.rxSize - 1) {
            *c = '\0';
            if (next_arg) {
                argv[argc++] = next_arg;
                next_arg = NULL;
            }
        } else if (!next_arg) {
            next_arg = c;
        }
    }

    // Put a new line if the buffer is maxed out
    if (shell.rxSize >= SHELL_MAX_BUFFER_SIZE) {
        prv_echo('\n');
    }

    // If a valid argument is found, search for it and handle it
    if (argc >= 1) {
        const shellCommand_t *command = prv_find_command(argv[0]);
        if (!command) {
            prv_echo_str("Unknown command: ");
            prv_echo_str(argv[0]);
            prv_echo('\n');
            prv_echo_str("Type 'help' to list all commands\n");
        } else {
            command->handler(argc, argv);
        }
        prv_echo('\n');
    }

    // Clear the receive buffer and send the shell prompt
    prv_reset_rx_buffer();
    prv_send_shell_prompt();
}

int shell_help_handler(int argc, char *argv[]) {
    SHELL_LOOP_FOR_COMMAND(command) {
        prv_echo_str(command->command);
        prv_echo_str(": ");
        prv_echo_str(command->help);
        prv_echo('\n');
    }
    return 0;
}

// If SHELL_NON_BLOCKING is define this function must be called periodically to handle data
void shell_task_handler()
{
    // Return if shell does not have any data to process
    if (shell.rxSize == 0 || !prv_shell_is_initialized()) {
        return;
    }
    // Process each buffered Rx char
    for(size_t i = 0; i < shell.rxSize; i++) {
        prv_process();
    }
}

// Error handler
static void prv_tx_buffer_error_handler(void)
{
    prv_reset_rx_buffer();
    shell_put_line("Error: buffer overflow");
}