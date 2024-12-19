#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

//Define I2C pins for I2C0 interface
#define I2C1_SDA_PIN 14         // SDA pin for I2C1
#define I2C1_SCL_PIN 15          // SCL pin for I2C1

//Define LED pins on the board
#define LED1 22                // LED1 connected to GPIO 22
#define LED2 21               // LED2 connected to GPIO 21
#define LED3 20                // LED3 connected to GPIO 20

// Define button (switch) pins
#define SW_0 9                   // Button SW_0 connected to GPIO 9
#define SW_1 8                 // Button SW_1 connected to GPIO 8
#define SW_2 7                   // Button SW_2 connected to GPIO 7

// EEPROM device and configuration
#define DEVADDR 0x50             // I2C address of the EEPROM chip
#define BAUDRATE 100000         // I2C communication speed (100kHz)
#define I2C_MEMORY_SIZE 32768    // Total memory size of the EEPROM (32KB)

// Definitions related to log entries
#define ENTRY_SIZE 64          // Each log entry is 64 bytes
#define MAX_ENTRIES 32           // Maximum number of entries is 32 (64 * 32 = 2048 bytes used for logs)
#define STRLEN 62                // Maximum length of the string we store (62 + CRC + '\0' = 64 total)
#define FIRST_ADDRESS 0         // The first address in EEPROM to start storing logs

// Structure to hold LED states
typedef struct ledstate {
    uint8_t state;              // Current state of the LED bitfield
    uint8_t not_state;           // Inverted state of the LED bitfield (for validation)
} ledstate;

// We use a static local variable hidden inside a function and a macro to access it,
// so we don't have a global variable for the current write address.
static int32_t* getCurrentWriteAddressPtr() {
    static int32_t cwa = -1;     // 'cwa' starts as -1 indicating "not initialized"
    return &cwa;                 // Return address of cwa so we can access/modify it through a macro
}

// Macro to access the current write address as if it were a variable
#define current_write_address (*getCurrentWriteAddressPtr())

// Function prototypes for all functions we will use
void initPins(void);
void eepromWrite(uint16_t memory_address, const uint8_t *data, size_t length);
void eepromRead(uint16_t memory_address, uint8_t *data_read, size_t length);
void updateLedState(ledstate *ls, uint8_t value);
bool validateLedState(ledstate *ls);
bool isButtonPressed(uint button);
uint16_t computeCRC16(const uint8_t *data_p, size_t length);
void logWriteEntry(const char *str);
void logReadEntries();
void clearLogEntries();
void initializeLogPointer();

// This function finds the next free entry in the EEPROM log without printing anything.
// It scans the EEPROM to find where valid logs end and returns the address of the next free spot.
static uint16_t findNextFreeEntry(void) {
    uint16_t address = FIRST_ADDRESS;     // Start scanning from the first log address
    uint8_t read_buff[ENTRY_SIZE];        // Temporary buffer to read a log entry from EEPROM

    // Loop until we reach the max possible entries or find an invalid one
    while (address < (FIRST_ADDRESS + ENTRY_SIZE * MAX_ENTRIES)) {
        eepromRead(address, read_buff, ENTRY_SIZE); // Read one 64-byte entry from EEPROM into read_buff

        if (read_buff[0] == 0) {
            // If the first byte is 0, it means this entry is empty, so we've found a free spot
            break;
        }

        // Find the end of the stored string (look for '\0')
        uint8_t *p = read_buff; // Pointer to start of read_buff
        // Move p forward until we hit '\0' or reach near the end
        while (*p != '\0' && (p - read_buff) < (ENTRY_SIZE - 3)) {
            p++;
        }

        // Calculate the length of the data including the '\0'
        size_t data_length = (size_t)(p - read_buff) + 1;

        // Compute the CRC of the data read
        uint16_t crc_calc = computeCRC16(read_buff, data_length);
        // Extract the stored CRC (two bytes after the string)
        uint8_t stored_crc_hi = read_buff[data_length];
        uint8_t stored_crc_lo = read_buff[data_length+1];
        uint16_t stored_crc = (stored_crc_hi << 8) | stored_crc_lo;

        if (crc_calc == stored_crc) {
            // If CRC matches, this entry is valid, move to the next entry
            address += ENTRY_SIZE;
        } else {
            // If CRC doesn't match, we've reached invalid data, so stop scanning
            break;
        }
    }

    return address; // Return the address of the next free entry or the end of valid logs
}

int main() {
    initPins(); // Initialize GPIO pins, I2C, etc.

    // If current_write_address is -1 (not initialized), initialize it now
    if (current_write_address == -1) {
        initializeLogPointer();
    }

    // Create a ledstate struct to hold LED states
    ledstate ls;
    uint8_t stored_led_state = 0;
    uint8_t stored_led_not_state = 0;

    const uint16_t led_state_address = I2C_MEMORY_SIZE - 1; // Use the last byte of EEPROM for LED states

    // Write a "Boot" log entry at the start of the program
    char boot_log[] = "Boot";
    printf("%s\n", boot_log);          // Print "Boot" to the console
    logWriteEntry(boot_log);           // Store "Boot" in EEPROM log

    // Read the LED state from EEPROM
    eepromRead(led_state_address, &stored_led_state, 1);       // Read LED state byte
    eepromRead(led_state_address - 1, &stored_led_not_state, 1);// Read LED not_state byte
    ls.state = stored_led_state;        // Set ls.state
    ls.not_state = stored_led_not_state;// Set ls.not_state

    // Validate the LED state read from EEPROM
    if (!validateLedState(&ls)) {
        // If invalid, set a default LED pattern (0x02 means LED2 on)
        updateLedState(&ls, 0x02);
        // Write this corrected state to EEPROM
        eepromWrite(led_state_address, &ls.state, 1);
        eepromWrite(led_state_address - 1, &ls.not_state, 1);
        gpio_put(LED2, ls.state & 0x02); // Turn on LED2
        sleep_ms(100);
    } else {
        // If valid, set LEDs according to state bits
        gpio_put(LED3, ls.state & 0x01);
        gpio_put(LED2, ls.state & 0x02);
        gpio_put(LED1, ls.state & 0x04);
    }

    char input_command1[STRLEN]; // Buffer for user input commands
    char chr;                    // Character read from input
    int lp = 0;                      // Index for input_command1

    while (true) {
        char log_message[STRLEN]; // Buffer to create log messages when a button is pressed

        // Check if SW_0 is pressed
        if (!isButtonPressed(SW_0)) {
            // Toggle the least significant bit of LED state
            ls.state ^= 0x01;
            printf("SW_0 is pressed\n");
            gpio_put(LED3, ls.state & 0x01); // Update LED3 according to ls.state
            updateLedState(&ls, ls.state);
            // Write updated LED state to EEPROM
            eepromWrite(led_state_address, &ls.state, 1);
            eepromWrite(led_state_address - 1, &ls.not_state, 1);
            sleep_ms(100);

            // Log the event with timestamp and LED state
            printf("Time since boot: %llu seconds, LED state: 0x%02X\n", time_us_64() / 1000000, ls.state);
            snprintf(log_message, STRLEN, "Time since boot: %llu seconds, LED state: 0x%02X", time_us_64() / 1000000, ls.state);
            logWriteEntry(log_message);

            // Wait until button is released
            while (!gpio_get(SW_0));
        }

        // Check if SW_1 is pressed
        if (!isButtonPressed(SW_1)) {
            ls.state ^= 0x02; // Toggle the bit for LED2
            printf("SW_1 is pressed\n");
            gpio_put(LED2, ls.state & 0x02);
            updateLedState(&ls, ls.state);
            // Store updated LED state
            eepromWrite(led_state_address, &ls.state, 1);
            eepromWrite(led_state_address - 1, &ls.not_state, 1);
            sleep_ms(100);

            // Log the change
            printf("Time since boot: %llu seconds, LED state: 0x%02X\n", time_us_64() / 1000000, ls.state);
            snprintf(log_message, STRLEN, "Time since boot: %llu seconds, LED state: 0x%02X", time_us_64() / 1000000, ls.state);
            logWriteEntry(log_message);

            // Wait until button is released
            while (!gpio_get(SW_1));
        }

        // Check if SW_2 is pressed
        if (!isButtonPressed(SW_2)) {
            ls.state ^= 0x04; // Toggle the bit for LED1
            printf("SW_2 is pressed\n");
            gpio_put(LED1, ls.state & 0x04);
            updateLedState(&ls, ls.state);
            // Store updated LED state
            eepromWrite(led_state_address, &ls.state, 1);
            eepromWrite(led_state_address - 1, &ls.not_state, 1);
            sleep_ms(100);

            // Log the change
            printf("Time since boot: %llu seconds, LED state: 0x%02X\n", time_us_64() / 1000000, ls.state);
            snprintf(log_message, STRLEN, "Time since boot: %llu seconds, LED state: 0x%02X", time_us_64() / 1000000, ls.state);
            logWriteEntry(log_message);

            // Wait until button is released
            while (!gpio_get(SW_2));
        }

        // Check for user commands from serial input (non-blocking)
        chr = getchar_timeout_us(0); // Read a character if available, else return 255
        while (chr != 255 && lp < STRLEN - 1) {
            input_command1[lp] = chr; // Store the character in the command buffer
            lp++;

            // If we hit a newline or carriage return, we got a full command
            if (chr == '\n' || chr == '\r') {
                input_command1[lp - 1] = '\0'; // Replace newline with null terminator
                lp = 0; // Reset input index

                // Process the command
                if (strcmp(input_command1, "erase") == 0) {
                    clearLogEntries();
                    // After clearing, reset the current_write_address
                    current_write_address = -1;
                    initializeLogPointer();
                } else if (strcmp(input_command1, "read") == 0) {
                    logReadEntries();
                } else if (strlen(input_command1) > 0) {
                    // Unrecognized command
                    printf("\n>>> Error: Unrecognized command '%s'.\n", input_command1);
                }
                break; // Done processing this command
            }

            // Wait a bit for next character
            chr = getchar_timeout_us(1000);
        }
    }
    return 0; //never reaches here
}


void initializeLogPointer() {
    // Only re-initialize if current_write_address is -1
    if (current_write_address == -1) {
        uint16_t addr = findNextFreeEntry();
        current_write_address = addr; // Set the global pointer to next free entry
    }
}


void logWriteEntry(const char *str) {
    // If we have never initialized current_write_address, do it now
    if (current_write_address == -1) {
        current_write_address = findNextFreeEntry();
    }

    // If we've reached the max entries, clear and start over
    if (current_write_address >= (FIRST_ADDRESS + ENTRY_SIZE * MAX_ENTRIES)) {
        printf("Maximum log entries. Erasing the log to log new messages\n");
        clearLogEntries();
        current_write_address = -1;
        initializeLogPointer();
    }

    // Determine length of the input string
    size_t size_length = strlen(str);
    if (size_length >= STRLEN - 1) {
        size_length = STRLEN - 1; // Prevent overflow
    }

    uint8_t log_buf[ENTRY_SIZE] = {0}; // Buffer for the log entry
    // Copy the string into log_buf
    strncpy((char *)log_buf, str, size_length);

    // Set null terminator
    uint8_t *pEnd = log_buf + size_length;
    *pEnd = '\0';
    pEnd++;

    // Compute CRC of the string (including '\0')
    uint16_t crc = computeCRC16(log_buf, size_length + 1);
    *pEnd = (uint8_t)(crc >> 8);
    pEnd++;
    *pEnd = (uint8_t)crc;

    // Write the log entry into EEPROM
    eepromWrite((uint16_t)current_write_address, log_buf, ENTRY_SIZE);
    printf("Log written at address: %u\n", (uint16_t)current_write_address);

    // Move to next potential entry address
    current_write_address += ENTRY_SIZE;
}


void logReadEntries() {
    printf("Reading log entries...\n");
    uint16_t address = FIRST_ADDRESS;
    uint8_t read_buff[ENTRY_SIZE];

    // Read entries until we find an invalid or empty one
    while (address < (FIRST_ADDRESS + ENTRY_SIZE * MAX_ENTRIES)) {
        eepromRead(address, read_buff, ENTRY_SIZE);

        if (read_buff[0] == 0) {
            // Empty entry means no more logs
            break;
        }

        // Find the end of the string
        uint8_t *p = read_buff;
        while (*p != '\0' && (p - read_buff) < (ENTRY_SIZE - 3)) {
            p++;
        }
        size_t data_length = (size_t)(p - read_buff) + 1;

        uint16_t crc_calc = computeCRC16(read_buff, data_length);
        uint8_t stored_crc_hi = read_buff[data_length];
        uint8_t stored_crc_lo = read_buff[data_length+1];
        uint16_t stored_crc = (stored_crc_hi << 8) | stored_crc_lo;

        if (crc_calc == stored_crc && data_length < ENTRY_SIZE - 2) {
            // Valid entry, print it
            printf("Log entry: %s\n", (char*)read_buff);
            address += ENTRY_SIZE; // Move to next entry
        } else {
            // Invalid entry found, stop reading
            break;
        }
    }

    printf("End of valid log entries.\n");
}


void clearLogEntries() {
    printf("Erasing all log messages...\nDelete log from address:\n");
    uint16_t address = FIRST_ADDRESS;

    // Overwrite all possible entries with zeros
    while (address < (FIRST_ADDRESS + ENTRY_SIZE * MAX_ENTRIES)) {
        printf("%u  ", address);
        uint8_t buf[ENTRY_SIZE] = {0};
        eepromWrite(address, buf, ENTRY_SIZE);
        address += ENTRY_SIZE;
    }
    printf("\nLog erased.\n");
}


uint16_t computeCRC16(const uint8_t *data_p, size_t length) {
    uint8_t x;
    uint16_t crc = 0xFFFF; // Start with 0xFFFF
    while (length--) {
        // This is a CRC-16 calculation loop
        x = crc >> 8 ^ *data_p++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((uint16_t)(x << 5) ^ ((uint16_t)x));
    }
    return crc; // Returns the computed CRC16 value
}


void eepromWrite(uint16_t memory_address, const uint8_t *data, size_t length) {
    // Prepare a buffer for I2C write: [HighAddrByte, LowAddrByte, Data...]
    uint8_t buf[2 + length];
    buf[0] = (uint8_t)(memory_address >> 8); // High byte of address
    buf[1] = (uint8_t)(memory_address);      // Low byte of address
    for (size_t i = 0; i < length; ++i) {
        buf[i + 2] = data[i]; // Copy data into buffer
    }
    // Write to EEPROM via I2C
    i2c_write_blocking(i2c1, DEVADDR, buf, length + 2, false);
    sleep_ms(20); // Wait for EEPROM write cycle to complete
}


void eepromRead(uint16_t memory_address, uint8_t *data_read, size_t length) {
    uint8_t buf[2];
    buf[0] = (uint8_t)(memory_address >> 8); // High byte of address
    buf[1] = (uint8_t)(memory_address);      // Low byte of address
    // Write the address we want to read from
    i2c_write_blocking(i2c1, DEVADDR, buf, 2, true);
    // Now read the data from that address
    i2c_read_blocking(i2c1, DEVADDR, data_read, length, false);
}


void updateLedState(ledstate *ls, uint8_t value) {
    ls->state = value;          // Set the state
    ls->not_state = (uint8_t)(~value); // Inverse the state
}


bool validateLedState(ledstate *ls) {
    // LED state is valid if ls->state matches the inverse of ls->not_state
    return ls->state == (uint8_t)~ls->not_state;
}


bool isButtonPressed(uint button) {
    // This function checks if a button is pressed by reading its GPIO state multiple times
    int press = 0;
    int release = 0;
    while (press < 3 && release < 3) {
        if (gpio_get(button)) {
            // If button is currently high (released state due to pull-up)
            press++;
            release = 0;
        } else {
            // If button is low (possibly pressed)
            release++;
            press = 0;
        }
        sleep_ms(10);
    }
    // If press count is greater than release count, it means button is stable in pressed state
    return press > release;
}


void initPins(void) {
    stdio_init_all(); // Initialize all stdio functions

    // Initialize I2C controllers
    i2c_init(i2c1, BAUDRATE);
    //i2c_init(i2c0, BAUDRATE);

    // Set I2C0 pins to their I2C functions
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);

    // Initialize LED pins as outputs
    gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);
    gpio_init(LED2);
    gpio_set_dir(LED2, GPIO_OUT);
    gpio_init(LED3);
    gpio_set_dir(LED3, GPIO_OUT);

    // Initialize button pins as inputs with pull-ups
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
