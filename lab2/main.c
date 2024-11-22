
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"

// Define GPIO pins
#define LED1 20
#define LED2 21
#define LED3 22
#define ROT_RIGHT 10
#define ROT_LEFT 11
#define ROT_SW 12

int brightness = 0;            // Initial brightness
bool led_state = false;        // LED on/off state
bool rot_sw_release = true;    // Button release state

typedef enum {
    EVENT_CW,
    EVENT_CCW
} encoder_event_t;

queue_t encoder_queue;  // Queue to store encoder events

// Function to initialize PWM on a pin
void setup_pwm_pin(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_clkdiv(slice_num, 125.0f);
    pwm_set_wrap(slice_num, 1000);
    pwm_set_enabled(slice_num, true);
}

// Function to update LED brightness
void update_brightness(int level) {
    pwm_set_gpio_level(LED1, level);
    pwm_set_gpio_level(LED2, level);
    pwm_set_gpio_level(LED3, level);
}

// Interrupt handler for the rotary encoder
void encoder_callback(uint gpio, uint32_t events) {
    bool rot_left_state = gpio_get(ROT_LEFT);
    bool rot_right_state = gpio_get(ROT_RIGHT);
    if (led_state) {
        encoder_event_t event;
        if (gpio == ROT_LEFT && rot_right_state == true) {
            event = EVENT_CCW;
            queue_try_add(&encoder_queue, &event);
        } else if (gpio == ROT_RIGHT && rot_left_state == true) {
            event = EVENT_CW;
            queue_try_add(&encoder_queue, &event);
        }
    }
}

// Initialize GPIO pins, PWM, and the queue for LEDs
void setup() {
    stdio_init_all();

    gpio_init(ROT_RIGHT);
    gpio_set_dir(ROT_RIGHT, GPIO_IN);
    gpio_pull_up(ROT_RIGHT);

    gpio_init(ROT_LEFT);
    gpio_set_dir(ROT_LEFT, GPIO_IN);
    gpio_pull_up(ROT_LEFT);

    gpio_init(ROT_SW);
    gpio_set_dir(ROT_SW, GPIO_IN);
    gpio_pull_up(ROT_SW);

    setup_pwm_pin(LED1);
    setup_pwm_pin(LED2);
    setup_pwm_pin(LED3);

    // Initialize the encoder event queue
    queue_init(&encoder_queue, sizeof(encoder_event_t), 10);

    // Attach interrupt handlers for encoder pins
    gpio_set_irq_enabled_with_callback(ROT_LEFT, GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
    gpio_set_irq_enabled_with_callback(ROT_RIGHT, GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
}

// Main program loop
int main(void) {
    setup();

    while (true) {
        // Check if button is pressed to toggle LED state and initialize brightness
        if (gpio_get(ROT_SW) == 0 && rot_sw_release) {
            rot_sw_release = false;  // Mark the switch as pressed

            if (led_state && brightness == 0) {
                brightness = 500;  // Set brightness to 50% if it was 0
                update_brightness(brightness);
                printf("Brightness set to 50%% (500) with LED on\n");
            } else {
                // Toggle LED on/off state
                led_state = !led_state;
                update_brightness(led_state ? brightness : 0);
                printf("LED State: %d, Brightness: %d\n", led_state, brightness);
            }

            sleep_ms(20);  // Debounce delay for button
        } else if (gpio_get(ROT_SW) == 1) {
            rot_sw_release = true;  // Mark switch as released
        }

        // Process encoder events from the queue
        if (led_state) {
            encoder_event_t event;
            while (queue_try_remove(&encoder_queue, &event)) {
                if (event == EVENT_CW) {
                    brightness += 100;
                    if (brightness > 1000) brightness = 1000;
                    printf("Brightness increased to: %d\n", brightness);
                } else if (event == EVENT_CCW) {
                    brightness -= 100;
                    if (brightness < 0) brightness = 0;
                    printf("Brightness decreased to: %d\n", brightness);
                }
                update_brightness(brightness);  // Update brightness
            }
        }

        sleep_ms(200);  // Small delay to avoid high CPU usage
    }
}
