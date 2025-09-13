#include "nvs_flash_storage.h"

void nvs_write_value_int32(const char *name_space, nvs_handle_t handle, const char *key, int32_t value)
{
    // Open the NVS partition with name_space specified. Open mode is read and write.
    esp_err_t err = nvs_open(name_space, NVS_READWRITE, &handle);
    if (err == ESP_OK)
    {
        err = nvs_set_i32(handle, key, value); // Write a value to NVSNVS
        if (err == ESP_OK)
        {
            nvs_commit(handle); // Save the changes in NVS.
            ESP_LOGI(NVS_TAG, "Variable stored successfully, the value is: %ld!", value);
        } else {
            ESP_LOGE(NVS_TAG, "Failed to store variable. Error: %s", esp_err_to_name(err));
        }
        nvs_close(handle); // Close NVS handle
    } else {
        ESP_LOGE(NVS_TAG, "Failed to open NVS handle from Write_Uint8_Value function: %s.", esp_err_to_name(err));
    }
}

void nvs_read_value_int32(const char *name_space, nvs_handle_t handle, const char *key, int32_t *value)
{
    esp_err_t err = nvs_open(name_space, NVS_READONLY, &handle);
    if (err == ESP_OK)
    {
        err = nvs_get_i32(handle, key, value); // Read a value and assign it to value pointer.
        switch (err) 
        {
            case ESP_OK:
                ESP_LOGI(NVS_TAG, "Retrieved value: %ld", *value);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGW(NVS_TAG, "The variable is not initialized yet.");
                break;
            default:
                ESP_LOGE(NVS_TAG, "Error reading variable.");
        }
        nvs_close(handle); // Close NVS handle
    } else {
        ESP_LOGE(NVS_TAG, "Failed to open NVS handle from Read_Uint32_Value function: %s.", esp_err_to_name(err));
    }
}

void nvs_write_value_uint8(const char *name_space, nvs_handle_t handle, const char *key, uint8_t value)
{
    esp_err_t err = nvs_open(name_space, NVS_READWRITE, &handle);
    if(err == ESP_OK)
    {
        err = nvs_set_u8(handle, key, value);
        if (err == ESP_OK) 
        {
            ESP_LOGI(NVS_TAG, "Value %u written successfully.", value);
            nvs_commit(handle); // Save the changes
        } else {
            ESP_LOGE(NVS_TAG, "Failed to write value. Error: %s", esp_err_to_name(err));
        }
        nvs_close(handle); // Close NVS handle
    } else {
        ESP_LOGE(NVS_TAG, "Failed to open NVS handle from Write_Uint8_Value function: %s.", esp_err_to_name(err));
    }
}

void nvs_read_value_uint8(const char *name_space, nvs_handle_t handle, const char *key, uint8_t *value)
{
    esp_err_t err = nvs_open(name_space, NVS_READONLY, &handle);
    if(err == ESP_OK)
    {
        err = nvs_get_u8(handle, key, value);
        if (err == ESP_OK) {
            ESP_LOGI(NVS_TAG, "Retrieved value: %u", *value);
        } else if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(NVS_TAG, "Key not found, returning default value.");
        } else {
            ESP_LOGE(NVS_TAG, "Failed to read value. Error: %s", esp_err_to_name(err));
        }
        nvs_close(handle); // Close NVS handle
    } else {
        ESP_LOGE(NVS_TAG, "Failed to open NVS handle from Read_Uint8_Value function: %s.", esp_err_to_name(err));
    }
}

void nvs_write_string(const char *name_space, nvs_handle_t handle, const char *key, const char *string)
{
    esp_err_t err = nvs_open(name_space, NVS_READWRITE, &handle);
    if(err == ESP_OK)
    {
        err = nvs_set_str(handle, key, string);
        if (err == ESP_OK)
        {
            ESP_LOGI(NVS_TAG, "String written to NVS successfully: %s.", string);
            nvs_commit(handle); // Save the changes
        } else {
            ESP_LOGE(NVS_TAG, "Failed to write string. Error: %s", esp_err_to_name(err));
        }
        nvs_close(handle); // Close NVS handle
    } else {
        ESP_LOGE(NVS_TAG, "Failed to open NVS handle from Write_String function: %s.", esp_err_to_name(err));
    }
}

void nvs_read_string(const char *name_space, nvs_handle_t handle, const char *key, char *buffer, size_t size_of_buffer)
{
    esp_err_t err = nvs_open(name_space, NVS_READONLY, &handle);
    if(err == ESP_OK)
    {
        err = nvs_get_str(handle, key, buffer, &size_of_buffer);
        if (err == ESP_OK) {
            ESP_LOGI(NVS_TAG, "Retrieved string: %s", buffer);
        } else if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(NVS_TAG, "Key not found, returning default string.");
        } else {
            ESP_LOGE(NVS_TAG, "Failed to read string. Error: %s", esp_err_to_name(err));
        }
        nvs_close(handle); // Close NVS handle
    } else {
        ESP_LOGE(NVS_TAG, "Failed to open NVS handle from Read_String function: %s.", esp_err_to_name(err));
    }
}

void nvs_flash_storage_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void nvs_flash_storage_erase(void)
{
    esp_err_t err = nvs_flash_erase();
    if (err == ESP_OK) {
        ESP_LOGI(NVS_TAG, "Older NVS Partition erased successfully");
    } else if (err == ESP_ERR_NOT_FOUND) {
        ESP_LOGW(NVS_TAG, "No NVS partition to erase.");
    } else {
        ESP_LOGE(NVS_TAG, "Failed to erase NVS Partition: %s.", esp_err_to_name(err));
    }
}

void nvs_delete_key(const char *name_space, nvs_handle_t handle, const char *key)
{
    esp_err_t err = nvs_open(name_space, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        err = nvs_erase_key(handle, key);
        if (err == ESP_OK) {
            ESP_LOGI(NVS_TAG, "Key '%s' deleted successfully.", key);
            nvs_commit(handle); // Save changes
        } else if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(NVS_TAG, "Key '%s' not found.", key);
        } else {
            ESP_LOGE(NVS_TAG, "Failed to delete key '%s'. Error: %s", key, esp_err_to_name(err));
        }
        nvs_close(handle);
    } else {
        ESP_LOGE(NVS_TAG, "Failed to open NVS. Error: %s", esp_err_to_name(err));
    }
}