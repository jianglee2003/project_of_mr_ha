#include "ledc_pwm.h"

/* Warning:
 * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2, ESP32P4 targets,
 * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
 * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
 */
void ledc_timer_init(pwm_ledc_mode_t speed_mode, resolution_t resolution,
                    ledc_timer_t timer, uint32_t freq,
                    channel_t channel, int gpio_num)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = 
    {
        .speed_mode       = speed_mode, // LEDC speed mode.
        .duty_resolution  = resolution, // LEDC channel duty resolution.
        .timer_num        = timer, // The timer source of channel.
        .freq_hz          = freq,  // Set output frequency for PWM Pulse (Hz).
        .clk_cfg          = LEDC_AUTO_CLK   // LEDC source clock will be automatically selected based on the giving resolution and duty parameter when init the timer.
                                            // Clock source for all timers must be the same one.
    };

    // Initialize the timer for Led_Control_PWM
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    // Below are important parameters, the other one can be ignored.
    ledc_channel_config_t ledc_channel = 
    {
        .speed_mode     = speed_mode,
        .channel        = channel, // LEDC channel
        .timer_sel      = timer,
        .intr_type      = LEDC_INTR_DISABLE, // Configure interrupt, enable or disable Interrupt.
        .gpio_num       = gpio_num, // The LEDC output gpio_num.
        .duty           = 0, // Set inital duty to 0%
        .hpoint         = 0 // LEDC channel hpoint value, the range is [0, (2**duty_resolution)-1]

        /*  EXPLAIN:
            hpoint: Hiểu đơn giản là tgian delay giữa mỗi lần phát xung, chỉ nên sử dụng khi cần tinh chỉnh pha cho xung PWM
                    Đơn vị: timer ticks
                    hpoint = 0: The PWM signal goes high at the very start of the PWM period (phase offset is 0).
                    hpoint > 0: The PWM signal will delay going high by the number of ticks specified in hpoint.
                                This means the pulse will start later in the cycle, essentially shifting the phase of the signal.
        */
    };

    // Initialize channel for led
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

// Generate PWM Pulse with input duty_val.
// Duty Pulse from 0 - 100 %.
void set_duty_pulse(pwm_ledc_mode_t speed_mode, channel_t channel, resolution_t resolution, uint16_t duty_val)
{
    if(resolution < 21) 
    {
        // Clamp duty_val (percentage 0–100)
        // if (duty_val < 0) duty_val = 0;
        if (duty_val > 100) duty_val = 100;

        // the power of 2 with resolution is the resolution range of the PWM Pulse.
        // In C, the power of 2 with a given exponent n is computed by left-shifting the number 1 by n bits.
        int duty_100 = (1 << resolution) - 1;

        //do LEDC_TIMER_10_BIT nen duty 100% khi duty_val o gia tri 1023
        int duty = (duty_val * duty_100 )/100;
        ledc_set_duty(speed_mode, channel, duty);

        // Update duty to apply the new value
        /*  Call this function to activate the LEDC updated parameters.
            After ledc_set_duty, we need to call this function to update the settings.
            And the new LEDC parameters don't take effect until the next PWM cycle.
        */
        ledc_update_duty(speed_mode, channel);
        ESP_LOGI(LEDC_PWM_TAG, "channel: %d, duty: %d%% -> raw %d (max %d)", channel, duty_val, duty, duty_100);
    }
    else
        ESP_LOGE(LEDC_PWM_TAG, "Resolution is out of range.");
}

