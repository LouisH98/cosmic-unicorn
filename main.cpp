#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cosmic_unicorn.hpp"

using namespace pimoroni;

PicoGraphics_PenRGB888 graphics(32, 32, nullptr);
CosmicUnicorn cosmic_unicorn;

const int width = CosmicUnicorn::WIDTH;
const int height = CosmicUnicorn::HEIGHT;

// initialize the grid and set all cells to 0
int grid[width][height] = {};
int last_grid[width][height] = {};

int sleep_time = 25;

int alive_colour = graphics.create_pen(0, 0, 255);

// helper function to count the number of alive neighbors for a cell
int count_alive_neighbors(int x, int y)
{
    int count = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            int neighbor_x = (x + i + width) % width; // handle edge cases by wrapping around
            int neighbor_y = (y + j + height + height) % height;
            if (!(i == 0 && j == 0) && grid[neighbor_x][neighbor_y] == 1)
            {
                count++;
            }
        }
    }

    return count;
}

void seed_grid()
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            grid[x][y] = rand() % 2;
        }
    }
}

bool is_grid_stable()
{
    return std::equal(std::begin(grid), std::end(grid), std::begin(last_grid));
}

void handle_input()
{
    if (cosmic_unicorn.is_pressed(cosmic_unicorn.SWITCH_BRIGHTNESS_UP))
    {
        cosmic_unicorn.adjust_brightness(+0.01);
    }
    if (cosmic_unicorn.is_pressed(cosmic_unicorn.SWITCH_BRIGHTNESS_DOWN))
    {
        cosmic_unicorn.adjust_brightness(-0.01);
    }
    if (cosmic_unicorn.is_pressed(cosmic_unicorn.SWITCH_A))
    {
        seed_grid();
    }
    if (cosmic_unicorn.is_pressed(cosmic_unicorn.SWITCH_VOLUME_UP))
    {
        sleep_time += 1;
    }
    if (cosmic_unicorn.is_pressed(cosmic_unicorn.SWITCH_VOLUME_DOWN))
    {
        if (sleep_time > 0)
        {
            sleep_time -= 1;
        }
    }
    if (cosmic_unicorn.is_pressed(cosmic_unicorn.SWITCH_B))
    {
        alive_colour = graphics.create_pen(rand() % 255, rand() % 255, rand() % 255);
    }
}

int main()
{
    stdio_init_all();

    cosmic_unicorn.init();
    cosmic_unicorn.set_brightness(0.3);

    seed_grid();

    while (true)
    {
        handle_input();
        graphics.set_pen(0, 0, 0);
        graphics.clear();

        // create a new grid to hold the next state of the game
        int next_grid[width][height] = {};

        if (is_grid_stable())
        {
            seed_grid();
        }

        // loop through each cell and determine its new state
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int alive_neighbors = count_alive_neighbors(x, y);
                // apply the rules of the game to determine the new state of the cell
                if (grid[x][y] == 1 && (alive_neighbors == 2 || alive_neighbors == 3))
                {
                    next_grid[x][y] = 1;
                    graphics.set_pen(alive_colour);
                    graphics.pixel(Point(x, y));
                }
                else if (grid[x][y] == 0 && alive_neighbors == 3)
                {
                    next_grid[x][y] = 1;
                    graphics.set_pen(255, 255, 255);
                    graphics.pixel(Point(x, y));
                }
            }
        }

        // update the grid to the new state
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                last_grid[x][y] = grid[x][y];
                grid[x][y] = next_grid[x][y];
            }
        }

        cosmic_unicorn.update(&graphics);

        sleep_ms(sleep_time); // adjust the speed of the game by changing the sleep time
    }

    return 0;
}