/**
 * @file client.c
 * @author https://github.com/shaderko
 * @brief Server-Client connection
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "../../util/util.h"

#include "client.h"
#include "../server/server.h"
#include "../network/network.h"

static Client *client = NULL;

static Client *Init()
{
    client = malloc(sizeof(Client));
    if (!client)
        ERROR_EXIT("Error allocating memory for client\n");

    client->id = -1;
    client->room_id = 0;

    uv_loop_init(&client->loop);

    // Initialize error for libuv
    int error;

    // Server destination address
    struct sockaddr_in dest;
    error = uv_ip4_addr("192.168.0.242", 8000, &dest);
    if (error < 0)
    {
        free(client);
        ERROR_EXIT("Error resolving server host: %s\n", uv_strerror(error));
    }

    // Client address
    struct sockaddr_in addr_to_bind;
    int bound_addr_len = sizeof(addr_to_bind);
    error = uv_ip4_addr("0.0.0.0", 0, (struct sockaddr_in *)&addr_to_bind);
    if (error < 0)
    {
        free(client);
        ERROR_EXIT("Error opening UDP socket: %s\n", uv_strerror(error));
    }

    // Initialize UDP socket
    // TODO: when creating connection, if the address bound to by udp on the server is in use, try another one
    error = uv_udp_init(&client->loop, &client->UDPrecv_socket);
    if (error < 0)
    {
        free(client);
        ERROR_EXIT("Error initializing UDP socket: %s\n", uv_strerror(error));
    }
    error = uv_udp_bind(&client->UDPrecv_socket, (const struct sockaddr *)&addr_to_bind, 0);
    if (error < 0)
    {
        free(client);
        ERROR_EXIT("Error binding UDP socket: %s\n", uv_strerror(error));
    }

    error = uv_udp_getsockname(&client->UDPrecv_socket, (struct sockaddr *)&client->address, &bound_addr_len);
    if (error < 0)
    {
        free(client);
        ERROR_EXIT("Error getting UDP socket name: %s\n", uv_strerror(error));
    }

    // Initialize UDP socket for sending
    error = uv_udp_init(&client->loop, &client->UDPsend_socket);
    if (error < 0)
    {
        free(client);
        ERROR_EXIT("Error initializing UDP socket: %s\n", uv_strerror(error));
    }
    error = uv_udp_connect(&client->UDPsend_socket, (const struct sockaddr *)&dest);
    if (error < 0)
    {
        free(client);
        ERROR_EXIT("Error binding UDP socket: %s\n", uv_strerror(error));
    }

    puts("Establishing connection to server.");

    // Initialize TCP socket
    uv_tcp_init(&client->loop, &client->TCPsocket);
    error = uv_tcp_connect(&client->connect_req, &client->TCPsocket, (const struct sockaddr *)&dest, connect_client);
    if (error < 0)
    {
        uv_close((uv_handle_t *)&client->UDPrecv_socket, NULL);
        free(client);
        ERROR_EXIT("Error opening TCP socket: %s\n", uv_strerror(error));
    }

    // Start thread for client loop
    uv_thread_t recv_tcp_data_thread;
    uv_thread_create(&recv_tcp_data_thread, thread_loop, &client->loop);

    puts("Client initialized.");

    return client;
}

static void connect_client(uv_connect_t *req, int status)
{
    if (status < 0)
        printf("Couldn't connect to server %s\n", uv_strerror(status));

    int error;
    // Start listening for messages on TCP socket
    error = uv_read_start(req->handle, alloc_buffer, AClient->ReceiveDataTCP);
    if (error)
        ERROR_EXIT("Could't start listening for messages on TCP socket: %s\n", uv_strerror(error));

    // Get client's address and port
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&client->address;
    char address_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr_in->sin_addr, address_str, sizeof(address_str));

    printf("Sending address: %s and port: %hu to server\n", address_str, ntohs(addr_in->sin_port));

    Message message = {client->id, CONNECTION_REQUEST, 0, sizeof(client->address), (char *)&client->address};
    send_data_tcp((uv_stream_t *)&client->TCPsocket, &message);
}

static void ReceiveDataTCP(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0)
    {
        fprintf(stderr, "Read error %s, disconnecting from server\n", uv_err_name(nread));
        uv_close((uv_handle_t *)stream, NULL);
        free(buf->base);
        return;
    }

    printf("nread: %zi\n", nread);

    size_t offset = 0;
    while (nread > 0)
    {
        Message *message;
        if (client->partial_msg == NULL)
        {
            // This is the start of a new message
            message = malloc(sizeof(Message));
            memcpy(message, buf->base + offset, sizeof(Message));
            message->data = malloc(message->length);
            message->data_received = 0;
            client->partial_msg = message;
            offset += sizeof(Message);
            nread -= sizeof(Message);
        }
        else
        {
            // This is a continuation of a message
            message = client->partial_msg;
        }

        printf("nread after msg: %zi\n", nread);

        // Copy data from buf to message
        size_t to_copy = (message->length - message->data_received < nread) ? message->length - message->data_received : nread;
        memcpy(message->data + message->data_received, buf->base + offset, to_copy);
        message->data_received += to_copy;
        offset += to_copy;
        nread -= to_copy;

        // If the message is not fully received yet, break the loop
        if (message->data_received < message->length)
        {
            puts("Message not fully received yet.");
            break;
        }

        // Full message received, reset partial message
        client->partial_msg = NULL;

        AClient->ParsingDataTCP(stream, message);
    }

    free(buf->base);
}

static void ParsingDataTCP(uv_stream_t *stream, Message *message)
{
    if (message->type == CONNECTION_RESPONSE)
    {
        puts("Connected to server.");

        // Send login request
        // TODO: add login data

        puts("Sending login request.");

        Message response = {client->id, LOGIN_REQUEST, 0, 0, NULL};
        send_data_tcp(stream, &response);

        free(message->data);
        free(message);
        return;
    }

    if (message->type == LOGIN_RESPONSE)
    {
        // Do all the Login stuff
        puts("Login response received");

        if (message->client_id == -1)
        {
            ERROR_EXIT("Couldn't login to server.\n");
        }

        client->id = message->client_id;
        printf("Client id %i\n", client->id);
    }

    if (client->id == -1)
        ERROR_EXIT("Client not logged in\n");

    switch (message->type)
    {
    case JOIN_ROOM_RESPONSE:
    {
        if (message->length != sizeof(ull))
            ERROR_EXIT("Wrong data length\n");

        memcpy(&client->room_id, message->data, sizeof(ull));
        printf("room id %lld\n", client->room_id);
        break;
    }
    case CREATE_ROOM_RESPONSE:
    {
        if (message->length != sizeof(ull))
            ERROR_EXIT("Wrong data length\n");

        memcpy(&client->room_id, message->data, sizeof(ull));
        printf("created room, id %lld\n", client->room_id);
        break;
    }
    case DISCONNECTION_RESPONSE:
    {
        puts("Disconnected from server.");
        client->room_id = 0;
        client->id = -1;
        break;
    }
    case DATA_RESPONSE:
    {
        puts("Received object TCP");

        SerializedObject *object = malloc(sizeof(SerializedObject));
        memcpy(object, message->data, sizeof(SerializedObject));

        // Collider
        object->collider.derived.data = malloc(object->collider.derived.len);
        memcpy(object->collider.derived.data, message->data + sizeof(SerializedObject), object->collider.derived.len);

        // Renderer
        object->renderer.derived.data = malloc(object->renderer.derived.len);
        memcpy(object->renderer.derived.data, message->data + sizeof(SerializedObject) + object->collider.derived.len, object->renderer.derived.len);

        AObject.Deserialize(object, NULL);

        free(object->collider.derived.data);
        free(object->renderer.derived.data);
        free(object);

        break;
    }
    case SYNCHRONIZATION_COMPLETE:
    {
        int error;
        // Start listening for messages on UDP socket
        error = uv_udp_recv_start(&client->UDPrecv_socket, alloc_buffer, AClient->ReceiveDataUDP);
        if (error < 0)
            ERROR_EXIT("Couldn't start listening for messages on UDP socket: %s\n", uv_strerror(error));
    }
    default:
        break;
    }

    free(message->data);
    free(message);
}

static void ReceiveDataUDP(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags)
{
    puts("Received object");
    if (nread < 0)
    {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)socket, NULL);
        free(buf->base);
        return;
    }

    Message *message = malloc(sizeof(Message));
    memcpy(message, buf->base, sizeof(Message));

    message->data = malloc(message->length);
    memcpy(message->data, buf->base + sizeof(Message), message->length);

    if (message->type != DATA_RESPONSE)
    {
        printf("Problem receiving object %i\n", message->type);
        free(message);
        free(buf->base);
        return;
    }

    SerializedObject *object = malloc(sizeof(SerializedObject));
    memcpy(object, buf->base + sizeof(Message), sizeof(SerializedObject));

    // // Collider
    // object->collider.derived.data = malloc(object->collider.derived.len);
    // memcpy(object->collider.derived.data, buf->base + sizeof(Message) + sizeof(SerializedObject), object->collider.derived.len);

    // // Renderer
    // object->renderer.derived.data = malloc(object->renderer.derived.len);
    // memcpy(object->renderer.derived.data, buf->base + sizeof(Message) + sizeof(SerializedObject) + object->collider.derived.len, object->renderer.derived.len);

    AObject.Deserialize(object, NULL);

    free(message->data);
    free(message);
    free(object);
    free(buf->base);
}

static void JoinRoom(Client *client, ull room_id)
{
    if (!client)
    {
        return;
    }

    // REMOVE AFTER TODO:
    if (client->room_id != 0)
    {
        return;
    }

    void *data = malloc(sizeof(ull));
    memcpy(data, &room_id, sizeof(ull));
    Message message = {client->id, JOIN_ROOM_REQUEST, 0, sizeof(ull), data};
    if (room_id == 0)
    {
        message.type = CREATE_ROOM_REQUEST;
    }

    send_data_tcp((uv_stream_t *)&client->TCPsocket, &message);

    free(data);

    printf("Packet sent\n");
}

static void SendObject(Client *client, Object *object)
{
    if (!client)
    {
        return;
    }

    printf("sending game object %llu\n", object->id);
    if (client->room_id <= 0)
    {
        puts("No room id");
        return;
    }

    SerializedDerived derived = AObject.Serialize(object);
    Message message = {client->id, DATA_RESPONSE, 0, derived.len, derived.data};
    send_data_udp(&client->UDPsend_socket, &message);
}

struct AClient AClient[1] =
    {{
        Init,
        ReceiveDataTCP,
        ParsingDataTCP,
        ReceiveDataUDP,
        JoinRoom,
        SendObject,
    }};