#include "wifi_sta_smartconfig.h"

EventGroupHandle_t wifi_sta_smartconfig_eventgroup = NULL;

// Function pointer declaration.
smartconfig_event_data_t smartconfig_event_data = NULL;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(SMARTCONFIG_TAG, "Starting Wifi Peripheral ...");
    } 
    
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupSetBits(wifi_sta_smartconfig_eventgroup, WIFI_DISCONNECTED_BIT);
        xEventGroupClearBits(wifi_sta_smartconfig_eventgroup, WIFI_CONNECTED_BIT);
    } 
    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_sta_smartconfig_eventgroup, WIFI_CONNECTED_BIT);
        xEventGroupClearBits(wifi_sta_smartconfig_eventgroup, WIFI_DISCONNECTED_BIT);

        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        uint8_t primary;
        wifi_second_chan_t second;
        esp_wifi_get_channel(&primary, &second);

        // Get the config to read the SSID and password
        wifi_config_t current_conf;
        esp_wifi_get_config(WIFI_IF_STA, &current_conf);

        ESP_LOGI(SMARTCONFIG_TAG, "Connected to AP, SSID: %s, Password: %s, got IP: " IPSTR ", channel: %d", 
                                    current_conf.sta.ssid, 
                                    current_conf.sta.password, 
                                    IP2STR(&event->ip_info.ip), 
                                    primary);
    }

    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(SMARTCONFIG_TAG, "Scan done");
        xEventGroupSetBits(wifi_sta_smartconfig_eventgroup, SMARTCONFIG_INIT_DONE_BIT);
    } 
    
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(SMARTCONFIG_TAG, "Found channel");
    }
    
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(SMARTCONFIG_TAG, "Got SSID and password");
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        // Set callback.
        smartconfig_event_data((char *) evt->ssid, (char *) evt->password);
        xEventGroupSetBits(wifi_sta_smartconfig_eventgroup, WIFI_INFO_RECEIVE_BIT);
    } 
    
    // ESP32 successfully send response to Mobile App.
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(wifi_sta_smartconfig_eventgroup, ESPTOUCH_DONE_BIT);
    }
}

// initialize wifi peripheral.
void initialise_wifi(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    wifi_sta_smartconfig_eventgroup = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void smartconfig_start(void) {
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SMARTCONFIG_TYPE) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
}

void smartconfig_stop(void) {
    ESP_LOGW(SMARTCONFIG_TAG, "smartconfig over");
    esp_smartconfig_stop();
}

void wifi_connect(char *ssid, char *password) {
    wifi_config_t wifi_config;

    // Clear trash value in struct before pass wifi info to.
    bzero(&wifi_config, sizeof(wifi_config_t));

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_LOGI(SMARTCONFIG_TAG, "SSID : %s", ssid);
    ESP_LOGI(SMARTCONFIG_TAG, "PASSWORD : %s", password);

    // Connect to Wifi, with SSID and Password.
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    esp_wifi_connect();
}

void smartconfig_event_data_set_callback(smartconfig_event_data_t cbs) {
    smartconfig_event_data = cbs;
}
