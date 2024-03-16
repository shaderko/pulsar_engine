/**
 * @file client.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-04-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <uv.h>

#include "../room/room.h"
#include "../../common/types/types.h"
#include "../../object/object.h"

typedef struct Client Client;
struct Client
{
    struct sockaddr address;
    int id;
    ull room_id;

    Message *partial_msg;

    uv_udp_t UDPsend_socket;
    uv_udp_t UDPrecv_socket;
    uv_tcp_t TCPsocket;
    uv_connect_t connect_req;
    uv_loop_t loop;
};

static void connect_client(uv_connect_t *req, int status);

struct AClient
{
    /**
     * Initializes a client, connects and logs in to server
     */
    Client *(*Init)();

    void (*ReceiveDataTCP)(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
    void (*ParsingDataTCP)(uv_stream_t *stream, Message *message);
    void (*ReceiveDataUDP)(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags);

    void (*JoinRoom)(Client *client, ull room_id);
    void (*SendObject)(Client *client, Object *object);
};

extern struct AClient AClient[1];

#endif