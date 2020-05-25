#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void server_module_bytearr_result(unsigned char* byte_arr, uint32_t bytes);
void server_module_string_result(const char* string_resp);


#ifdef __cplusplus
}
#endif