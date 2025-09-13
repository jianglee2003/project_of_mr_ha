// This library is only used for Low Speed Mode.

#ifndef _LEDC_PWM_H_
#define _LEDC_PWM_H_

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "rom/ets_sys.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#define LEDC_PWM_TAG "LEDC_PWM"

//////// Khai báo để ra xung PWM. với xung 38Khz thì phải sử dụng LEDC timer dưới 10 bit
#define LEDC_TIMER                  LEDC_TIMER_0
#define LEDC_MODE                   LOW_SPEED
#define LEDC_OUTPUT_IO_CHANNEL_0    GPIO_NUM_18 // RED 
#define LEDC_OUTPUT_IO_CHANNEL_1    GPIO_NUM_5 // Green
#define LEDC_OUTPUT_IO_CHANNEL_2    GPIO_NUM_19 // Blue
#define LEDC_CHANNEL                CHANNEL_0
#define LEDC_DUTY_RES               RES_10_BIT // Set duty resolution to 10 bits
#define LEDC_FREQUENCY              (38000) // Frequency in Hertz. Set frequency at 38 kHz

typedef enum {
    LOW_SPEED = LEDC_LOW_SPEED_MODE,
    HIGH_SPEED = LEDC_HIGH_SPEED_MODE,
} pwm_ledc_mode_t; // Found in "ledc_types.h", enum "ledc_mode_t".

typedef enum {
    CHANNEL_0 = LEDC_CHANNEL_0,
    CHANNEL_1 = LEDC_CHANNEL_1,
    CHANNEL_2 = LEDC_CHANNEL_2,
    CHANNEL_3 = LEDC_CHANNEL_3,
    CHANNEL_4 = LEDC_CHANNEL_4,
    CHANNEL_5 = LEDC_CHANNEL_5,
    CHANNEL_6 = LEDC_CHANNEL_6,
    CHANNEL_7 = LEDC_CHANNEL_7,
    CHANNEL_8 = LEDC_CHANNEL_MAX,
} channel_t; // Found in "ledc_types.h", enum "ledc_channel_t".

typedef enum {
    RES_1_BIT = LEDC_TIMER_1_BIT,
    RES_2_BIT = LEDC_TIMER_2_BIT,
    RES_3_BIT = LEDC_TIMER_3_BIT,
    RES_4_BIT = LEDC_TIMER_4_BIT,
    RES_5_BIT = LEDC_TIMER_5_BIT,
    RES_6_BIT = LEDC_TIMER_6_BIT,
    RES_7_BIT = LEDC_TIMER_7_BIT,
    RES_8_BIT = LEDC_TIMER_8_BIT,
    RES_9_BIT = LEDC_TIMER_9_BIT,
    RES_10_BIT = LEDC_TIMER_10_BIT,
    RES_11_BIT = LEDC_TIMER_11_BIT,
    RES_12_BIT = LEDC_TIMER_12_BIT,
    RES_13_BIT = LEDC_TIMER_13_BIT,
    RES_14_BIT = LEDC_TIMER_14_BIT,
    RES_15_BIT = LEDC_TIMER_15_BIT,
    RES_16_BIT = LEDC_TIMER_16_BIT,
    RES_17_BIT = LEDC_TIMER_17_BIT,
    RES_18_BIT = LEDC_TIMER_18_BIT,
    RES_19_BIT = LEDC_TIMER_19_BIT,
    RES_20_BIT = LEDC_TIMER_20_BIT,
    RES_MAX = LEDC_TIMER_BIT_MAX,
} resolution_t; // Found in "ledc_types.h", enum "ledc_timer_bit_t".

// Initialize all parameter for LEDC PWM.
void ledc_timer_init(pwm_ledc_mode_t speed_mode, resolution_t resolution,
                    ledc_timer_t timer, uint32_t freq,
                    channel_t channel, int gpio_num);

// Generate PWM Pulse with input duty_val.
// Duty Pulse from 0 - 100 %.
void set_duty_pulse(pwm_ledc_mode_t speed_mode, channel_t channel, resolution_t resolution, uint16_t duty_val);

#endif
