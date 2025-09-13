/*  Simple HTTP Server Example

    ONLY USE FOR WINDOW.
*/

#include "http_server_app.h"

#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN  (64)

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static const char *TAG_HTTP_SERVER = "HTTP_server_example";
static httpd_handle_t server = NULL;
static httpd_req_t *REQ;

static http_post_callback_t http_post_switch_callback = NULL;
static http_post_callback_t http_post_rgb_callback = NULL;

static http_get_callback_t http_get_env_sensor_callback = NULL;

// Command to embed a picture to Flash.
extern const uint8_t index_html_image_start[] asm("_binary_Image_jpg_start");
extern const uint8_t index_html_image_end[] asm("_binary_Image_jpg_end");

// Command to embed a html file to Flash.
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

/************************************GET_DASHBOARD************************************/
/* An HTTP GET handler */
static esp_err_t dashboard_handler(httpd_req_t *req)
{
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    // const char* resp_str = (const char*) "Test Get request from ESP32";
    // httpd_resp_send(req, resp_str, strlen(resp_str));

    // httpd_resp_set_type(req, "Image/jpg");
    // httpd_resp_send(req, (const char *) index_html_image_start, (index_html_image_end - index_html_image_start));

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *) index_html_start, (index_html_end - index_html_start));
    return ESP_OK;
}

static const httpd_uri_t get_dashboard =
{
    .uri       = "/get_dashboard",
    .method    = HTTP_GET,
    .handler   = dashboard_handler,
    .user_ctx  = NULL
};
/*******************************************END*********************************************/

/***************************************GET_DATA_DHT11**************************************/
/* An HTTP GET handler */
static esp_err_t env_sensor_get_data(httpd_req_t *req)
{
    // char resp_str[50];
    // sprintf(resp_str, "{\"temperature\": \"%d\", \"humidity\": \"%d\"}", 25, 67);
    // httpd_resp_send(req, resp_str, strlen(resp_str));
    REQ = req;
    http_get_env_sensor_callback();
    return ESP_OK;
}

static const httpd_uri_t env_sensor_data =
{
    .uri       = "/env_sensor_data",
    .method    = HTTP_GET,
    .handler   = env_sensor_get_data,
    .user_ctx  = NULL
};
/*******************************************END*********************************************/

/************************************SWITCH_POST_AND_URI***********************************/
/* An HTTP POST handler */
static esp_err_t switch_1_handler(httpd_req_t *req)
{
    char buf[100];

    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    http_post_switch_callback(buf, req->content_len);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t switch_1 = 
{
    .uri       = "/switch_1",
    .method    = HTTP_POST,
    .handler   = switch_1_handler,
    .user_ctx  = NULL
};
/*******************************************END*********************************************/

/**************************************RGB_POST_AND_URI*************************************/
/* An HTTP POST handler */
static esp_err_t rgb_handler(httpd_req_t *req)
{
    char buf[100];

    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    http_post_rgb_callback(buf, req->content_len);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t rgb = 
{
    .uri       = "/rgb",
    .method    = HTTP_POST,
    .handler   = rgb_handler,
    .user_ctx  = NULL
};
/*******************************************END*********************************************/

/*  _ This handler allows the custom error handling functionality to be tested from client side.
    _ For that, when a PUT request 0 is sent to URI /ctrl, the /hello and /echo URIs are unregistered and following
    custom error handler http_404_error_handler() is registered.
    _ Afterwards, when /hello or /echo is requested, this custom error handler is invoked which,
    after sending an error message to client, either closes the underlying socket (when requested URI is /echo)
    or keeps it open (when requested URI is /hello).
    _ This allows the client to infer if the custom error handler is functioning as expected by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/get", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/get URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    else if (strcmp("/post", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/post URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); // Thư viện chuẩn để hỗ trợ chip ESP32 đóng gói bản tin theo chuẩn HTTP.

    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG_HTTP_SERVER, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG_HTTP_SERVER, "Registering URI handlers");
        httpd_register_uri_handler(server, &get_dashboard);
        httpd_register_uri_handler(server, &env_sensor_data);
        httpd_register_uri_handler(server, &switch_1);
        httpd_register_uri_handler(server, &rgb);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else
        ESP_LOGI(TAG_HTTP_SERVER, "Error starting server!");
}

void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}

void send_response(char *data, int len)
{
    httpd_resp_send(REQ, data, len);
}

void http_set_callback_switch(void *cb)
{
    http_post_switch_callback = cb;
}

void http_set_callback_env_sensor(void *cb)
{
    http_get_env_sensor_callback = cb;
}

void http_set_callback_rgb(void *cb)
{
    http_post_rgb_callback = cb;
}
