#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <array>
#include "cosmic_unicorn.hpp"

using namespace pimoroni;
using namespace std;

PicoGraphics_PenRGB888 graphics(32, 32, nullptr);
CosmicUnicorn cosmic_unicorn;

const int width = CosmicUnicorn::WIDTH;
const int height = CosmicUnicorn::HEIGHT;

vector<vector<int>> grid(width, vector<int>(height, 0));

// create history vector to store the last 3 frames
vector<vector<vector<int>>> history(3, vector<vector<int>>(width, vector<int>(height, 0)));

int sleep_time = 25;

int alive_colour = graphics.create_pen(0, 0, 255);

int shift_colour(int colour, int shift)
{
    int r = (colour >> 16) & 0xFF;
    int g = (colour >> 8) & 0xFF;
    int b = colour & 0xFF;

    r = (r + shift) % 255;
    g = (g + shift) % 255;
    b = (b + shift) % 255;

    return graphics.create_pen(r, g, b);
}

int dim_colour_by_percent(int colour, int percent)
{
    int r = (colour >> 16) & 0xFF;
    int g = (colour >> 8) & 0xFF;
    int b = colour & 0xFF;

    r = r * (100 - percent) / 100;
    g = g * (100 - percent) / 100;
    b = b * (100 - percent) / 100;

    return graphics.create_pen(r, g, b);
}

void add_to_history(const vector<vector<int>> &grid)
{
    // add the current grid to the history vector
    history.push_back(grid);

    // remove the oldest grid from the history vector
    if (history.size() > 3)
    {
        history.erase(history.begin());
    }
}

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

bool is_game_stuck()
{
    // check if the current grid is the same as any of the grids in the history vector
    if (grid == history[0])
    {
        return true;
    }
    return false;
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
        vector<vector<int>> next_grid(width, vector<int>(height, 0));

        if (is_game_stuck())
        {
            sleep_ms(1000);
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
                    graphics.set_pen(shift_colour(alive_colour, 30));
                    graphics.pixel(Point(x, y));
                }
            }
        }

        // update the grid to the new state
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                grid[x][y] = next_grid[x][y];
            }
        }

        cosmic_unicorn.update(&graphics);

        add_to_history(grid);

        sleep_ms(sleep_time); // adjust the speed of the game by changing the sleep time
    }

    return 0;
}