/*  Simple HTTP Server Example

    ONLY USE FOR WINDOW.
*/

#ifndef __HTTP_SERVER_APP_H_
#define __HTTP_SERVER_APP_H_

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/param.h>
#include "nvs_flash.h"

#include "esp_netif.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_check.h"
#include <esp_wifi.h>
#include <esp_system.h>
#include "esp_eth.h"
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// Callback Handler for post and get data.
typedef void (*http_post_callback_t) (char *data, int len);
typedef void (*http_get_callback_t) (void);

void start_webserver(void);
void stop_webserver(void);

void send_response(char *data, int len);

void http_set_callback_switch(void *cb);
void http_set_callback_env_sensor(void *cb);
void http_set_callback_slider(void *cb);
void http_set_callback_rgb(void *cb);

#endif
