#include <stdio.h>

#include "wifi_sta_smartconfig.h"
#include "http_server_app.h"
#include "environment_sensor.h"
#include "nvs_flash_storage.h"
#include "ledc_pwm.h"
#include "mqtt.h"

#include "driver/gpio.h"

#include "cJSON.h"

#include "esp_log.h"

#define HTTP_DATA "HTTP_Data_Callback"

#define BOOT_BUTTON       GPIO_NUM_0
#define LED_ON_KIT_WEACT  GPIO_NUM_22

// Device led indicator.
#define LED_GREEN_PIN     GPIO_NUM_21
#define LED_RED_PIN       GPIO_NUM_23
#define LED_BLUE_PIN      GPIO_NUM_22

/*************************************NVS_ZONE************************************/
/*  This flag is used for NVS storage.
    Flag = 0: No data are stored in NVS Flash, or the ESP32 is full reset.
    Flag = 1: There are data stored in NVS.
*/
uint8_t wifi_info_flag = 0;

// Handler for NVS Flash storage.
nvs_handle_t my_handle;

// Name space for NVS.
#define NVS_NAME_SPACE              "Name_Space"

// Key for flag and data strings.
#define NVS_FLAG_KEY                "Flag"
#define NVS_SSID_KEY                "SSID"
#define NVS_PASSWORD_KEY            "Password"
/*********************************************************************************/

// Store Wifi Information.
char ssid_str[30] = {0}, password_str[30] = {0};

// Store RGB value.
int red = 0, green = 0, blue = 0, change_flag = 0;

void http_switch_callback(char *data, int len) {
    // Compare data received with the string "ON" or "OFF" for next work.
    if(strcmp(data, "ON") == 0) {
        gpio_set_level(LED_GREEN_PIN, 1);
        ESP_LOGI("Switch Callback", "%s", data);
    }
    else if(strcmp(data, "OFF") == 0) {
        gpio_set_level(LED_GREEN_PIN, 0);
        ESP_LOGI("Switch Callback", "%s", data);
    }
}

void http_env_sensor_callback(void) {
    float temp = 0, humd = 0;
    // Read temperature, humidity, and light intensity.
    temp =  read_temperature();
    humd = read_humidity();

    char resp_str[50];
    sprintf(resp_str, "{\"temperature\": \"%.2f\", \"humidity\": \"%.2f\"}", temp, humd);
    send_response(resp_str, strlen(resp_str));
    // httpd_resp_send(req, resp_str, strlen(resp_str));
}

void http_rgb_callback(char *data, int len) {
    ESP_LOGI("RGB Callback", "Value: %s", data);
    sscanf(data, "%2x%2x%2x", &red, &green, &blue);
    change_flag = 1;
}

void set_rgb_color(void * parameter) {
    for(;;) {
        if(change_flag == 1) {
            // Map each channel
            int r_percent = (red * 100) / 255;
            int g_percent = (green * 100) / 255;
            int b_percent = (blue * 100) / 255;

            set_duty_pulse(LOW_SPEED, LEDC_CHANNEL_0, RES_10_BIT, r_percent);
            set_duty_pulse(LOW_SPEED, LEDC_CHANNEL_1, RES_10_BIT, g_percent);
            set_duty_pulse(LOW_SPEED, LEDC_CHANNEL_2, RES_10_BIT, b_percent);

            change_flag = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void process_command_from_mqtt(esp_mqtt_client_handle_t client, char* topic, char *data, int data_length) {
  cJSON *json = cJSON_Parse(data);
  if (json == NULL) {
      ESP_LOGE("JSON", "Error parsing JSON data, data received: %s", data);
      return;
  }

  char topic_str[20];

  // status
  cJSON *status = cJSON_GetObjectItem(json, "status");
  if (strstr(status->valuestring, "on") != NULL) {
    gpio_set_level(LED_GREEN_PIN, 1);
    ESP_LOGI("Switch", "command: %s", status->valuestring);
    sprintf(topic_str, "%s", "/led/");
  } 
  
  else if (strstr(status->valuestring, "off") != NULL) {
    gpio_set_level(LED_GREEN_PIN, 0);
    ESP_LOGI("Switch", "command: %s", status->valuestring);
    sprintf(topic_str, "%s", "/led/");
  } 
  
  else {
    ESP_LOGI("RGB Callback", "Value: %s", status->valuestring);
    sscanf(status->valuestring, "%2x%2x%2x", &red, &green, &blue);
    change_flag = 1;
    sprintf(topic_str, "%s", "/rgb/");
  }

  // Response to MQTT.
  char response[50];
  sprintf(response, "{\"status\":\"%s\",\"type\":\"response\"}", status->valuestring);
  esp_mqtt_client_publish(client, topic_str, response, 0, 0, 1);
}

void push_env_data_to_mqtt_task(void *parameter){
  for(;;) {
    xEventGroupWaitBits(mqtt_eventgroup, MQTT_CONNECTED_TO_TOPIC_BIT, pdFALSE, pdFALSE, portMAX_DELAY); 

    float temp = 0, humd = 0;
    temp =  read_temperature();
    humd = read_humidity();

    char data[40];
    sprintf(data, "{\"temp\":%.2f,\"humd\":%.2f}", temp, humd);
    int msg_id = esp_mqtt_client_publish(global_client, "/env_data/", data, 0, 0, 1);
    if (msg_id < 0) {
        ESP_LOGE(TAG_MQTT, "Publish failed, client not ready");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void smartconfig_led_indicator(void *parameter) {
  for(;;) {
    gpio_set_level(LED_RED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(LED_RED_PIN, 0);

    EventBits_t bit = xEventGroupGetBits(wifi_sta_smartconfig_eventgroup);
    if(bit & SMARTCONFIG_INIT_DONE_BIT) {
      vTaskDelay(pdMS_TO_TICKS(1000)); 
    }

    if(bit & WIFI_INFO_RECEIVE_BIT) {
      xEventGroupClearBits(wifi_sta_smartconfig_eventgroup, WIFI_INFO_RECEIVE_BIT);
      vTaskDelete(NULL); 
    }

    vTaskDelay(pdMS_TO_TICKS(150));  
  }
}

void wifi_led_indicator(void * parameter) {
  for(;;) {
    gpio_set_level(LED_BLUE_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(LED_BLUE_PIN, 0);

    EventBits_t bit = xEventGroupGetBits(wifi_sta_smartconfig_eventgroup);
    if(bit & WIFI_CONNECTED_BIT) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelay(pdMS_TO_TICKS(150));
  }
}

void http_server_and_mqtt_client_initialise(void * parameter) {
  xEventGroupWaitBits(wifi_sta_smartconfig_eventgroup, 
                      WIFI_INFO_RECEIVE_BIT | WIFI_INFO_NVS_STORAGE_BIT, 
                      pdFALSE, 
                      pdFALSE, 
                      portMAX_DELAY); 

  xTaskCreate(wifi_led_indicator, "wifi_led_indicator", 2048, NULL, 6, NULL);

  // Connect to Wifi.
  wifi_connect(ssid_str, password_str);
  xEventGroupWaitBits(wifi_sta_smartconfig_eventgroup, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);  

  // Web server.
  start_webserver();
  http_set_callback_switch(http_switch_callback);
  http_set_callback_env_sensor(http_env_sensor_callback);
  http_set_callback_rgb(http_rgb_callback);

  // MQTT Client.
  mqtt_app_start(BROKER_URL);

  // Set the callback function for receiving data.
  set_callback_get_data_mqtt(process_command_from_mqtt);

  // Sequently send env data to MQTT.
  xTaskCreate(push_env_data_to_mqtt_task, "push_env_data_to_mqtt_task", 4096, NULL, 4, NULL);

  vTaskDelete(NULL);
}

void smartconfig_task(void * parm) {
  ESP_ERROR_CHECK( esp_smartconfig_set_type(SMARTCONFIG_TYPE) );
  smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );

  xTaskCreate(http_server_and_mqtt_client_initialise, "http_server_and_mqtt_client_initialise", 4096, NULL, 4, NULL);

  // If this BIT is set, that means the ESP32 is connected to AP, then response info to the Moblie App.
  xEventGroupWaitBits(wifi_sta_smartconfig_eventgroup, ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
  ESP_LOGW(SMARTCONFIG_TAG, "smartconfig over");
  esp_smartconfig_stop();

  // Set the flag to 1, that means new data will be stored in NVS.
  wifi_info_flag = 1; 

  // Write flag value to NVS.
  nvs_write_value_uint8(NVS_NAME_SPACE, my_handle, NVS_FLAG_KEY, wifi_info_flag);

  // Write all Informations about SSID, Password, Topics to NVS.
  nvs_write_string(NVS_NAME_SPACE, my_handle, NVS_SSID_KEY, ssid_str);
  nvs_write_string(NVS_NAME_SPACE, my_handle, NVS_PASSWORD_KEY, password_str);

  vTaskDelete(NULL);
}

// Callback function for smartconfig - get data event.
void get_wifi_info_from_smartconfig_callback(char *ssid, char *password) {
  memcpy(ssid_str, ssid, sizeof(ssid_str));
  memcpy(password_str, password, sizeof(password_str));
}

void reset_device_by_button_task(void *paramter) {
  for(;;) {
    if(gpio_get_level(BOOT_BUTTON) == 0) {
      ESP_LOGW("RESET", "release button for normal reset, keep holding button in 5 second for full reset.");
      vTaskDelay(pdMS_TO_TICKS(100));

      // Normal reset.
      if(gpio_get_level(BOOT_BUTTON) == 1) {
        esp_restart();
      } 
      
      // Full reset.
      else if(gpio_get_level(BOOT_BUTTON) == 0) {
        vTaskDelay(pdMS_TO_TICKS(5000));

        if(gpio_get_level(BOOT_BUTTON) == 0) {
          ESP_LOGW("RESET", "Full reset for the ESP32.");
          nvs_flash_erase();
          vTaskDelay(500 / portTICK_PERIOD_MS);
          esp_restart();
        }
      }
    } 
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void app_main(void) {
    // Initalize NVS Flash for storing data in case power down.
    nvs_flash_storage_init();

    gpio_reset_pin(BOOT_BUTTON);
    gpio_reset_pin(LED_BLUE_PIN);
    gpio_reset_pin(LED_RED_PIN);
    gpio_reset_pin(LED_GREEN_PIN);

    gpio_set_direction(LED_BLUE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GREEN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GREEN_PIN, 0);

    gpio_set_direction(BOOT_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOOT_BUTTON, GPIO_PULLDOWN_ONLY);

    i2c_master_init();
    i2c_addr_check();
    
    ahtxx_init();

    ledc_timer_init(LOW_SPEED, RES_10_BIT, LEDC_TIMER_0, LEDC_FREQUENCY, CHANNEL_0, LEDC_OUTPUT_IO_CHANNEL_0);
    ledc_timer_init(LOW_SPEED, RES_10_BIT, LEDC_TIMER_0, LEDC_FREQUENCY, CHANNEL_1, LEDC_OUTPUT_IO_CHANNEL_1);
    ledc_timer_init(LOW_SPEED, RES_10_BIT, LEDC_TIMER_0, LEDC_FREQUENCY, CHANNEL_2, LEDC_OUTPUT_IO_CHANNEL_2);
    xTaskCreate(set_rgb_color, "set_rgb_color", 4096, NULL, 4, NULL);

    initialise_wifi();

    xTaskCreate(reset_device_by_button_task, "reset_device_by_button_task", 2048, NULL, 4, NULL);

    // Read the flag value from NVS.
    nvs_read_value_uint8(NVS_NAME_SPACE, my_handle, NVS_FLAG_KEY, &wifi_info_flag);

    // No wifi info are stored in NVS.
    if(wifi_info_flag == 0) {
        ESP_LOGW("Flag", "The device has not configured Wifi info yet.");
        xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
        xTaskCreate(smartconfig_led_indicator, "smartconfig_led_indicator", 2048, NULL, 4, NULL);
        smartconfig_event_data_set_callback(get_wifi_info_from_smartconfig_callback);
    }

    // Wifi info already stored in NVS.
    else if(wifi_info_flag == 1) {
        ESP_LOGI("Flag", "read wifi info from NVS.");
        nvs_read_string(NVS_NAME_SPACE, my_handle, NVS_SSID_KEY, ssid_str, sizeof(ssid_str));
        nvs_read_string(NVS_NAME_SPACE, my_handle, NVS_PASSWORD_KEY, password_str, sizeof(password_str));

        xEventGroupSetBits(wifi_sta_smartconfig_eventgroup, WIFI_INFO_NVS_STORAGE_BIT);
        xTaskCreate(http_server_and_mqtt_client_initialise, "http_server_and_mqtt_client_initialise", 4096, NULL, 4, NULL);
    }
}