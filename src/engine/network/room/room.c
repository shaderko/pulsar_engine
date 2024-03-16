/**
 * @file room.c
 * @author https://github.com/shaderko
 * @brief Room is used as one game (where the actual game runs)
 * @version 0.1
 * @date 2023-04-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdbool.h>
#include <linmath.h>

#include "../../util/util.h"
#include "../network/network.h"
#include "room.h"
#include "../server/server.h"
#include "../../object/object.h"

/**
 * @brief Creates a room with unique id and starts a thread for it
 *
 * @param server - server to create room in
 * @return Room*
 */
static Room *Init(Server *server)
{
    printf("Creating room\n");

    Room *room = malloc(sizeof(Room));
    if (room == NULL)
    {
        ERROR_EXIT("Couldn't allocate memory for room!\n");
    }
    room->server = server;
    room->loop = uv_default_loop(); // TODO: unused
    room->scene = NULL;

    room->is_active = true;
    room->clients = NULL;
    room->clients_size = 0;
    room->room_id = generate_random_id();

    // Create queue TODO: segfaults if the capacity is lower
    room->queue = malloc(sizeof(RoomQueue));
    if (!room->queue)
    {
        free(room);
        ERROR_EXIT("Couldn't allocate memory for room %lld queue!\n", room->room_id);
    }
    room->queue->mutex = SDL_CreateMutex();
    if (room->queue->mutex == NULL)
    {
        free(room);
        ERROR_EXIT("Error failed creating queue mutex! %s\n", SDL_GetError());
    }
    room->queue->data = malloc(sizeof(Message *) * 256);
    if (!room->queue->data)
    {
        free(room->queue);
        free(room);
        ERROR_EXIT("Couldn't allocate memory for room %lld queue data!\n", room->room_id);
    }
    room->queue->tail = 0;
    room->queue->size = 0;
    room->queue->capacity = 12525;

    // TODO: add function to add room to server
    server->rooms = realloc(server->rooms, (server->rooms_size + 1) * sizeof(Room *));
    if (server->rooms == NULL)
    {
        ARoom->DeleteRoom(room);
        ERROR_EXIT("Couldn't allocate memory for server rooms, room %lld!\n", room->room_id);
    }
    server->rooms[server->rooms_size] = room;
    server->rooms_size++;

    room->thread = SDL_CreateThread(ARoom->RoomGame, "Thread", room);

    // uv_thread_t send_udp_data_thread;
    // uv_thread_create(&send_udp_data_thread, thread_loop, &room->loop);

    printf("Room created with id %lld\n", room->room_id);

    return room;
}

static int RoomGame(void *room_init)
{
    Room *room = (Room *)room_init;
    printf("Loading map for room %lld\n", room->room_id);

    room->scene = AScene->Init(&((vec3){0, 0, 0}));
    AScene->ReadFile(room->scene, "file2");

    printf("Map loaded, starting main loop\n");

    while (room->is_active)
    {
        Uint32 startTime = SDL_GetTicks();

        // Receive updates
        ARoom->ProcessData(room);

        // Do all the game stuff
        AScene->Update(room->scene);

        // Send game objects
        ARoom->SendData(room);

        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - startTime;
        if (elapsedTime < 16)
        {
            SDL_Delay(16 - elapsedTime);
        }
        else
        {
            float fps = 1000.0f / elapsedTime;
            printf("Room %lld is clogged, fps: %.2f\n", room->room_id, fps);
        }
    }

    ARoom->DeleteRoom(room);
    SDL_DetachThread(room->thread);

    return 0;
}

static void ProcessData(Room *room)
{
    if (!room)
    {
        ERROR_EXIT("Error processing data.\n");
    }

    SDL_LockMutex(room->queue->mutex);
    while (room->queue->size > 0)
    {
        int index = (room->queue->tail - room->queue->size) % room->queue->capacity;
        if (index < 0)
        {
            index += room->queue->capacity;
        }

        Message *message = room->queue->data[index];

        // TODO: this only works for objects, add more types, like player, etc.
        // But I don't know if this should be in engine or in the game, because all the other types are game specific

        SerializedObject *object = malloc(sizeof(SerializedObject));
        memcpy(object, message->data, sizeof(SerializedObject));

        // Collider
        object->collider.derived.data = malloc(object->collider.derived.len);
        memcpy(object->collider.derived.data, message->data + sizeof(SerializedObject), object->collider.derived.len);

        // Renderer
        object->renderer.derived.data = malloc(object->renderer.derived.len);
        memcpy(object->renderer.derived.data, message->data + sizeof(SerializedObject) + object->collider.derived.len, object->renderer.derived.len);

        AScene->Add(room->scene, AObject.Deserialize(object, room->scene));

        free(object->collider.derived.data);
        free(object->renderer.derived.data);
        free(object);
        free(message->data);
        free(message);

        room->queue->size--;
        room->queue->data[index] = NULL;
    }
    SDL_UnlockMutex(room->queue->mutex);
}

/**
 * @brief Sends all the game objects to all the clients in the room
 *
 * @param room - room to send data from
 */
static void SendData(Room *room)
{
    Server *server = AServer->GetServer();

    if (room->clients_size <= 0)
    {
        return;
    }

    for (int i = 0; i < room->scene->objects_size; i++)
    {
        Object *object = room->scene->objects[i];
        if (!object)
        {
            ERROR_EXIT("Object is NULL!\n");
        }
        for (int j = 0; j < room->clients_size; j++)
        {
            ServerClient *client = room->clients[j];
            if (!client)
            {
                ERROR_EXIT("Client is NULL!\n");
            }

            if (!client->synchronized)
            {
                continue;
            }

            AServer->SendObject(object, client->id);
        }
    }
}

/**
 * @brief Deletes room and all its data, clients rooms pointers are set to NULL, room is removed from server rooms array
 *
 * @param room - room to delete
 */
static void DeleteRoom(Room *room)
{
    printf("Deleting Room %lld\n", room->room_id);

    for (int i = 0; i < room->clients_size; i++)
    {
        room->clients[i]->room = NULL;
    }

    for (int i = 0; i < room->server->rooms_size; i++)
    {
        if (room->server->rooms[i]->room_id == room->room_id)
        {
            room->server->rooms_size--;
            if (room->server->rooms_size <= 0)
            {
                break;
            }
            room->server->rooms[i] = room->server->rooms[room->server->rooms_size];
            printf("Room %lld deleted\n", room->room_id);
            break;
        }
    }

    SDL_Thread *thread = room->thread;
    room->is_active = false;
    free(room->queue);
    free(room);
    SDL_DetachThread(thread);
}

/**
 * @brief Get the Room object from server
 *
 * @param server - server that should be searched for the room id
 * @param room_id - room id
 * @return Room* if room with the given id exists, otherwise NULL
 */
static Room *GetRoom(Server *server, ull room_id)
{
    for (int i = 0; i < server->rooms_size; i++)
    {
        if (server->rooms[i]->room_id == room_id)
        {
            return server->rooms[i];
        }
    }

    return NULL;
}

/**
 * @brief Add the client to a room, if the client is already in a room, nothing happens
 *
 * @param room - room to add the client to
 * @param client - client to add
 */
static void JoinClient(Room *room, ServerClientHandle *client_stream)
{
    ServerClient *client = client_stream->client;
    for (int i = 0; i < room->clients_size; i++)
    {
        if (room->clients[i] == client)
        {
            return;
        }
    }

    room->clients = realloc(room->clients, sizeof(ServerClient *) * (room->clients_size + 1));
    if (room->clients == NULL)
    {
        ERROR_EXIT("Couldn't allocate memory for room %lld clients!\n", room->room_id);
    }

    room->clients[room->clients_size] = client;
    room->clients_size++;

    client->room = room;

    puts("Synchronizing client");

    if (!room->scene)
    {
        puts("Client joined room without scene, waiting for scene creation");
        while (!room->scene)
            SDL_Delay(100);
    }

    // Send all the game objects to the client
    for (int i = 0; i < room->scene->objects_size; i++)
    {
        puts("Sending object");
        Object *object = room->scene->objects[i];
        if (!object)
        {
            ERROR_EXIT("Object is NULL!\n");
        }

        AServer->SendObjectTCP(object, client_stream);
    }

    // Send synchronization complete message to client

    Message message = {client->id, SYNCHRONIZATION_COMPLETE, 0, 0, NULL};
    send_data_tcp((uv_stream_t *)client_stream, &message);

    client->synchronized = true;

    puts("Client joined room");
}

/**
 * @brief Removes the client from a room
 *
 * @param room - room to remove the client from
 * @param client - client to be removed from the room
 */
static void RemoveClient(Room *room, ServerClient *client)
{
    for (int i = 0; i < room->clients_size; i++)
    {
        if (room->clients[i] == client)
        {
            if (room->clients_size <= 1)
            {
                room->clients[i] = NULL;
            }
            room->clients_size--;
            room->clients[i] = room->clients[room->clients_size];

            client->room = NULL;
        }
    }

    puts("Client removed from room");
}

struct ARoom ARoom[1] = {{
    Init,
    RoomGame,
    ProcessData,
    SendData,
    DeleteRoom,
    GetRoom,
    JoinClient,
    RemoveClient,
}};