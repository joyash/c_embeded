

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdbool.h>
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"



#define motor_pina 2
#define motor_pinb 3
#define motor_pinc 6
#define motor_pind 13
#define opt_fork 28
#define sensing_buz 27
#define press_for_calib 7
#define press_for_dispens 9
#define light_code 20



bool item_dropped = false;
bool no_item = false;
int item_is_counting = 0;
int rounding_counting = 0;
int step_counting = 0;
int stepToMiddle = 0;
bool does_it_calibrated = false;
bool openfork = false;




const __uint8_t step_values[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};



void putting_motor_coil(uint8_t step) {
    gpio_put(motor_pina, step_values[step][0]);
    gpio_put(motor_pinb, step_values[step][1]);
    gpio_put(motor_pinc, step_values[step][2]);
    gpio_put(motor_pind, step_values[step][3]);
}
void create_led(uint led){
    uint period, freq;
    period = 1000;
    freq = 125;
    uint slice_num = pwm_gpio_to_slice_num(led);
    pwm_config config_led = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config_led,freq);
    pwm_config_set_wrap(&config_led,period-1);
    pwm_set_enabled(slice_num, led);
    pwm_init(slice_num,&config_led,true);
    gpio_set_function(led,GPIO_FUNC_PWM);
}

void change_light(uint led, int bright){
    uint slice, channel;
    slice = pwm_gpio_to_slice_num(led);
    channel = pwm_gpio_to_channel(led);
    pwm_set_chan_level(slice,channel,bright);
}

void for_use_motor(int steps, int direction, int speed_ms) {
    static int current_step = 0;
    for (int i = 0; i < steps; i++) {
        current_step = (direction == 1)
            ? (current_step + 1) % 8
            : (current_step - 1 + 8) % 8;
        putting_motor_coil(current_step);
        sleep_ms(speed_ms);
    }
}


bool check_button_pressed(int pin) {
    if (gpio_get(pin) == 0) {
        sleep_ms(50);
        if (gpio_get(pin) == 0) {
            return true;
        }
    }
    return false;
}
void generic_irq_callback(uint gpio, uint32_t event_mask) {
    static uint32_t last_irq_time_opto = 0;
    static uint32_t last_irq_time_piezo = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());


    if (gpio == opt_fork && (event_mask & GPIO_IRQ_EDGE_FALL)) {
        if (current_time - last_irq_time_opto > 50) {
            openfork = true;
            last_irq_time_opto = current_time;
        }
    }


    if (gpio == sensing_buz && (event_mask & GPIO_IRQ_EDGE_FALL)) {
        if (current_time - last_irq_time_piezo > 85) {
            item_dropped = true;
            last_irq_time_piezo = current_time;
        }
    }
}




void pin_initialization() {
    stdio_init_all();

    gpio_init(motor_pina);
    gpio_set_dir(motor_pina, GPIO_OUT);

    gpio_init(motor_pinb);
    gpio_set_dir(motor_pinb, GPIO_OUT);

    gpio_init(motor_pinc);
    gpio_set_dir(motor_pinc, GPIO_OUT);

    gpio_init(motor_pind);
    gpio_set_dir(motor_pind, GPIO_OUT);

    gpio_init(opt_fork);
    gpio_pull_up(opt_fork);

    gpio_init(sensing_buz);
    gpio_set_dir(sensing_buz, GPIO_IN);
    gpio_pull_up(sensing_buz);

    gpio_init(press_for_calib);
    gpio_set_dir(press_for_calib, GPIO_IN);
    gpio_pull_up(press_for_calib);

    gpio_init(press_for_dispens);
    gpio_set_dir(press_for_dispens, GPIO_IN);
    gpio_pull_up(press_for_dispens);

    gpio_init(light_code);
    gpio_set_dir(light_code, GPIO_OUT);


    gpio_set_irq_enabled_with_callback(opt_fork, GPIO_IRQ_EDGE_FALL, true, &generic_irq_callback);

    gpio_set_irq_enabled_with_callback(sensing_buz, GPIO_IRQ_EDGE_FALL, true, &generic_irq_callback);
}


void blink_led(int times) {
    for (int i = 0; i < times; i++) {
        gpio_put(light_code, 1);
        sleep_ms(200);
        gpio_put(light_code, 0);
        sleep_ms(200);
    }
}

void motor_calibration(int *steps_per_revolution, int *step_to_middle, bool *is_calibrated) {
    printf("\nThe program is about to starts calibration...\n");

    int step_calib = 0;
    int late_count_in_ms = 1;
    int steps_inside_fork[3] = {0, 0, 0};
    int steps_outside_fork[3] = {0, 0, 0};


    for (int i = 0; i < *steps_per_revolution; i++) {
        putting_motor_coil(step_calib);
        step_calib = (step_calib + 1) % 8;
        sleep_ms(late_count_in_ms);
    }


    while (gpio_get(opt_fork) == 1) {
        putting_motor_coil(step_calib);
        step_calib = (step_calib + 1) % 8;
        sleep_ms(late_count_in_ms);
    }
    sleep_ms(10);

    for (int i = 0; i < 3; i++) {
        int inside_count = 0;
        int outside_count = 0;

        while (gpio_get(opt_fork) == 0) {
            putting_motor_coil(step_calib);
            step_calib = (step_calib + 1) % 8;
            sleep_ms(late_count_in_ms);
            inside_count++;
        }

        while (gpio_get(opt_fork) == 1) {
            putting_motor_coil(step_calib);
            step_calib = (step_calib + 1) % 8;
            sleep_ms(late_count_in_ms);
            outside_count++;
        }

        steps_inside_fork[i] = inside_count;
        steps_outside_fork[i] = outside_count;
        sleep_ms(10);
    }

    *steps_per_revolution = (steps_inside_fork[0] + steps_inside_fork[1] + steps_inside_fork[2] +
                             steps_outside_fork[0] + steps_outside_fork[1] + steps_outside_fork[2]) / 3;
    *step_to_middle = (steps_inside_fork[0] + steps_inside_fork[1] + steps_inside_fork[2]) / 6;

    *is_calibrated = true;

    printf("\nThe program completed the calibration.\n");
    printf(" - Total steps in one round: %d\n", *steps_per_revolution);
    printf(" - Steps to Midpoint: %d\n", *step_to_middle);

    for_use_motor(*step_to_middle, 1, 2);
    printf(" Now the motor is in opto fork centerpoint. \n");
}




void dispense_pills(int steps_per_segment) {
    item_dropped = false;
    no_item = false;
    rounding_counting = 0;
    item_is_counting = 0;

    while (rounding_counting < 7 && !no_item) {
        item_dropped = false;


        for_use_motor(steps_per_segment, 1, 2);
        sleep_ms(85);
        if (item_dropped) {
            item_is_counting++;
            printf("One pill detetected by sensor . Total numbers is : %d\n", item_is_counting);
        } else {
            blink_led(5);
            printf("The sensor doesn't sense any pill.\n");
        }

        rounding_counting++;
        sleep_ms(5000);


        if (item_is_counting >= 7) {
            no_item = true;
            printf("Dispenser compartment is empty.\n");
        }
    }

    printf(" !!! --- This round of dispensing is complete. Total number of pills disposed  : %d\n", item_is_counting);
}

int main() {
    pin_initialization();

    int steps_per_revolution = 0;
    int step_to_middle = 0;
    bool is_calibrated = false;
    absolute_time_t timeout = make_timeout_time_ms(0);

    while (1) {
        if (openfork) {
            openfork = false;

        }

        if (item_dropped) {
            item_dropped = false;

        }
        printf(" !!! --- Please press button to start calibration --- !!! \n");


        while (!check_button_pressed(press_for_calib)) {
            if (time_reached(timeout)) {
                gpio_put(light_code, !gpio_get(light_code));
                timeout = make_timeout_time_ms(500);
            }

        }


        motor_calibration(&steps_per_revolution, &step_to_middle, &is_calibrated);


        gpio_put(light_code, 1);


        printf(" !!! --- Please press button to start dispensing --- !!!\n");
        while (!check_button_pressed(press_for_dispens)) {

            sleep_ms(10);
        }
        gpio_put(light_code, 0);



        int steps_per_segment = steps_per_revolution / 8;
        dispense_pills(steps_per_segment);


        gpio_put(light_code, 0);
        item_is_counting = 0;
        rounding_counting = 0;
        is_calibrated = false;
    }
}

