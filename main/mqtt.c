#include "mqtt.h"

EventGroupHandle_t mqtt_eventgroup = NULL;

static mqtt_getdata_callback mqtt_callback_get_data = NULL;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", message, error_code);
    }
}

esp_mqtt_event_handle_t global_event = NULL;
esp_mqtt_client_handle_t global_client = NULL;

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base = %s, event_id = %" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    int msg_id; // Message ID.

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED: {
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");

            global_event = event;
            global_client = event->client;

            char topic[15];

            sprintf(topic, "%s", "/led/");
            msg_id = esp_mqtt_client_subscribe(global_client, topic, 0);
            if(msg_id < 0) {
                ESP_LOGE(TAG_MQTT, "subscribe fail, topic: %s", topic);
            }

            sprintf(topic, "%s", "/rgb/");
            msg_id = esp_mqtt_client_subscribe(global_client, topic, 0);
            if(msg_id < 0) {
                ESP_LOGE(TAG_MQTT, "subscribe fail, topic: %s", topic);
            }

            sprintf(topic, "%s", "/env_data/");
            msg_id = esp_mqtt_client_subscribe(global_client, topic, 0);
            if(msg_id < 0) {
                ESP_LOGE(TAG_MQTT, "subscribe fail, topic: %s", topic);
            }

            xEventGroupSetBits(mqtt_eventgroup, MQTT_CONNECTED_TO_TOPIC_BIT);
            xEventGroupClearBits(mqtt_eventgroup, MQTT_DISCONNECTED_BIT);
            break;
        }
        case MQTT_EVENT_DISCONNECTED: {
            ESP_LOGW(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
            xEventGroupSetBits(mqtt_eventgroup, MQTT_DISCONNECTED_BIT);
            xEventGroupClearBits(mqtt_eventgroup, MQTT_CONNECTED_TO_TOPIC_BIT);
            break;
        }
        case MQTT_EVENT_SUBSCRIBED: {
            ESP_LOGI(TAG_MQTT,
                    "MQTT_EVENT_SUBSCRIBED, topic: %s, msg_id = %d",
                    event->topic ? event->topic : "(null)",
                    event->msg_id);
            break;
        }
        case MQTT_EVENT_UNSUBSCRIBED: {
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        }
        case MQTT_EVENT_PUBLISHED: {
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        }
        case MQTT_EVENT_DATA: {
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA"); 
            // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            // printf("DATA=%.*s\r\n", event->data_len, event->data);

            // Simple led topic.
            if(strstr(event->topic, "/led/") != NULL || strstr(event->topic, "/rgb/") != NULL) {
                if(strstr(event->data, "request") != NULL) {
                    mqtt_callback_get_data(global_client, event->topic, event->data, event->data_len);
                }
            }
            break;
        }
        case MQTT_EVENT_ERROR: {
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG_MQTT, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        }
        default: {
            ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
            break;
        }
    }
}

void mqtt_app_start(char *broker) {
    mqtt_eventgroup = xEventGroupCreate();

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker,
        // .broker.verification.certificate = (const char *)server_cert_pem_start,
        // .credentials = {
        //   .authentication = {
        //     .certificate = (const char *)client_cert_pem_start,
        //     .key = (const char *)client_key_pem_start,
        //   },
        // }
    };

    ESP_LOGI(TAG_MQTT, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    ESP_LOGI(TAG_MQTT, "INIT MQTT OK");
}

void set_callback_get_data_mqtt(void *cb) {
    mqtt_callback_get_data = cb;
}
