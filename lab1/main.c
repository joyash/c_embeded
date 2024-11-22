
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Define pins
#define SW0 7     // Button to increase brightness
#define SW1 8     // Button to toggle LED on/off
#define SW2 9     // Button to decrease brightness
#define LED1 20   // First LED pin
#define LED2 21   // Second LED pin
#define LED3 22   // Third LED pin

int brightness = 500;      // Initial brightness (50% of max)
bool led_on = false;       // Initial LED state is off

// State variables for button debouncing
bool sw1_released = true;
//bool sw0_released = true;
//bool sw2_released = true;

// Variables to control holding behavior
bool sw0_holding = false;
bool sw2_holding = false;

// Function to configure PWM on a specified pin
void setup_pwm_pin(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);          // Set the pin function to PWM
    uint slice_num = pwm_gpio_to_slice_num(pin);    // Get the PWM slice for this pin
    pwm_set_clkdiv(slice_num, 125.0f);              // Set the clock divider for 1 MHz
    pwm_set_wrap(slice_num, 1000);                  // Set the wrap value for 1 kHz frequency
    pwm_set_enabled(slice_num, true);               // Enable PWM on this slice
}

// Function to set brightness on all three LEDs
void update_brightness(int level) {
    pwm_set_gpio_level(LED1, level);  // Set the PWM level for LED1
    pwm_set_gpio_level(LED2, level);  // Set the PWM level for LED2
    pwm_set_gpio_level(LED3, level);  // Set the PWM level for LED3
}

void setup() {
    stdio_init_all();

    // Initialize button pins as input with pull-up resistors
    gpio_init(SW0);
    gpio_set_dir(SW0, GPIO_IN);
    gpio_pull_up(SW0);

    gpio_init(SW1);
    gpio_set_dir(SW1, GPIO_IN);
    gpio_pull_up(SW1);

    gpio_init(SW2);
    gpio_set_dir(SW2, GPIO_IN);
    gpio_pull_up(SW2);

    // Initialize all three LEDs as PWM outputs
    setup_pwm_pin(LED1);
    setup_pwm_pin(LED2);
    setup_pwm_pin(LED3);
}

int main() {
    setup();

    while (true) {
        // Toggle LED on/off with SW1
        if (gpio_get(SW1) == 0 && sw1_released) {    // Detect button press with release condition
            if (led_on) {
                if (brightness == 0) {
                    brightness = 500;                // Set brightness to 50%
                    update_brightness(brightness);
                } else {
                    led_on = false;                  // Turn off LED
                    update_brightness(0);            // Set brightness to 0
                }
            } else {
                led_on = true;                       // Turn on LED
                if (brightness == 0) {
                    brightness = 500;                // Set to 50% if brightness was zero
                    update_brightness(brightness);
                } else {
                    update_brightness(brightness);   // Set brightness to current level
                }
            }
            sw1_released = false;                    // Mark button as no longer released
            sleep_ms(250);                           // Increased debounce delay to stabilize input
        } else if (gpio_get(SW1) == 1) {             // Detect when button is fully released
            sw1_released = true;
        }

        // Increase brightness with SW0 (hold to increase)
        if (led_on && gpio_get(SW0) == 0) {
            if (!sw0_holding) {                       // Detect first press
                sw0_holding = true;
            }
            brightness += 100;                        // Increase brightness gradually
            if (brightness > 1000) brightness = 1000; // Cap to max brightness
            update_brightness(brightness);
            sleep_ms(100);                           // Adjust the delay for smoothness
        } else {
            sw0_holding = false;                     // Button released
        }

        // Decrease brightness with SW2 (hold to decrease)
        if (led_on && gpio_get(SW2) == 0) {
            if (!sw2_holding) {                       // Detect first press
                sw2_holding = true;
            }
            brightness -= 100;                        // Decrease brightness gradually
            if (brightness < 0) brightness = 0;      // Cap to min brightness
            update_brightness(brightness);
            sleep_ms(100);                           // Adjust the delay for smoothness
        } else {
            sw2_holding = false;                     // Button released
        }
    }
}
