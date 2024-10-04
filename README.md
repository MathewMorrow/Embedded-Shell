This is a simple lightweight shell for embedded targets. It is simple to use and can opperate in blocking or non-blocking (asynchronous mode). It only requires a function pointer to transmit a char and to check if the transmit hardware is ready.  
New commands are added to the shell_commands.c source file.  

HOW TO USE THE EMBEDDED SHELL COMMAND LINE
1. **Setup the Shell:**
   Use the 'shellImpl_t' structure to configure the shell and pass to the shell_init function at boot

   a. **Provide a Pointer to the Transmit Function:**
      You need to pass a function that transmits a single character over the UART or other communication interface.

   b. **Provide a Pointer to the Transmit-Ready Check Function:**
      This function should return whether the transmit hardware (UART, etc.) is ready to send more data.

2. **Receiving Data:**
   Place the 'shell_receive_char(char c)' function inside your receive interrupt handler to pass incoming characters 
   directly to the shell for processing.
   
   Example for use in Rx IRQ handler:  
   void USARTx_IRQHandler(void) {  
       if (rx interrupt condition) {  
           char c = receive_character_from_uart();  
           shell_receive_char(c);  
       }  
   }  

3. **Non-Blocking Transmission:**
   Ensure that the `SHELL_NON_BLOCKING` is defined in the source file
   Place the shell_task_handler() to be run periodically to handle shell commands asynchronously


4. **Register New Commands In The shell_commands.c Source File:**  
   Add any new command into the shellCommand_t s_shell_commands_array[]  
   Create the function decleration and definition for the new command if needed  
   Add external dependencies to the shell_commands.c as needed  
