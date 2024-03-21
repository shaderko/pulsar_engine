#include "cellular_automaton.h"
#include "../src/engine/object/object.h"

#include <stdint.h>

#define X_SIZE 100
#define Y_SIZE 100
#define Z_SIZE 100

static int grid[X_SIZE][Y_SIZE][Z_SIZE] = {0};

void initializeGrid()
{
    // grid[X_SIZE / 2][Y_SIZE / 2][Z_SIZE / 2] = 1;
    srand(time(NULL)); // Seed the random number generator with the current time
    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            for (int z = 0; z < Z_SIZE; z++)
            {
                grid[x][y][z] = (double)rand() / RAND_MAX < 0.1f ? 1 : 0;
            }
        }
    }
}

int countActiveNeighbors(int x, int y, int z)
{
    int count = 0;
    // Loop through all neighbors
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            for (int k = -1; k <= 1; k++)
            {
                // Skip the cell itself
                if (i == 0 && j == 0 && k == 0)
                    continue;
                int newX = x + i, newY = y + j, newZ = z + k;
                // Check boundaries
                if (newX >= 0 && newX < X_SIZE && newY >= 0 && newY < Y_SIZE && newZ >= 0 && newZ < Z_SIZE)
                {
                    count += grid[newX][newY][newZ];
                }
            }
        }
    }
    return count;
}

void updateCellState(int x, int y, int z)
{
    int activeNeighbors = countActiveNeighbors(x, y, z);

    // Adjusted rules to promote grouped formations with cave-like holes
    if (grid[x][y][z] == 0 && (activeNeighbors == 5 || activeNeighbors == 6))
    {
        grid[x][y][z] = 1;
    }
    // Survival: An alive cell remains alive if it has 4 to 6 active neighbors
    else if (grid[x][y][z] == 1 && activeNeighbors >= 14)
    {
        grid[x][y][z] = 1;
    }
    // Death: An alive cell dies if it has fewer than 4 or more than 6 active neighbors
    else
    {
        grid[x][y][z] = 0;
    }
}

void updateGrid()
{
    int tempGrid[X_SIZE][Y_SIZE][Z_SIZE] = {0};

    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            for (int z = 0; z < Z_SIZE; z++)
            {
                updateCellState(x, y, z);          // Update the state based on your rules
                tempGrid[x][y][z] = grid[x][y][z]; // Copy the updated state to the temp grid
            }
        }
    }

    // Copy the temp grid back to the main grid
    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            for (int z = 0; z < Z_SIZE; z++)
            {
                grid[x][y][z] = tempGrid[x][y][z];
            }
        }
    }
}

Scene *StartCellularAutomaton()
{
    puts("Starting Cellular Automaton");

    initializeGrid();
    // Example: Update the grid 10 times
    for (int i = 0; i < 10; i++)
    {
        updateGrid();
        // Add code here to display or analyze the grid
    }

    // Create a scene
    Scene *scene = AScene->Init((vec3){1, 1, 1});

    // Create objects for the grid
    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            for (int z = 0; z < Z_SIZE; z++)
            {
                printf("x: %d, y: %d, z: %d, %d\n", x, y, z, grid[x][y][z]);
                if (grid[x][y][z] == 1)
                {
                    puts("Creating object");
                    Object *o = AObject.InitBox(false, true, 1, (vec3){x * 3, y * 3, z * 3}, (vec3){1, 1, 1});
                    AScene->Add(scene, o);
                }
            }
        }
    }

    return scene;
}