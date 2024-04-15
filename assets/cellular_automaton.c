#include "cellular_automaton.h"
#include "../src/engine/object/object.h"
#include "../src/engine/util/util.h"

#include <stdint.h>
#include <math.h>

#define SET_BIT(byte, bit) ((byte) |= (1U << (bit)))
#define CLEAR_BIT(byte, bit) ((byte) &= ~(1U << (bit)))
#define TOGGLE_BIT(byte, bit) ((byte) ^= (1U << (bit)))
#define CHECK_BIT(byte, bit) ((byte) & (1U << (bit)))

#define GET_BYTE_INDEX(z) ((z / 8))
#define GET_BIT_INDEX(z) (z % 8)

#define X_SIZE 150
#define Y_SIZE 150
#define Z_SIZE 150

uint8_t ***grid;

void deleteCellularAutomaton()
{
    puts("Deleting grid...");
    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            free(grid[x][y]);
        }
        free(grid[x]);
    }
    free(grid);
    puts("Grid deleted!");
}

void initializeGrid()
{
    printf("Initializing grid...");

    // Allocate memory for the X dimension
    grid = malloc(X_SIZE * sizeof(uint8_t **));
    if (!grid)
        ERROR_RETURN("Failed to allocate memory for grid %d X dimension", X_SIZE);
    for (int x = 0; x < X_SIZE; x++)
    {
        // Allocate memory for the Y dimension
        grid[x] = malloc(Y_SIZE * sizeof(uint8_t *));
        if (!grid[x])
            ERROR_RETURN("Failed to allocate memory for grid %d Y dimension", Y_SIZE);
        for (int y = 0; y < Y_SIZE; y++)
        {
            // Allocate memory for the Z dimension, compacted into bytes
            grid[x][y] = calloc(GET_BYTE_INDEX(Z_SIZE) + 1, sizeof(uint8_t));
            if (!grid[x][y])
                ERROR_RETURN("Failed to allocate memory for grid %d Z dimension", Z_SIZE);
        }
    }

    printf("Initialized");

    srand(time(NULL));

    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            for (int z = 0; z < Z_SIZE; z++)
            {
                if ((double)rand() / RAND_MAX < 0.1f)
                {
                    SET_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
                }
                else
                {
                    CLEAR_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
                }
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
                    if (CHECK_BIT(grid[newX][newY][GET_BYTE_INDEX(newZ)], GET_BIT_INDEX(newZ)))
                        count++;
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
    if (!CHECK_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z)) && (activeNeighbors == 5 || activeNeighbors == 6))
    {
        SET_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
    }
    // Survival: An alive cell remains alive if it has 4 to 6 active neighbors
    else if (CHECK_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z)) && activeNeighbors >= 14)
    {
        SET_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
    }
    // Death: An alive cell dies if it has fewer than 4 or more than 6 active neighbors
    else
    {
        CLEAR_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
    }
}

void updateGrid()
{
    // Calculate the size of the Z dimension in bytes
    size_t zByteSize = GET_BYTE_INDEX(Z_SIZE) + 1;

    // Dynamically allocate memory for tempGrid on the heap
    uint8_t ***tempGrid = malloc(X_SIZE * sizeof(uint8_t **));
    for (int x = 0; x < X_SIZE; x++)
    {
        tempGrid[x] = malloc(Y_SIZE * sizeof(uint8_t *));
        for (int y = 0; y < Y_SIZE; y++)
        {
            tempGrid[x][y] = calloc(zByteSize, sizeof(uint8_t));
        }
    }

    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            for (int z = 0; z < Z_SIZE; z++)
            {
                updateCellState(x, y, z); // Update the state based on your rules
                if (CHECK_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z)))
                {
                    SET_BIT(tempGrid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
                }
                else
                {
                    CLEAR_BIT(tempGrid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
                }
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
                if (CHECK_BIT(tempGrid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z)))
                {
                    SET_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
                }
                else
                {
                    CLEAR_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z));
                }
            }
        }
    }

    // Free the allocated memory for tempGrid
    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            free(tempGrid[x][y]);
        }
        free(tempGrid[x]);
    }
    free(tempGrid);
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
    Scene *scene = AScene->Init(&(vec3){1, 1, 1});

    // Create objects for the grid
    for (int x = 0; x < X_SIZE; x++)
    {
        for (int y = 0; y < Y_SIZE; y++)
        {
            for (int z = 0; z < Z_SIZE; z++)
            {
                if (CHECK_BIT(grid[x][y][GET_BYTE_INDEX(z)], GET_BIT_INDEX(z)))
                {
                    printf("Cell at (%d, %d, %d) is alive.\n", x, y, z);
                    Object *o = AObject.InitBox((vec3){x * 2, y * 2, z * 2}, (vec3){0, 0, 0}, (vec3){1, 1, 1});
                    // AScene->Add(scene, o);
                }
            }
        }
    }

    return scene;
}