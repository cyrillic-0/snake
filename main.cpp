#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>

using namespace std;

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;
const int CELL = 25;
const int CELL_W = SCR_WIDTH / CELL;
const int CELL_H = SCR_HEIGHT / CELL;


void draw_grid(SDL_Renderer* rend) {
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    for (int i = CELL; i < SCR_WIDTH; i += CELL) {
        SDL_RenderDrawLine(rend, i, 0, i, SCR_HEIGHT);
        SDL_RenderDrawLine(rend, 0, i, SCR_WIDTH, i);
    }
}


class Color {
public:
    Color(int r, int g, int b, int a=255) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
    int r, g, b, a;
};


class Cell {
public:
    Cell() {}

    Cell(int x, int y) {
        this->x = x;
        this->y = y;
    }

    void move_to(int x, int y) {
        this->x = x;
        this->y = y;
    }

    int x, y;
};


class SnakeCell : public Cell {
public:
    SnakeCell(int x, int y, int order) {
        this->x = x;
        this->y = y;
        this->order = order;
    }

    int order;
    int dx = 1, dy = 0;
};


void fill_screen(SDL_Renderer* rend, const Color& clr) {
    SDL_SetRenderDrawColor(rend, clr.r, clr.g, clr.b, clr.a);
    SDL_RenderFillRect(rend, NULL);
}


void fill_cell(SDL_Renderer* rend, const Cell& cell, const Color& clr) {
    SDL_Rect tmp_rect;
    SDL_SetRenderDrawColor(rend, clr.r, clr.g, clr.b, clr.a);
    tmp_rect.x = cell.x*CELL; tmp_rect.y = cell.y*CELL;
    tmp_rect.w = tmp_rect.h = CELL;
    SDL_RenderFillRect(rend, &tmp_rect);
}


class Snake {
public:
    Snake(int len) {
        for (int i = 1; i <= len; i++) {
            cells.push_back(SnakeCell(len-i, 0, last_cell_id));
            last_cell_id++;
        }
    }

    void change_direction(SDL_KeyboardEvent& event) {
        switch (event.keysym.scancode) {
            case SDL_SCANCODE_A: dx_glob = (dx_glob == 1) ? 1 : -1; dy_glob = 0; break;
            case SDL_SCANCODE_D: dx_glob = (dx_glob == -1) ? -1: 1; dy_glob = 0; break;
            case SDL_SCANCODE_S: dx_glob = 0; dy_glob = (dy_glob == -1) ? -1 : 1; break;
            case SDL_SCANCODE_W: dx_glob = 0; dy_glob = (dy_glob == 1) ? 1 : -1; break;
            default: break;
        }
    }

    void new_cell() {
        SnakeCell lcell = cells[(cells.size()-1)];
        cells.push_back(SnakeCell(lcell.x-lcell.dx, lcell.y-lcell.dy, last_cell_id));
        last_cell_id++;
    }

    void move_cells() {
        vector<SnakeCell> first_cells;
        for (int i = 0; i < cells.size()-1; i++) {
            first_cells.push_back(cells[i]);
        }
        for (int i = 0; i < cells.size(); i++) {
            cells[i].dx = (!i) ? dx_glob : first_cells[i-1].dx;
            cells[i].dy = (!i) ? dy_glob : first_cells[i-1].dy;

            cells[i].x += (cells[i].x < 0 || cells[i].x > CELL_W-1) ? (-CELL_W)*cells[i].dx : cells[i].dx;
            cells[i].y += (cells[i].y < 0 || cells[i].y > CELL_H-1) ? (-CELL_H)*cells[i].dy : cells[i].dy;
        }
    }

    bool check_collision_with_cell(const Cell& cell) {
        return cells[0].x == cell.x && cells[0].y == cell.y;
    }

    bool check_collision_with_snakecells() {
        for (int i = 1; i < cells.size(); i++) {
            if (cells[0].x == cells[i].x && cells[0].y == cells[i].y) {
                return true;
            }
        }
        return false;
    }

    void fill_cells(SDL_Renderer* rend) {
        for (int i = 0; i < cells.size(); i++) {
            fill_cell(rend, cells[i], Color(0, 255, 0));
        }
    }

private:
    vector<SnakeCell> cells;
    int last_cell_id = 0;
    int dx_glob = 1, dy_glob = 0;
};




int main() {
    // SDL2 Init
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        return 1;
    }

    SDL_Window* win; SDL_Renderer* rend;
    SDL_CreateWindowAndRenderer(SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_SHOWN, &win, &rend);
    SDL_SetWindowTitle(win, "Snake v1.0");

    // Randomizer Init
    time_t lt = time(NULL);
    srand(lt);

    Snake snake(5);
    Cell apple(rand() % CELL_W, rand() % CELL_H);

    SDL_Event event;
    bool quit = false;
    int score = 0;
    int timer_count = 0, timer_limit = 200;

    while (!quit) {
        SDL_PollEvent(&event);

        // Check Events
        switch (event.type) {
            case SDL_QUIT: quit = true; break;
            case SDL_KEYDOWN: snake.change_direction(event.key); break;
        }

        if (timer_count <= timer_limit) {
            timer_count++;
        } 
        else {
            // Snake Move
            timer_count = 0;
            snake.move_cells();
            if (snake.check_collision_with_cell(apple)) {
                apple.move_to(rand() % CELL_W, rand() % CELL_H);
                snake.new_cell();
                score++;
            }
            else if(snake.check_collision_with_snakecells()) {
                // Game Over
                quit = true;
                cout << "Game Over! Score: " << score << endl;
            }
        }

        // Fill cells and update the screen
        fill_screen(rend, Color(0, 0, 0));
        fill_cell(rend, apple, Color(255, 0, 0));
        snake.fill_cells(rend);
        draw_grid(rend);
        SDL_RenderPresent(rend);
    }

    // Quit
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}