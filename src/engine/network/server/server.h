/**
 * @file server.h
 * @author https://github.com/shaderko
 * @brief Server-Client connection
 * @version 0.1
 * @date 2023-04-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SERVER_H
#define SERVER_H

#include <uv.h>
#include <stdbool.h>

#include "../../object/object.h"

// definition for loop include
typedef struct Room Room;

typedef enum MessageType MessageType;
enum MessageType
{
    CONNECTION_REQUEST,
    CONNECTION_RESPONSE,
    LOGIN_REQUEST,
    LOGIN_RESPONSE,
    JOIN_ROOM_REQUEST,
    JOIN_ROOM_RESPONSE,
    CREATE_ROOM_REQUEST,
    CREATE_ROOM_RESPONSE,
    DATA_REQUEST,
    DATA_RESPONSE,
    CLIENT_COMMAND,
    SERVER_COMMAND,
    CLIENT_NOTIFICATION,
    SERVER_NOTIFICATION,
    ERROR_NOTIFICATION,
    DISCONNECTION_REQUEST,
    DISCONNECTION_RESPONSE,
    SYNCHRONIZATION_COMPLETE
};

typedef struct Message Message;
struct Message
{
    int client_id;
    MessageType type;

    int data_received;

    /**
     * data
     */
    int length;
    char *data;
};

typedef struct ServerClient ServerClient;
struct ServerClient
{
    int id;

    /**
     * Last time data was sent to client or received from client
     */
    uint32_t last_sent;

    struct sockaddr address;
    uv_udp_t UDPsocket;

    /**
     * Room the client is connected to, this is only here so the server doesn't have to loop through all rooms to find the client
     */
    Room *room;
    bool synchronized;
};

typedef struct ServerClientHandle ServerClientHandle;
struct ServerClientHandle
{
    uv_tcp_t handle;
    ServerClient *client;
};

typedef struct Server Server;
struct Server
{
    uv_loop_t *loop;
    struct sockaddr_in address;

    /**
     * Socket to send and receive game data
     */
    uv_udp_t UDPsocket;

    /**
     * Socket to send and receive chat, connection, disconnection and other non-game data
     */
    uv_tcp_t TCPsocket;

    /**
     * All connected clients to the server
     */
    ServerClient *clients;
    int clients_size;

    /**
     * All rooms created in the server
     */
    Room **rooms;
    int rooms_size;
};

struct AServer
{
    /**
     * Initializes a server
     */
    void (*Init)();

    /**
     * Add client to server
     */
    ServerClient *(*CreateClient)();

    ServerClient *(*GetClient)(int client_id);

    /**
     * Delete client from server
     */
    void (*DeleteClient)(int client_id);

    /**
     * Serialize message
     */
    SerializedDerived (*SerializeMessage)(Message *message);

    /**
     * Deserialize message
     */
    Message *(*DeserializeMessage)(void *buf);

    /**
     * Respond to client request
     */
    // void (*Response)(IPaddress address, int client_id, MessageType type, int size, void *data);

    void (*ReceiveConnection)(uv_stream_t *stream, int status);

    /**
     * Receive data from client
     */
    void (*ReceiveDataTCP)(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);

    /**
     * Receive data from client
     */
    void (*ReceiveDataUDP)(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);

    /**
     * Send object to client
     */
    void (*SendObject)(Object *object, int client_id);

    void (*SendObjectTCP)(Object *object, ServerClientHandle *client_stream);

    /**
     * Get the Server object
     */
    Server *(*GetServer)();
};

extern struct AServer AServer[1];

#endif