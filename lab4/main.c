#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "uart.h"

#define STRLEN 80 // Maximum length for the response string

// UART configuration settings
#define UART_NR 1           // Using UART1
#define UART_TX_PIN 4       // Pin 4 is configured as UART TX
#define UART_RX_PIN 5       // Pin 5 is configured as UART RX
#define BAUD_RATE 9600      // UART communication speed set to 9600 baud

// Function to send an AT command to the LoRa module and wait for a response
int send_command(const char *command, char *response_buffer, int maxlen, int max_attempts) {
    int attempt = 0;         // Tracks the number of attempts made
    int response_len = 0;    // Length of the response received

    // Loop to retry sending the command up to max_attempts
    while (attempt < max_attempts) {
        uart_send(UART_NR, command);  // Send the command via UART
        uint32_t start_time = time_us_32();
        response_len = 0;

        // Wait for data to become readable and process the response
        while ((time_us_32() - start_time) <= 500000) { // 500 ms timeout
            char c;
            if (uart_read(UART_NR, (uint8_t *)&c, 1) == 1) { // Read 1 character
                if (response_len < maxlen - 1) { // Buffer limit is not exceeded
                    response_buffer[response_len++] = c; // Store character in the response buffer
                    if (c == '\n') { // Stop reading if a line feed is found
                        response_buffer[response_len] = '\0'; // Null-terminate the response
                        return 1; // Valid response received
                    }
                }
            }
        }

        attempt++; // Increment the attempt counter if no response is received
    }

    return 0; // Failure: No response after max_attempts
}

// Function to process the DevEui response (received from the LoRa module)
void format_deveui(const char *devEui) {
    char processedDevEui[60]; // Buffer to store the processed DevEui

    // Iterate through the input string, skipping colons
    for (int i = 0, j = 0; i < 59; ++i) {
        if (devEui[i] != ':') { // Skip colon characters
            processedDevEui[j++] = tolower(devEui[i]); // Convert to lowercase and store
        }
    }
    processedDevEui[59] = '\0'; // Null-terminate the processed DevEui

    printf("DevEui: %s\n", processedDevEui); // Print the processed DevEui
}

// Main function implementing the state machine
int main() {
    const uint led_gpio = 22;   // GPIO pin for LED (not actively used here)
    const uint button_gpio = 7; // GPIO pin for the button (SW_0)

    int current_state = 0;      // Variable to track the current state of the program

    // Initialize the LED pin as output
    gpio_init(led_gpio);
    gpio_set_dir(led_gpio, GPIO_OUT);

    // Initialize the button pin as input with a pull-up resistor
    gpio_init(button_gpio);
    gpio_set_dir(button_gpio, GPIO_IN);
    gpio_pull_up(button_gpio);

    // Initialize UART and standard input/output
    stdio_init_all();
    uart_setup(UART_NR, UART_TX_PIN, UART_RX_PIN, BAUD_RATE);

    printf("Boot\n"); // Print a message to indicate the program has started

    char response_buffer[STRLEN]; // Buffer to hold UART responses

    // Infinite loop for the state machine
    while (true) {
        switch (current_state) {
            case 0: // Waiting for the user to press SW_0
                while (gpio_get(button_gpio)) { // Poll the button state
                    sleep_ms(10); // Debounce delay
                }
                current_state = 1; // Transition to State 1 ensures program only runs when the user is ready.
                break;

            case 1: // Send "AT" command to check connectivity
                if (send_command("AT\r\n", response_buffer, STRLEN, 5)) { // Try sending the command
                    printf("--- connecting ---\n");
                    printf("Connected to LoRa module\n"); // Success message
                    current_state = 2; // Move to next state
                } else { // If no response after 5 attempts
                    printf("Module not responding\n");
                    current_state = 0; // Return to initial state
                }
                break;

            case 2: // Send "AT+VER" command to get firmware version
                if (send_command("AT+VER\r\n", response_buffer, STRLEN, 5)) { // Try sending the command
                    printf("Firmware Version: %s\n", response_buffer); // Print firmware version
                    current_state = 3; // Move to next state
                } else { // If no response after 5 attempts
                    printf("Module stopped responding\n");
                    current_state = 0; // Return to initial state
                }
                break;

            case 3: // Send "AT+ID=DEVEUI" command to get DevEui
                if (send_command("AT+ID=DEVEUI\r\n", response_buffer, STRLEN, 5)) { // Try sending the command
                    format_deveui(response_buffer); // Process and print the DevEui
                    current_state = 0; // Return to initial state
                } else { // If no response after 5 attempts
                    printf("Module stopped responding\n");
                    current_state = 0; // Return to initial state
                }
                break;
        }
    }
}
