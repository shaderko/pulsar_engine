/**
 * @file server.c
 * @author https://github.com/shaderko
 * @brief Server is only used to accept connections and assign clients to rooms
 * @version 0.1
 * @date 2023-04-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "server.h"

#include "../room/room.h"
#include "../../util/util.h"
#include "../../common/types/types.h"
#include "../network/network.h"

static Server *server = NULL;

static void Init()
{
    server = malloc(sizeof(Server));
    if (!server)
        ERROR_EXIT("Server memory couldn't be allocated!\n");

    server->clients = NULL;
    server->clients_size = 0;

    server->rooms = NULL;
    server->rooms_size = 0;

    server->loop = uv_default_loop();
    if (!server->loop)
        ERROR_EXIT("Error initializing server loop\n");

    // Print the version of libuv
    unsigned int version = uv_version();
    printf("Using libuv version: %ul\n", version);
    //

    int error;

    // Address to receive data on (localhost)
    error = uv_ip4_addr("0.0.0.0", 8000, &server->address);
    if (error < 0)
        ERROR_EXIT("Error resolving server host: %s\n", uv_strerror(error));

    // Initialize UDP socket
    error = uv_udp_init(server->loop, &server->UDPsocket);
    if (error < 0)
        ERROR_EXIT("Error initializing UDP socket: %s\n", uv_strerror(error));
    error = uv_udp_bind(&server->UDPsocket, (const struct sockaddr *)&server->address, UV_UDP_REUSEADDR);
    if (error < 0)
        ERROR_EXIT("Error binding UDP socket: %s\n", uv_strerror(error));
    error = uv_udp_recv_start(&server->UDPsocket, alloc_buffer, AServer->ReceiveDataUDP);
    if (error < 0)
        ERROR_EXIT("Error receiving UDP data: %s\n", uv_strerror(error));
    puts("UDP socket initialized!");

    // Initialize TCP socket
    error = uv_tcp_init(server->loop, &server->TCPsocket);
    if (error < 0)
        ERROR_EXIT("Error initializing TCP socket: %s\n", uv_strerror(error));
    error = uv_tcp_bind(&server->TCPsocket, (const struct sockaddr *)&server->address, UV_UDP_REUSEADDR);
    if (error < 0)
        ERROR_EXIT("Error binding TCP socket: %s\n", uv_strerror(error));
    error = uv_listen((uv_stream_t *)&server->TCPsocket, 128, AServer->ReceiveConnection);
    if (error < 0)
        ERROR_EXIT("Listen error %s\n", uv_strerror(error));
    puts("TCP socket initialized!");

    // Print Server address and port
    char address_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server->address.sin_addr, address_str, sizeof(address_str));
    printf("Bound to IPv6 address: %s, port: %hu\n", address_str, ntohs(server->address.sin_port));
    //

    puts("Server initialized! Receving data.");

    // Start main loop
    uv_run(server->loop, UV_RUN_DEFAULT);
}

static void ReceiveConnection(uv_stream_t *stream, int status)
{
    if (status < 0)
    {
        printf("New connection error %s\n", uv_strerror(status));
        uv_close((uv_handle_t *)stream, NULL);
        return;
    }

    // TODO: check if client was connected and wants to reconnect, if so, connect him to the correct room

    ServerClientHandle *client_stream = (ServerClientHandle *)malloc(sizeof(ServerClientHandle));
    ServerClient *client = AServer->CreateClient();
    client_stream->client = client;
    uv_tcp_init(server->loop, (uv_tcp_t *)client_stream);
    client_stream->handle.data = client_stream;

    if (uv_accept(stream, (uv_stream_t *)client_stream) != 0)
    {
        puts("Client couldn't connect");

        AServer->DeleteClient(client->id);

        uv_close((uv_handle_t *)client_stream, NULL); // Free client_stream in close_cb TODO:
    }

    puts("Client connected");

    uv_read_start((uv_stream_t *)client_stream, alloc_buffer, AServer->ReceiveDataTCP);
}

/**
 * @brief Add a client to the server
 *
 * @param TCPsocket - of the client's connection
 * @return ServerClient*
 */
static ServerClient *CreateClient()
{
    server->clients = realloc(server->clients, sizeof(ServerClient) * (server->clients_size + 1));
    if (!server->clients)
        ERROR_EXIT("Server clients couldn't be allocated!\n");

    ServerClient client = {server->clients_size, SDL_GetTicks(), 0, 0, 0, 0};

    puts("Client created!");

    server->clients[server->clients_size] = client;
    server->clients_size++;

    return &server->clients[server->clients_size - 1];
}

static ServerClient *GetClient(int client_id)
{
    for (int i = 0; i < server->clients_size; i++)
    {
        if (server->clients[i].id == client_id)
        {
            return &server->clients[i];
        }
    }
    return NULL;
}

// TODO: Fix this, doesn't work
static void DeleteClient(int client_id)
{
    ServerClient *client = NULL;
    for (int i = 0; i < server->clients_size; i++)
    {
        if (server->clients[i].id == client_id)
        {
            client = &server->clients[i];
            break;
        }
    }
    if (client == NULL)
    {
        return;
    }

    // TODO: check if client is in a room and delete it from there
    puts("Deleted client");

    if (server->clients_size == 1)
    {
        free(client);
        free(server->clients);
        server->clients = NULL;
        server->clients_size = 0;
        puts("Clients list freed");
        return;
    }

    server->clients[client_id] = server->clients[server->clients_size - 1];

    free(client);
}

static SerializedDerived SerializeMessage(Message *message)
{
    char *message_data = malloc(message->length + sizeof(Message));
    if (!message_data)
        ERROR_EXIT("Couldn't allocate memory for message data!\n");

    SerializedDerived derived = {message->length + sizeof(Message), message_data};

    memcpy(derived.data, message, sizeof(Message));
    memcpy(derived.data + sizeof(Message), message->data, message->length);

    return derived;
}

static Message *DeserializeMessage(void *buf)
{
    Message *message = malloc(sizeof(Message));
    memcpy(message, (char *)buf, sizeof(Message));

    message->data = malloc(message->length);
    memcpy(message->data, (char *)buf + sizeof(Message), message->length);

    return message;
}

static void ReceiveDataTCP(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    ServerClientHandle *client_stream = (ServerClientHandle *)stream;
    ServerClient *client = client_stream->client;

    if (nread < 0)
    {
        fprintf(stderr, "Read error %s, disconnecting client\n", uv_err_name(nread));
        uv_close((uv_handle_t *)stream, NULL); // Free client_stream in close_cb TODO:
        // AServer->DeleteClient(client->id);
        free(buf->base);
        return;
    }

    Message *message = AServer->DeserializeMessage(buf->base);
    free(buf->base);

    switch (message->type)
    {
    case CONNECTION_REQUEST:
    {
        puts("Connection request received");

        // Get client's address
        memcpy(&client->address, message->data, message->length);

        // Print client's address and port
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&client->address;
        char address_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr_in->sin_addr, address_str, sizeof(address_str));

        // Get ip from tcp connection
        struct sockaddr_storage peername;
        int namelen = sizeof(peername);
        uv_tcp_getpeername((uv_tcp_t *)client_stream, (struct sockaddr *)&peername, &namelen);
        struct sockaddr_in *addr = (struct sockaddr_in *)&peername;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
        uv_ip4_name(addr, ip, INET_ADDRSTRLEN);
        printf("Client's IPv6 address: %s, port: %hu\n", ip, ntohs(addr_in->sin_port));
        //

        int error;

        // Initialize UDP connection to client
        error = uv_udp_init(server->loop, &client->UDPsocket);
        if (error < 0)
            ERROR_EXIT("Error initializing UDP socket %s\n", uv_strerror(error));
        error = uv_udp_connect(&client->UDPsocket, &client->address);
        if (error)
            ERROR_EXIT("Error binding UDP socket %s\n", uv_strerror(error));

        Message response = {0, CONNECTION_RESPONSE, 0, 0, NULL};
        send_data_tcp((uv_stream_t *)client_stream, &response);
        break;
    }
    case LOGIN_REQUEST:
    {
        puts("Login request received");

        // Do all the Login stuff TODO:

        // Respond with the client id
        Message response = {client->id, LOGIN_RESPONSE, 0, 0, NULL};
        send_data_tcp((uv_stream_t *)client_stream, &response);
        break;
    }
    case CREATE_ROOM_REQUEST:
    {
        puts("Create room request received");

        // Create room
        Room *room = ARoom->Init(server);
        if (room == NULL)
        {
            printf("Couldn't create room\n");
            free(message->data);
            free(message);
            return;
        }
        ARoom->JoinClient(room, client_stream);

        // Respond with the room id
        Message response = {client->id, CREATE_ROOM_RESPONSE, 0, sizeof(ull), (char *)&room->room_id};
        send_data_tcp((uv_stream_t *)client_stream, &response);
        break;
    }
    case JOIN_ROOM_REQUEST:
    {
        puts("Join room request received");

        // Check if correct data was sent
        if (message->length != sizeof(ull))
        {
            printf("Invalid message length\n");
            free(message->data);
            free(message);
            return;
        }

        // Join room
        Room *room = ARoom->GetRoom(server, *(ull *)message->data);
        if (room == NULL)
        {
            printf("Couldn't join room\n");
            free(message->data);
            free(message);
            return;
        }
        ARoom->JoinClient(room, client_stream);

        // Respond with the room id
        Message response = {client->id, JOIN_ROOM_RESPONSE, 0, sizeof(ull), (char *)&room->room_id};
        send_data_tcp((uv_stream_t *)client_stream, &response);
        break;
    }
    default:
        break;
    }

    free(message->data);
    free(message);
}

static void ReceiveDataUDP(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
    if (nread < 0)
    {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        // Disconnect client TODO:
        free(buf->base);
        return;
    }

    puts("Received data UDP");

    Message *message = AServer->DeserializeMessage(buf->base);
    free(buf->base);

    ServerClient *client = AServer->GetClient(message->client_id);

    // Process the type of data sent
    switch (message->type)
    {
    case DATA_RESPONSE:
    {
        Room *room = client->room;
        if (!room)
            break;

        SDL_LockMutex(room->queue->mutex);
        room->queue->data[room->queue->tail] = message;
        room->queue->size++;
        room->queue->tail++;

        if (room->queue->tail > room->queue->capacity)
        {
            room->queue->tail = 0;
        }
        SDL_UnlockMutex(room->queue->mutex);
        break;
    }
    default:
        break;
    }
}

static void SendObject(Object *object, int client_id)
{
    printf("Sending object udp of id %llu to %i\n", object->id, client_id);
    ServerClient *client = AServer->GetClient(client_id);
    if (client == NULL)
    {
        return;
    }

    SerializedDerived derived = AObject.SerializePartial(object);
    Message message = {client_id, DATA_RESPONSE, 0, derived.len, derived.data};
    send_data_udp(&client->UDPsocket, &message);
}

static void SendObjectTCP(Object *object, ServerClientHandle *client_stream)
{
    ServerClient *client = client_stream->client;
    if (client == NULL)
        ERROR_EXIT("Client not found!\n");

    printf("Synchronizing object of id %llu to %i\n", object->id, client->id);

    SerializedDerived derived = AObject.Serialize(object);
    Message message = {client->id, DATA_RESPONSE, 0, derived.len, derived.data};
    send_data_tcp((uv_stream_t *)client_stream, &message);
}

static Server *GetServer()
{
    return server;
}

struct AServer AServer[1] =
    {{
        Init,
        CreateClient,
        GetClient,
        DeleteClient,
        SerializeMessage,
        DeserializeMessage,
        ReceiveConnection,
        ReceiveDataTCP,
        ReceiveDataUDP,
        SendObject,
        SendObjectTCP,
        GetServer,
    }};