/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __ESP_ALINK_LOG_H__
#define __ESP_ALINK_LOG_H__

#include "esp_log.h"
#include "errno.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALINK_LOGE( format, ... ) ESP_LOGE(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGW( format, ... ) ESP_LOGW(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGI( format, ... ) ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define ALINK_LOGD( format, ... ) ESP_LOGD(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGV( format, ... ) ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define ALINK_ERROR_CHECK(con, err, format, ...) if(con) {ALINK_LOGE(format, ##__VA_ARGS__); perror(__func__); return err;}
#define ALINK_PARAM_CHECK(con) if(con) {ALINK_LOGE("Parameter error: %s", #con); assert(0 && #con);}

#ifndef CONFIG_LOG_ALINK_LEVEL
#define CONFIG_LOG_ALINK_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#endif
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL CONFIG_LOG_ALINK_LEVEL

#ifdef __cplusplus
}
#endif


#endif/*!< __ESP_ALINK_LOG_H__ */

