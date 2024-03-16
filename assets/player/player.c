#include "player.h"

void move(Player *player)
{
    player->position[0] = player->position[0] + player->velocity[0] * 1; // add global.time.deltatime
    player->position[1] = player->position[1] + player->velocity[1] * 1;
}
