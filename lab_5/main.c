/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Define pins
#define SW0 7     // Button to toggle LED1
#define SW1 8     // Button to toggle LED2
#define SW2 9     // Button to toggle LED3
#define LED1 20   // First LED pin
#define LED2 21   // Second LED pin
#define LED3 22   // Third LED pin

// Define pins for I²C
#define I2C_SDA 4  // SDA pin for I²C0
#define I2C_SCL 5  // SCL pin for I²C0

// Define I²C address for the EEPROM
#define EEPROM_I2C_ADDR 0x50

// EEPROM memory size
#define EEPROM_SIZE 256
#define LED_STATE_ADDRESS 0x00 // EEPROM address for saving LED state

// Variables for LED states and button debouncing
uint8_t led_states = 0b010; // Default: LED2 on, others off
bool sw0_released = true, sw1_released = true, sw2_released = true;

// Function prototypes
void eeprom_write_byte(uint8_t address, uint8_t data);
uint8_t eeprom_read_byte(uint8_t address);
void save_to_eeprom(uint8_t state);
bool load_from_eeprom(uint8_t *state);
void toggle_led1();
void toggle_led2();
void toggle_led3();
void print_led_state();

void setup() {
    stdio_init_all();

    // Initialize I²C
    i2c_init(i2c0, 100 * 1000); // 100 kHz I²C clock
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

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

    // Initialize LED pins as output
    gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);

    gpio_init(LED2);
    gpio_set_dir(LED2, GPIO_OUT);

    gpio_init(LED3);
    gpio_set_dir(LED3, GPIO_OUT);

    // Load LED state from EEPROM
    uint8_t saved_state;
    if (load_from_eeprom(&saved_state)) {
        led_states = saved_state; // Valid state loaded
    } else {
        led_states = 0b010; // Default state (middle LED on)
    }

    // Set initial LED states
    gpio_put(LED1, (led_states & 0b001) ? 1 : 0);
    gpio_put(LED2, (led_states & 0b010) ? 1 : 0);
    gpio_put(LED3, (led_states & 0b100) ? 1 : 0);

    print_led_state();
}

int main() {
    setup();

    while (true) {
        // Separate toggle logic for each LED
        toggle_led1();
        toggle_led2();
        toggle_led3();
    }
}

// Toggle LED1 when SW0 is pressed
void toggle_led1() {
    if (gpio_get(SW0) == 0 && sw0_released) { // Button pressed and released
        led_states ^= 0b001; // Toggle LED1 state
        gpio_put(LED1, (led_states & 0b001) ? 1 : 0); // Update LED1
        print_led_state();
        save_to_eeprom(led_states); // Save state to EEPROM
        sw0_released = false;
        sleep_ms(200); // Debounce delay
    } else if (gpio_get(SW0) == 1) { // Button released
        sw0_released = true;
    }
}

// Toggle LED2 when SW1 is pressed
void toggle_led2() {
    if (gpio_get(SW1) == 0 && sw1_released) { // Button pressed and released
        led_states ^= 0b010; // Toggle LED2 state
        gpio_put(LED2, (led_states & 0b010) ? 1 : 0); // Update LED2
        print_led_state();
        save_to_eeprom(led_states); // Save state to EEPROM
        sw1_released = false;
        sleep_ms(200); // Debounce delay
    } else if (gpio_get(SW1) == 1) { // Button released
        sw1_released = true;
    }
}

// Toggle LED3 when SW2 is pressed
void toggle_led3() {
    if (gpio_get(SW2) == 0 && sw2_released) { // Button pressed and released
        led_states ^= 0b100; // Toggle LED3 state
        gpio_put(LED3, (led_states & 0b100) ? 1 : 0); // Update LED3
        print_led_state();
        save_to_eeprom(led_states); // Save state to EEPROM
        sw2_released = false;
        sleep_ms(200); // Debounce delay
    } else if (gpio_get(SW2) == 1) { // Button released
        sw2_released = true;
    }
}

// Save LED state to EEPROM
void save_to_eeprom(uint8_t state) {
    eeprom_write_byte(LED_STATE_ADDRESS, state); // Write state to predefined address
    sleep_ms(10); // Allow EEPROM write time
    printf("Saved to EEPROM: %03b\n", state);
}

// Load LED state from EEPROM
bool load_from_eeprom(uint8_t *state) {
    uint8_t saved_state = eeprom_read_byte(LED_STATE_ADDRESS); // Read state from EEPROM
    if (saved_state <= 0b111) { // Validate state (only 3 bits used)
        *state = saved_state;
        printf("Loaded from EEPROM: %03b\n", *state);
        return true;
    }
    printf("Invalid EEPROM data. Using default state.\n");
    return false;
}

// Function to write a byte to the EEPROM
void eeprom_write_byte(uint8_t address, uint8_t data) {
    uint8_t buffer[2] = {address, data}; // Address followed by data
    i2c_write_blocking(i2c0, EEPROM_I2C_ADDR, buffer, 2, false); // Write to EEPROM
    printf("EEPROM: Written 0x%02X to address 0x%02X\n", data, address);
}

// Function to read a byte from the EEPROM
uint8_t eeprom_read_byte(uint8_t address) {
    uint8_t data;
    i2c_write_blocking(i2c0, EEPROM_I2C_ADDR, &address, 1, true); // Set address
    i2c_read_blocking(i2c0, EEPROM_I2C_ADDR, &data, 1, false);   // Read data
    printf("EEPROM: Read 0x%02X from address 0x%02X\n", data, address);
    return data;
}

// Function to print the current LED state
void print_led_state() {
    printf("LED states: %03b at %lu seconds\n", led_states, time_us_64() / 1000000);
}
*/

/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/time.h"

// I2C GPIO Pins
#define SDA_PIN 14
#define SCL_PIN 15

// EEPROM Address and Control Registers
#define EEPROM_ADDR 0x50
#define EEPROM_CTRL_REG 0xFE
#define EEPROM_DATA_REG 0x7F

// LED Pin Definitions
#define LED_1_PIN 20
#define LED_2_PIN 21
#define LED_3_PIN 22

// Button Pin Definitions
#define BUTTON_1_PIN 9
#define BUTTON_2_PIN 8
#define BUTTON_3_PIN 7

typedef enum { OFF = 0, ON = 1 } LedStatus;

// Function prototypes
void init_gpio_pins();
void init_i2c();
void toggle_led_state(LedStatus *current_state, int led_bit, int gpio_pin, uint8_t *stored_state);
void save_state_to_eeprom(uint8_t stored_state);
uint8_t load_state_from_eeprom();
void display_led_status(uint64_t elapsed_time, LedStatus led1, LedStatus led2, LedStatus led3);

const char* led_state_to_str(LedStatus state) {
    return (state == OFF) ? "off" : "on";
}

void init_gpio_pins() {
    gpio_init(LED_1_PIN);
    gpio_set_dir(LED_1_PIN, GPIO_OUT);

    gpio_init(LED_2_PIN);
    gpio_set_dir(LED_2_PIN, GPIO_OUT);

    gpio_init(LED_3_PIN);
    gpio_set_dir(LED_3_PIN, GPIO_OUT);

    gpio_init(BUTTON_1_PIN);
    gpio_set_dir(BUTTON_1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_1_PIN);

    gpio_init(BUTTON_2_PIN);
    gpio_set_dir(BUTTON_2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_2_PIN);

    gpio_init(BUTTON_3_PIN);
    gpio_set_dir(BUTTON_3_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_3_PIN);
}

void init_i2c() {
    i2c_init(i2c1, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
}

void save_state_to_eeprom(uint8_t stored_state) {
    uint8_t buffer[3] = { EEPROM_CTRL_REG, EEPROM_DATA_REG, stored_state };
    i2c_write_blocking(i2c1, EEPROM_ADDR, buffer, 3, false);
}

uint8_t load_state_from_eeprom() {
    uint8_t address[2] = { EEPROM_CTRL_REG, EEPROM_DATA_REG };
    uint8_t stored_state = 0x00;
    i2c_write_blocking(i2c1, EEPROM_ADDR, address, 2, true);
    i2c_read_blocking(i2c1, EEPROM_ADDR, &stored_state, 1, false);
    return stored_state;
}

void display_led_status(uint64_t elapsed_time, LedStatus led1, LedStatus led2, LedStatus led3) {
    printf("LED TIME SINCE START : %llu seconds !!! \n STATE: LED 1: %s -: LED 2: %s -: LED 3: %s !!!\n",
           elapsed_time, led_state_to_str(led1), led_state_to_str(led2), led_state_to_str(led3));
}

void toggle_led_state(LedStatus *current_state, int led_bit, int gpio_pin, uint8_t *stored_state) {
    *current_state ^= 1;  // Toggle LED state
    *stored_state = (*stored_state & ~(1 << led_bit)) | (*current_state << led_bit);
    gpio_put(gpio_pin, *current_state);
    save_state_to_eeprom(*stored_state);  // Update EEPROM
}

int main() {
    stdio_init_all();
    init_gpio_pins();
    init_i2c();

    // Read LED state from EEPROM
    uint8_t stored_state = load_state_from_eeprom();
    LedStatus led1 = (stored_state & (1 << 0)) >> 0;
    LedStatus led2 = (stored_state & (1 << 1)) >> 1;
    LedStatus led3 = (stored_state & (1 << 2)) >> 2;

    // Set default state if invalid
    if (led1 == -1 || led2 == -1 || led3 == -1) {
        led1 = OFF;
        led2 = ON;
        led3 = OFF;
        stored_state = (led1 << 0) | (led2 << 1) | (led3 << 2);
        save_state_to_eeprom(stored_state);
    }

    // Set initial LED states
    gpio_put(LED_1_PIN, led1);
    gpio_put(LED_2_PIN, led2);
    gpio_put(LED_3_PIN, led3);

    absolute_time_t start_time = get_absolute_time();

    while (true) {
        uint64_t elapsed_time = absolute_time_diff_us(start_time, get_absolute_time()) / 1000000;

        // Button 1 press
        if (!gpio_get(BUTTON_1_PIN)) {
            while (!gpio_get(BUTTON_1_PIN)) { sleep_ms(10); }
            toggle_led_state(&led1, 0, LED_1_PIN, &stored_state);
            display_led_status(elapsed_time, led1, led2, led3);
        }

        // Button 2 press
        if (!gpio_get(BUTTON_2_PIN)) {
            while (!gpio_get(BUTTON_2_PIN)) { sleep_ms(10); }
            toggle_led_state(&led2, 1, LED_2_PIN, &stored_state);
            display_led_status(elapsed_time, led1, led2, led3);
        }

        // Button 3 press
        if (!gpio_get(BUTTON_3_PIN)) {
            while (!gpio_get(BUTTON_3_PIN)) { sleep_ms(10); }
            toggle_led_state(&led3, 2, LED_3_PIN, &stored_state);
            display_led_status(elapsed_time, led1, led2, led3);
        }
    }
}
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// GPIO Pin Definitions
#define LED_0 20
#define LED_1 21
#define LED_2 22
#define SW_0 9
#define SW_1 8
#define SW_2 7

// EEPROM and I2C Definitions
#define DEV_ADDR 0x50
#define I2C_ID i2c1
#define HIGHEST_ADDR 0x7FFF
#define PAGE_SIZE 64
#define MAGIC_BYTE 0xA5

typedef struct LED_State {
    uint8_t pin;
    uint8_t state;
    uint8_t not_state; // Used for validation
} LED_State;

// Function to Set LED State
void led_set_state(LED_State *state, uint8_t value) {
    state->state = value;
    state->not_state = ~value;
}

bool led_state_is_valid(LED_State *state) {
    return state->state == (uint8_t)~state->not_state;
}

// GPIO Initialization
void init_gpio(void) {
    gpio_init(LED_0);
    gpio_set_dir(LED_0, GPIO_OUT);

    gpio_init(LED_1);
    gpio_set_dir(LED_1, GPIO_OUT);

    gpio_init(LED_2);
    gpio_set_dir(LED_2, GPIO_OUT);

    gpio_init(SW_0);
    gpio_set_dir(SW_0, GPIO_IN);
    gpio_pull_up(SW_0);

    gpio_init(SW_1);
    gpio_set_dir(SW_1, GPIO_IN);
    gpio_pull_up(SW_1);

    gpio_init(SW_2);
    gpio_set_dir(SW_2, GPIO_IN);
    gpio_pull_up(SW_2);
}

// EEPROM Functions
void eeprom_write(uint16_t address, uint8_t *data, size_t len) {
    uint8_t frame[len + 2];
    frame[0] = (address >> 8) & 0xFF; // High byte of address
    frame[1] = address & 0xFF;       // Low byte of address
    memcpy(&frame[2], data, len);
    i2c_write_blocking(I2C_ID, DEV_ADDR, frame, len + 2, false);
    sleep_ms(5);
}

void eeprom_read(uint16_t address, uint8_t *buffer, size_t len) {
    uint8_t addr[2] = {
        (address >> 8) & 0xFF,
        address & 0xFF
    };
    i2c_write_blocking(I2C_ID, DEV_ADDR, addr, 2, true);
    i2c_read_blocking(I2C_ID, DEV_ADDR, buffer, len, false);
}

void eeprom_store_led_state(LED_State *leds) {
    uint16_t address = HIGHEST_ADDR - sizeof(LED_State) * 3;

    // Write the LED state twice: normal and inverted
    for (uint8_t i = 0; i < 3; i++) {
        eeprom_write(address + i * 2, (uint8_t *)&leds[i].state, 1);
        uint8_t inverted = ~leds[i].state;
        eeprom_write(address + i * 2 + 1, &inverted, 1);
    }
}

bool eeprom_load_led_state(LED_State *leds) {
    uint16_t address = HIGHEST_ADDR - sizeof(LED_State) * 3;

    for (uint8_t i = 0; i < 3; i++) {
        uint8_t state, inverted;
        eeprom_read(address + i * 2, &state, 1);
        eeprom_read(address + i * 2 + 1, &inverted, 1);

        if (state != (uint8_t)~inverted) {
            return false; // Invalid state
        }
        led_set_state(&leds[i], state);
    }
    return true;
}

// Apply LED States to GPIO Pins
void led_apply_state(LED_State *leds) {
    for (uint8_t i = 0; i < 3; i++) {
        gpio_put(leds[i].pin, leds[i].state);
    }
}

// Toggle LED State and Store in EEPROM
void led_toggle_and_store(LED_State *leds, uint8_t index) {
    leds[index].state = !leds[index].state;
    leds[index].not_state = ~leds[index].state;
    led_apply_state(leds);
    eeprom_store_led_state(leds);

    printf("LED_%d state: %s at %llu seconds\n",
           index,
           leds[index].state ? "ON" : "OFF",
           time_us_64() / 1000000);
}

// Function to print LED states and time on startup
void print_initial_state(LED_State *leds) {
    uint64_t timestamp = time_us_64() / 1000000; // Convert microseconds to seconds
    printf("Program started at %llu seconds.\n", timestamp);

    for (int i = 0; i < 3; i++) {
        printf("LED_%d state: %s\n",
               i,
               leds[i].state ? "ON" : "OFF");
    }
}

// Main Function
int main(void) {
    stdio_init_all();
    printf("System Booting...\n");

    init_gpio();
    i2c_init(I2C_ID, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C); // SDA
    gpio_set_function(15, GPIO_FUNC_I2C); // SCL
    printf("GPIO and I2C Initialized.\n");

    LED_State leds[3] = {
        {LED_0, 0, ~0},
        {LED_1, 0, ~0},
        {LED_2, 0, ~0}
    };

    // Attempt to Load LED State from EEPROM
    if (!eeprom_load_led_state(leds)) {
        printf("No valid LED state found in EEPROM. Initializing...\n");
        led_set_state(&leds[0], 0);
        led_set_state(&leds[1], 1); // Set middle LED to ON
        led_set_state(&leds[2], 0);
        eeprom_store_led_state(leds);
    }

    led_apply_state(leds);

    // Print initial state
    print_initial_state(leds);

    while (1) {
        if (!gpio_get(SW_0)) {
            while (!gpio_get(SW_0)) { sleep_ms(50); } // Debounce
            led_toggle_and_store(leds, 0);
        }

        if (!gpio_get(SW_1)) {
            while (!gpio_get(SW_1)) { sleep_ms(50); } // Debounce
            led_toggle_and_store(leds, 1);
        }

        if (!gpio_get(SW_2)) {
            while (!gpio_get(SW_2)) { sleep_ms(50); } // Debounce
            led_toggle_and_store(leds, 2);
        }

        sleep_ms(10); // Prevent CPU overuse
    }

    return 0;
}
