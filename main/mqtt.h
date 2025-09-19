#ifndef __MQTT_H
#define __MQTT_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_mac.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"

#include "mqtt_client.h"

#define TAG_MQTT "mqtt_lib"

// khai báo MQTT broker.
#define BROKER_URL  "mqtt://broker.emqx.io:1883" 

#define MQTT_DATA_RECEIVED_BIT          BIT0
#define MQTT_CONNECTED_TO_TOPIC_BIT     BIT1
#define MQTT_DISCONNECTED_BIT           BIT2
#define FULL_RESET_BIT                  BIT3

extern EventGroupHandle_t mqtt_eventgroup;

extern esp_mqtt_client_handle_t global_client;

typedef void (*mqtt_getdata_callback) (esp_mqtt_client_handle_t client, char *topic, char* data, int data_length); 

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_app_start(char *broker);

void set_callback_get_data_mqtt(void *cb);

#endif

