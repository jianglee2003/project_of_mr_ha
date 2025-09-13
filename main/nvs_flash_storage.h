/*  This library is simple example to use NVS (Non-Volatile Storage).
    Types of data can be stored in NVS:
    +) integer types: uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t
    +) zero-terminated string
    +) variable length binary data (blob)
    This lib is currently used for uint8, int32, string data.

    Basicly:
    +) all the nvs_set_...() are the same with the nvs_set_i8() API, except for the data type.
    +) all the nvs_get_...() are the same with the nvs_get_i8() API, except for the data type.

    Before start, you need to define name_space and declare a handle for NVS.

    Namespace:
    To mitigate potential conflicts in key names between different components, NVS assigns each key-value pair to one of namespaces.
    Namespace names follow the same rules as key names, i.e., the maximum length is 15 characters.
    Furthermore, there can be no more than 254 different namespaces in one NVS partition.
    Namespace name is specified in the nvs_open() or nvs_open_from_partition call.
    This call returns an opaque handle, which is used in subsequent calls to the nvs_get_*, nvs_set_*, and nvs_commit() functions.
    This way, a handle is associated with a namespace, and key names will not collide with same names in other namespaces.
    Please note that the namespaces with the same name in different NVS partitions are considered as separate namespaces.
*/

#ifndef _NVS_FLASH_STORAGE_H_
#define _NVS_FLASH_STORAGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define NVS_TAG "NVS_Flash"

// Example.
// #define STORAGE_NAMESPACE "NVS_Storage"
// nvs_handle_t my_handle

// Used for int32 value.
void nvs_write_value_int32(const char *name_space, nvs_handle_t handle, const char *key, int32_t value);
void nvs_read_value_int32(const char *name_space, nvs_handle_t handle, const char *key, int32_t *value);

// Used for uint8 value.
void nvs_write_value_uint8(const char *name_space, nvs_handle_t handle, const char *key, uint8_t value);
void nvs_read_value_uint8(const char *name_space, nvs_handle_t handle, const char *key, uint8_t *value);

// Used for string.
void nvs_write_string(const char *name_space, nvs_handle_t handle, const char *key, const char *string);
void nvs_read_string(const char *name_space, nvs_handle_t handle, const char *key, char *buffer, size_t size_of_buffer);

// Used for initialize or delete the NVS.
void nvs_flash_storage_init(void);
void nvs_flash_storage_erase(void);

// Delete a key-value pair in NVS.
void nvs_delete_key(const char *name_space, nvs_handle_t handle, const char *key);

#endif
