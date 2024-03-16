/**
 * @file network.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-07-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <uv.h>
#include "../server/server.h"

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void written(uv_write_t *req, int status);
void send_data_tcp(uv_stream_t *stream, Message *message);
void send_data_udp(uv_udp_t *handle, Message *message);
void thread_loop(void *arg);

#endif