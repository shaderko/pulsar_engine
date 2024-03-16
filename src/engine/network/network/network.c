/**
 * @file network.c
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-07-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "network.h"

/**
 * @brief Allocate buffer for reading
 *
 * @param handle
 * @param suggested_size
 * @param buf
 */
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    // TODO: This needs to be fixed for production and actually if this shuold work over network
    // to be able to split data into multiple chunks and then merge them together on the clients side
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void written(uv_write_t *req, int status)
{
    if (status < 0)
        printf("Couldn't write to server\n");

    puts("Written.");
    char *buf = (void *)req->data;
    free(buf);
    free(req);
}

void send_data_tcp(uv_stream_t *stream, Message *message)
{
    SerializedDerived message_serialized = AServer->SerializeMessage(message);
    uv_write_t *res = malloc(sizeof(uv_write_t));
    uv_buf_t response_buf = uv_buf_init((char *)message_serialized.data, message_serialized.len);
    printf("Size of response: %zu\n", response_buf.len);
    res->data = (void *)response_buf.base;
    uv_write(res, stream, &response_buf, 1, written);
}

void send_data_udp(uv_udp_t *handle, Message *message)
{
    SerializedDerived message_serialized = AServer->SerializeMessage(message);
    uv_buf_t response_buf = uv_buf_init((char *)message_serialized.data, message_serialized.len);
    int result = uv_udp_try_send(handle, &response_buf, 1, NULL);
    if (result < 0)
    {
        printf("Error sending UDP packet: %s\n", uv_strerror(result));
    }
    free(response_buf.base);
}

void thread_loop(void *arg)
{
    uv_loop_t *loop = (uv_loop_t *)arg;
    uv_run(loop, UV_RUN_DEFAULT);
}
