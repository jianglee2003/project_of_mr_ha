#ifndef _WIFI_STA_SMARTCONFIG_H_
#define _WIFI_STA_SMARTCONFIG_H_

#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_eap_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_mac.h"

#include "nvs_flash.h"

#include "driver/gpio.h"

#define SMARTCONFIG_TAG             "smartconfig"

#define SMARTCONFIG_TYPE            SC_TYPE_ESPTOUCH

#define SMARTCONFIG_INIT_DONE_BIT   BIT0
#define WIFI_INFO_NVS_STORAGE_BIT   BIT1 // Indicate that wifi info already stored in NVS.
#define WIFI_INFO_RECEIVE_BIT       BIT2 // Indicate that the device received wifi info from Smartconfig.
#define WIFI_CONNECTED_BIT          BIT3
#define WIFI_DISCONNECTED_BIT       BIT4
#define ESPTOUCH_DONE_BIT           BIT5 // Use for turn off Smartconfig.

extern EventGroupHandle_t wifi_sta_smartconfig_eventgroup;

// Function pointer to get the SSID and Password.
typedef void (*smartconfig_event_data_t) (char *, char *);
extern smartconfig_event_data_t smartconfig_event_data;

void initialise_wifi(void);

void smartconfig_start(void);
void smartconfig_stop(void);

void wifi_connect(char *ssid, char *password);

void smartconfig_event_data_set_callback(smartconfig_event_data_t cbs);

#endif
