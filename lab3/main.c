/*
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state
bool is_calibrated = false;
int steps_per_revolution = 0;

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
void run_motor(int steps, int delay_ms, int direction) {
    for (int i = 0; i < steps; i++) {
        int step = (direction == 1) ? i % 8 : (7 - i % 8);
        set_coils(step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
    set_coils(0); // Turn off coils after movement
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor() {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        set_coils(step);
        step = (step + 1) % 8;
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command
void print_status() {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
    } else {
        printf("  Steps per revolution: not available\n");
    }
}

// Function to run the motor for N times 1/8th of a revolution
void run_command(int n) {
    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of revolutions.\n");
        return;
    }

    int steps_to_run = (is_calibrated ? steps_per_revolution : 4096) / 8;
    int total_steps = steps_to_run * n;
    printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);
    run_motor(total_steps, 2, 1);  // Default to clockwise direction (1)
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Main program loop
    while (1) {
        printf("Enter command (status, calib, or run N): ");
        char command[10];
        int n = 0;
        scanf("%s", command);

        if (strcmp(command, "status") == 0) {
            print_status();
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor();
        } else if (strncmp(command, "run", 3) == 0) {
            // Check if an argument (N) was provided
            if (sscanf(command + 4, "%d", &n) != 1) {
                n = 8; // Default to one full revolution if N is omitted
            }
            run_command(n);
        } else {
            printf("Unknown command.\n");
        }
    }
}
*/
/*
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state
bool is_calibrated = false;
int steps_per_revolution = 0;

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
void run_motor(int steps, int delay_ms, int direction) {
    for (int i = 0; i < steps; i++) {
        int step = (direction == 1) ? i % 8 : (7 - i % 8);
        set_coils(step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
    set_coils(0); // Turn off coils after movement
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor() {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        set_coils(step);
        step = (step + 1) % 8;
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command
void print_status() {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
    } else {
        printf("  Steps per revolution: not available\n");
    }
}

// Function to run the motor for N times 1/8th of a revolution
void run_command(int n) {
    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of revolutions.\n");
        return;
    }

    int steps_to_run = (is_calibrated ? steps_per_revolution : 4096) / 8;
    int total_steps = steps_to_run * n;
    printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);
    run_motor(total_steps, 2, 1);  // Default to clockwise direction (1)
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Main program loop
    while (1) {
        printf("Enter command (status, calib, or run): ");
        char command[10];
        int n = 0;
        scanf("%s", command);

        if (strcmp(command, "status") == 0) {
            print_status();
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor();
        } else if (strcmp(command, "run") == 0) {
            // Prompt user for number of revolutions if "run" is entered alone
            printf("Enter number of 1/8 revolutions (default 8 for a full revolution): ");
            if (scanf("%d", &n) != 1) {
                n = 8; // Default to one full revolution if N is omitted or invalid
            }
            run_command(n);
        } else {
            printf("Unknown command.\n");
        }
    }
}

*/
/*

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state
bool is_calibrated = false;
int steps_per_revolution = 0;

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
void run_motor(int steps, int delay_ms, int direction) {
    for (int i = 0; i < steps; i++) {
        int step = (direction == 1) ? i % 8 : (7 - i % 8);
        set_coils(step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
    set_coils(0); // Turn off coils after movement
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor() {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        set_coils(step);
        step = (step + 1) % 8;
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command
void print_status() {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
    } else {
        printf("  Steps per revolution: not available\n");
    }
}

// Function to run the motor for N times 1/8th of a revolution
void run_command(int n) {
    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of revolutions.\n");
        return;
    }

    int steps_to_run = (is_calibrated ? steps_per_revolution : 4096) / 8;
    int total_steps = steps_to_run * n;
    printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);
    run_motor(total_steps, 2, 1);  // Default to clockwise direction (1)
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Main program loop
    while (1) {
        printf("Enter command (status, calib, or run): ");
        char command[10];
        scanf("%s", command);

        if (strcmp(command, "status") == 0) {
            print_status();
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor();
        } else if (strcmp(command, "run") == 0) {
            int n = 0;
            printf("Enter number of 1/8 revolutions (default 8 for a full revolution): ");
            if (scanf("%d", &n) != 1 || n <= 0) {
                n = 8; // Default to one full revolution if input is invalid or omitted
            }
            run_command(n);
        } else {
            printf("Unknown command.\n");
        }
    }
}
*/
/* it is almost working

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state
bool is_calibrated = false;
int steps_per_revolution = 0;
int current_position = 0; // Variable to track the current position in steps

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
void run_motor(int steps, int delay_ms, int direction) {
    for (int i = 0; i < steps; i++) {
        int step = (direction == 1) ? i % 8 : (7 - i % 8);
        set_coils(step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
    set_coils(0); // Turn off coils after movement
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor() {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        set_coils(step);
        step = (step + 1) % 8;
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    current_position = 0; // Reset current position after calibration
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command
void print_status() {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
        printf("  Current position (in steps): %d\n", current_position);
    } else {
        printf("  Steps per revolution: not available\n");
    }
}

// Function to run the motor for N times 1/8th of a revolution
void run_command(int n) {
    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of 1/8 revolutions.\n");
        return;
    }

    int steps_to_run = (is_calibrated ? steps_per_revolution : 4096) / 8;
    int total_steps = steps_to_run * n;
    printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);

    // Run the motor in the forward direction
    run_motor(total_steps, 2, 1);

    // Update the current position
    current_position = (current_position + total_steps) % steps_per_revolution;  // Keep it within one revolution
    printf("Updated position: %d steps\n", current_position);
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Main program loop
    while (1) {
        printf("Enter command (status, calib, or run N): ");
        char command[10];
        int n = 0;
        scanf("%s", command);

        if (strcmp(command, "status") == 0) {
            print_status();
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor();
        } else if (strncmp(command, "run", 3) == 0) {
            // Handle cases with or without a number after "run"
            char *run_num = command + 4;
            if (sscanf(run_num, "%d", &n) != 1) {
                printf("Specify the number of 1/8 revolutions (defaulting to 8 for a full revolution): ");
                if (scanf("%d", &n) != 1) n = 8; // Default to one full revolution if no input is provided
            }
            run_command(n);
        } else {
            printf("Unknown command.\n");
        }
    }
}
*/

/*
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state
bool is_calibrated = false;
int steps_per_revolution = 0;
int current_position = 0; // Variable to track the current position in steps

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
void run_motor(int steps, int delay_ms, int direction) {
    for (int i = 0; i < steps; i++) {
        int step = (direction == 1) ? i % 8 : (7 - i % 8);
        set_coils(step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
    set_coils(0); // Turn off coils after movement
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor() {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        set_coils(step);
        step = (step + 1) % 8;
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    current_position = 0; // Reset current position after calibration
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command
void print_status() {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
        printf("  Current position (in steps): %d\n", current_position);
    } else {
        printf("  Steps per revolution: not available\n");
    }
}

// Function to run the motor for N times 1/8th of a revolution
void run_command(int n) {
    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of 1/8 revolutions.\n");
        return;
    }

    int steps_to_run = (is_calibrated ? steps_per_revolution : 4096) / 8;
    int total_steps = steps_to_run * n;
    printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);

    // Run the motor in the forward direction
    run_motor(total_steps, 2, 1);

    // Update the current position
    current_position = (current_position + total_steps) % steps_per_revolution;  // Keep it within one revolution
    printf("Updated position: %d steps\n", current_position);
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Main program loop
    while (1) {
        printf("\nEnter command (status, calib, or run N): ");
        char command[10];
        int n = 0;
        scanf("%s", command);

        if (strcmp(command, "status") == 0) {
            print_status();
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor();
        } else if (strncmp(command, "run", 3) == 0) {
            // Handle cases with or without a number after "run"
            if (sscanf(command + 3, "%d", &n) != 1) {
                printf("Specify the number of 1/8 revolutions (defaulting to 8 for a full revolution): ");
                if (scanf("%d", &n) != 1) n = 8; // Default to one full revolution if no input is provided
            }
            run_command(n);
        } else {
            printf("Unknown command.\n");
        }
    }
}
*/
/*
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state
bool is_calibrated = false;
int steps_per_revolution = 0;
int current_position = 0; // Variable to track the current position in steps

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
void run_motor(int steps, int delay_ms, int direction) {
    for (int i = 0; i < steps; i++) {
        int step = (direction == 1) ? i % 8 : (7 - i % 8);
        set_coils(step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
    set_coils(0); // Turn off coils after movement
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor() {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        set_coils(step);
        step = (step + 1) % 8;
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    current_position = 0; // Reset current position after calibration
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command
void print_status() {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
        printf("  Current position (in steps): %d\n", current_position);
    } else {
        printf("  Steps per revolution: not available\n");
    }
}

// Function to run the motor for N times 1/8th of a revolution
void run_command(int n) {
    if (!is_calibrated) {
        printf("Motor not calibrated yet. Please calibrate the motor first.\n");
        return;
    }

    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of 1/8 revolutions.\n");
        return;
    }

    int steps_to_run = steps_per_revolution / 8;  // One step is 1/8 of a revolution
    int total_steps = steps_to_run * n;

    // If no argument was given, default to a full revolution
    if (n == 0) {
        total_steps = steps_per_revolution;
        printf("Running motor for a full revolution (%d steps).\n", total_steps);
    } else {
        printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);
    }

    // Run the motor in the forward direction
    run_motor(total_steps, 2, 1);

    // Update the current position
    current_position = (current_position + total_steps) % steps_per_revolution;  // Keep it within one revolution
    printf("Updated position: %d steps\n", current_position);
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Main program loop
    while (1) {
        printf("\nEnter command (status, calib, or run N): ");
        char command[10];
        int n = 0;
        scanf("%s", command);

        if (strcmp(command, "status") == 0) {
            print_status();
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor();
        } else if (strncmp(command, "run", 3) == 0) {
            // Handle cases with or without a number after "run"
            if (sscanf(command + 3, "%d", &n) != 1 || n == 0) {
                // If no number is given or the number is 0, default to 1 full revolution
                run_command(0); // Default to 1 full revolution if no number is entered
            } else {
                run_command(n);
            }
        } else {
            printf("Unknown command.\n");
        }
    }
}
*/
/* almost working
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdlib.h>

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state
bool is_calibrated = false;
int steps_per_revolution = 0;
int current_position = 0; // Variable to track the current position in steps

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
void run_motor(int steps, int delay_ms, int direction) {
    for (int i = 0; i < steps; i++) {
        int step = (direction == 1) ? i % 8 : (7 - i % 8);
        set_coils(step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
    set_coils(0); // Turn off coils after movement
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor() {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        set_coils(step);
        step = (step + 1) % 8;
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    current_position = 0; // Reset current position after calibration
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command
void print_status() {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
        printf("  Current position (in steps): %d\n", current_position);
    } else {
        printf("  Steps per revolution: not available\n");
    }
}

// Function to run the motor for N times 1/8th of a revolution
void run_command(int n) {
    if (!is_calibrated) {
        printf("Motor not calibrated yet. Please calibrate the motor first.\n");
        return;
    }

    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of 1/8 revolutions.\n");
        return;
    }

    int steps_to_run = steps_per_revolution / 8;  // One step is 1/8 of a revolution
    int total_steps = steps_to_run * n;

    printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);

    // Run the motor in the forward direction
    run_motor(total_steps, 2, 1);

    // Update the current position
    current_position = (current_position + total_steps) % steps_per_revolution;  // Keep it within one revolution
    printf("Updated position: %d steps\n", current_position);
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Main program loop
    while (1) {
        printf("\nEnter command (status, calib, or run N): ");
        char command[10];
        int n = 0;

        scanf("%s", command);

        if (strcmp(command, "status") == 0) {
            print_status();
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor();
        } else if (strncmp(command, "run", 3) == 0) {
            // Handle cases with or without a number after "run"
            char *endptr;
            n = strtol(command + 3, &endptr, 10);

            // If conversion fails or no number was provided, default to a full revolution
            if (*endptr != '\0' || n <= 0) {
                n = 8;  // Default to full revolution
            }
            run_command(n);
        } else {
            printf("Unknown command.\n");
        }
    }
}
*/
/* this code is working
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdlib.h>

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state and motor position
bool is_calibrated = false;
int steps_per_revolution = 0;
int current_position = 0; // Track the current position in steps
int current_step = 0; // Track the last step in the sequence

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
void run_motor(int steps, int delay_ms, int direction) {
    for (int i = 0; i < steps; i++) {
        // Determine the next step in the sequence
        if (direction == 1) { // Forward
            current_step = (current_step + 1) % 8;
        } else { // Backward
            current_step = (current_step - 1 + 8) % 8;
        }

        // Set the coils to the current step in the sequence
        set_coils(current_step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
    set_coils(0); // Turn off coils after movement
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor() {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        set_coils(step);
        step = (step + 1) % 8;
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    current_position = 0; // Reset current position after calibration
    current_step = 0;     // Start from step 0
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command, showing current position and sequence step
void print_status() {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
        printf("  Current position (in steps): %d\n", current_position);
        printf("  Current sequence step (0 to 7): %d\n", current_step);  // New line showing current step in sequence
    } else {
        printf("  Steps per revolution: not available\n");
    }
}


// Function to run the motor for N times 1/8th of a revolution
void run_command(int n) {
    if (!is_calibrated) {
        printf("Motor not calibrated yet. Please calibrate the motor first.\n");
        return;
    }

    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of 1/8 revolutions.\n");
        return;
    }

    int steps_to_run = steps_per_revolution / 8;  // One step is 1/8 of a revolution
    int total_steps = steps_to_run * n;

    // If no argument was given, default to a full revolution
    if (n == 0) {
        total_steps = steps_per_revolution;
        printf("Running motor for a full revolution (%d steps).\n", total_steps);
    } else {
        printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);
    }

    // Run the motor in the forward direction
    run_motor(total_steps, 2, 1);

    // Update the current position
    current_position = (current_position + total_steps) % steps_per_revolution;  // Keep it within one revolution
    printf("Updated position: %d steps\n", current_position);
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Main program loop
    while (1) {
        printf("\nEnter command (status, calib, or run N): ");
        char command[10];
        int n = 0;
        // scanf("%s", command);
        fgets(command, 10, stdin);
        command[strcspn(command, "\r")] = 0;
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "status") == 0) {
            print_status();
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor();
        } else if (strncmp(command, "run", 3) == 0) {
            char *endptr;
            n = strtol(command + 4, &endptr, 10);

            if (*endptr != '\0' || n <= 0) {
                n = 8;
            }
            run_command(n);
        } else {
            printf("Unknown command.\n");
        }
    }
}
*/

/*
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdlib.h>

// Define GPIO pins for stepper motor coils
#define COIL_A 2
#define COIL_B 3
#define COIL_C 6
#define COIL_D 13

// Define GPIO pin for optical sensor
#define OPTO_FORK 28

// Half-stepping sequence for stepper motor control
const uint8_t step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

// Variables to store calibration state
bool is_calibrated = false;
int steps_per_revolution = 0; // Remains global as it's constant after calibration

// Function to set the GPIO states for the motor coils
void set_coils(uint8_t step) {
    gpio_put(COIL_A, step_sequence[step][0]);
    gpio_put(COIL_B, step_sequence[step][1]);
    gpio_put(COIL_C, step_sequence[step][2]);
    gpio_put(COIL_D, step_sequence[step][3]);
}

// Function to run the stepper motor for a specified number of steps
int run_motor(int steps, int delay_ms, int direction, int current_step) {
    for (int i = 0; i < steps; i++) {
        // Determine the next step in the sequence
        if (direction == 1) { // Forward
            current_step = (current_step + 1) % 8;
        } else { // Backward
            current_step = (current_step - 1 + 8) % 8;
        }

        // Set the coils to the current step in the sequence
        set_coils(current_step);
        sleep_ms(delay_ms); // Adjust delay for motor speed
    }
 //   set_coils(0); // Turn off coils after movement
    return current_step; // Return the updated step
}

// Function to calibrate the motor by detecting steps per revolution
void calibrate_motor(int *current_position, int *current_step) {
    printf("Starting calibration...\n");
    int delay_ms = 2;
    int step = 0;
    int step_counts[3] = {0, 0, 0}; // Array to store step counts for three revolutions

    // Wait for the first falling edge of the sensor (optical sensor detecting a fork)
    while (gpio_get(OPTO_FORK) == 1) {
        //set_coils(step);
        //step = (step + 1) % 8;
        static int step_index = 0;
        step_index = (step_index + 1) % 8;
        set_coils(step_index);
        sleep_ms(delay_ms);
    }

    // Count steps between falling edges to determine steps per revolution
    for (int i = 0; i < 3; i++) {
        int count = 0;

        // Count steps until sensor is low (falling edge)
        while (gpio_get(OPTO_FORK) == 0) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        // Count steps until sensor goes high again (rising edge)
        while (gpio_get(OPTO_FORK) == 1) {
            set_coils(step);
            step = (step + 1) % 8;
            sleep_ms(delay_ms);
            count++;
        }

        step_counts[i] = count;
    }

    // Calculate the average steps per revolution
    steps_per_revolution = (step_counts[0] + step_counts[1] + step_counts[2]) / 3;
    is_calibrated = true;
    *current_position = 0; // Reset current position after calibration
    *current_step = 0;     // Start from step 0
    printf("Calibration complete. Average steps per revolution: %d\n", steps_per_revolution);
}

// Function to handle the 'status' command, showing current position and sequence step
void print_status(int current_position, int current_step) {
    printf("Motor status:\n");
    printf("  Calibrated: %s\n", is_calibrated ? "Yes" : "No");
    if (is_calibrated) {
        printf("  Steps per revolution: %d\n", steps_per_revolution);
        printf("  Current position (in steps): %d\n", current_position);
        printf("  Current sequence step (0 to 7): %d\n", current_step); // New line showing current step in sequence
    } else {
        printf("  Steps per revolution: not available\n");
    }
}

// Function to run the motor for N times 1/8th of a revolution
void run_command(int n, int *current_position, int *current_step) {
    if (!is_calibrated) {
        printf("Motor not calibrated yet. Please calibrate the motor first.\n");
        return;
    }

    if (n <= 0) {
        printf("Invalid input. Please enter a positive number of 1/8 revolutions.\n");
        return;
    }

    int steps_to_run = steps_per_revolution / 8;  // One step is 1/8 of a revolution
    int total_steps = steps_to_run * n;

    printf("Running motor for %d steps (1/8 revolution %d times).\n", total_steps, n);

    // Run the motor in the forward direction
    *current_step = run_motor(total_steps, 2, 1, *current_step);

    // Update the current position
    *current_position = (*current_position + total_steps) % steps_per_revolution; // Keep it within one revolution
    printf("Updated position: %d steps\n", *current_position);
}

int main() {
    stdio_init_all();

    // Initialize GPIOs as outputs for the stepper motor
    gpio_init(COIL_A);
    gpio_init(COIL_B);
    gpio_init(COIL_C);
    gpio_init(COIL_D);
    gpio_set_dir(COIL_A, GPIO_OUT);
    gpio_set_dir(COIL_B, GPIO_OUT);
    gpio_set_dir(COIL_C, GPIO_OUT);
    gpio_set_dir(COIL_D, GPIO_OUT);

    // Initialize optical sensor pin as input with pull-up
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    // Local variables for position and step
    static int current_position = 0;
    static int current_step = 0;

    // Main program loop
    while (1) {
        printf("\nEnter command (status, calib, or run N): ");
        char command[10];
        int n = 0;

        fgets(command, 10, stdin);
        command[strcspn(command, "\r")] = 0;
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "status") == 0) {
            print_status(current_position, current_step);
        } else if (strcmp(command, "calib") == 0) {
            calibrate_motor(&current_position, &current_step);
        } else if (strncmp(command, "run", 3) == 0) {
            char *endptr;
            n = strtol(command + 4, &endptr, 10);

            if (*endptr != '\0' || n <= 0) {
                n = 8;
            }
            run_command(n, &current_position, &current_step);
        } else {
            printf("Unknown command.\n");
        }
    }
}
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>

#define IN1 2
#define IN2 3
#define IN3 6
#define IN4 13
#define OPTO_FORK 28
#define STEP_DELAY_MS 2

const int half_step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1},
};

int steps_per_revolution = 0;
bool is_calibrated = false;

void set_step() {
    static int step = 0;
    step=(step+1) % 8;
    gpio_put(IN1, half_step_sequence[step][0]);
    gpio_put(IN2, half_step_sequence[step][1]);
    gpio_put(IN3, half_step_sequence[step][2]);
    gpio_put(IN4, half_step_sequence[step][3]);
}

void calibrate_motor() {
    printf("Calibration has started...\n");
    int step_count[3];

    for (int i = 0; i < 3; i++) {
        printf(".......\n");
        int count = 0;

        // Find the first falling edge
        while (gpio_get(OPTO_FORK)) {
            set_step();
            sleep_ms(STEP_DELAY_MS);
            count++;
        }

        // Find rising edge to ensure start of a clean revolution
        count = 0;
        while (!gpio_get(OPTO_FORK)) {
            set_step();
            sleep_ms(STEP_DELAY_MS);
            count++;
        }

        // Measure one full revolution
        while (gpio_get(OPTO_FORK)) {
            set_step();
            sleep_ms(STEP_DELAY_MS);
            count++;
        }

        step_count[i] = count;
        printf("Rotation %d: %d steps\n", i + 1, count);
    }

    // Calculate average steps per revolution
    steps_per_revolution = (step_count[0] + step_count[1] + step_count[2]) / 3;
    is_calibrated = true;

    printf("Calibration has been completed. Steps per revolution: %d\n", steps_per_revolution);
}

void run_motor(int N) {
    // Determine steps for N * (1/8) revolutions. If N == 8, it completes a full revolution.
    int steps_to_run = (steps_per_revolution / 8) * N;

    for (int i = 0; i < steps_to_run; i++) {
        set_step();
        sleep_ms(STEP_DELAY_MS);
    }
}

int main() {
    stdio_init_all();

    gpio_init(IN1);
    gpio_init(IN2);
    gpio_init(IN3);
    gpio_init(IN4);
    gpio_init(OPTO_FORK);

    gpio_set_dir(IN1, GPIO_OUT);
    gpio_set_dir(IN2, GPIO_OUT);
    gpio_set_dir(IN3, GPIO_OUT);
    gpio_set_dir(IN4, GPIO_OUT);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK); // Enable internal pull-up on the optical sensor pin

    char command[20];
    while (true) {
        printf("Please enter command to proceed (status, calib, run N): ");
        fgets(command, sizeof(command), stdin);  // Use fgets to read the full line

        if (strncmp(command, "status", 6) == 0) {
            if (is_calibrated) {
                printf("Calibration has been done. Steps per revolution: %d\n", steps_per_revolution);
            } else {
                printf("Has no been calibrated yet.\n");
            }
        } else if (strncmp(command, "calib", 5) == 0) {
            calibrate_motor();
        } else if (strncmp(command, "run", 3) == 0) {
            int N = 8;  // Default to 8 (full revolution) if no value is provided
            sscanf(command + 4, "%d", &N);
            run_motor(N);
        } else {
            printf("Command not found. Please enter valid command\n");
        }
    }
}
