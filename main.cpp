#include <SDL.h>
#include <iostream>
#include <math.h>
#define PI 3.141592653589
#define DR 0.0174533
#undef main

void DrawFilledRect(SDL_Renderer* renderer, int x, int y, int width, int height) {

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;

    SDL_RenderFillRect(renderer, &rect);
}

void DrawRect(SDL_Renderer* renderer, int x, int y, int width, int height) {

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;

    SDL_RenderDrawRect(renderer, &rect);
}

void draw_player(SDL_Renderer* renderer, int x, int y, double pa, double pdx, double pdy) {
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    DrawFilledRect(renderer, x, y, 8, 8);

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void displayMap(SDL_Renderer* renderer, int map[], int mapx, int mapy) {
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    for (int y = 0; y < mapy; y++) {
        for (int x = 0; x < mapx; x++) {
            int posx = x * 64, posy = y * 64;
            if (map[y * mapx + x] > 0) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                DrawFilledRect(renderer, posx, posy, 64, 64);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                DrawFilledRect(renderer, posx, posy, 64, 64);
            }
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            DrawRect(renderer, posx, posy, 64, 64);
        }
    }
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

int px = 300, py = 300;
double pa = 0.0001;

bool APressed = false;
bool DPressed = false;
bool WPressed = false;
bool SPressed = false;

const int mapX = 8, mapY = 8, mapTS = mapX * mapY;

int map[mapTS] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};



float dist(float ax, float ay, float bx, float by) {
    return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}

void drawRays3d(SDL_Renderer* renderer) {
    int r, mx, my, mp, dof; double rx, ry, ra, xo, yo, disT;
    ra = pa - DR * 30; if (ra < 0) { ra += 2 * PI; } if (ra > 2 * PI) { ra -= 2 * PI; }

    for (r = 0; r < 60; r++) {
        // Horizontal
        dof = 0;
        float disH = 10000000, hx = px, hy = py;
        float aTan = -1 / tan(ra);
        if (ra > PI) { ry = (((int)py >> 6) << 6) - 0.0001; rx = (py - ry) * aTan + px; yo = -64; xo = -yo * aTan; } // looking up
        if (ra < PI) { ry = (((int)py >> 6) << 6) + 64;     rx = (py - ry) * aTan + px; yo = 64; xo = -yo * aTan; } // looking down
        if (ra == 0 || ra == PI) { rx = px; ry = py; dof = 8; } // looking straight left or right
        while (dof < 8) {
            mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1) { hx = rx; hy = ry; disH = dist(px, py, hx, hy); dof = 8; }
            else { rx += xo; ry += yo; dof += 1; }
        }

        Uint8 rc, g, b, a;
        SDL_GetRenderDrawColor(renderer, &rc, &g, &b, &a);

        // Vertical 
        dof = 0;
        float nTan = -tan(ra);
        float disV = 10000000, vx = px, vy = py;
        if (ra > PI / 2 && ra < (3 * PI) / 2) { rx = (((int)px >> 6) << 6) - 0.0001; ry = (px - rx) * nTan + py; xo = -64; yo = -xo * nTan; } // looking left
        if (ra < PI / 2 || ra >(3 * PI) / 2) { rx = (((int)px >> 6) << 6) + 64;     ry = (px - rx) * nTan + py; xo = 64; yo = -xo * nTan; } // looking right
        if (ra == 0 || ra == PI) { rx = px; ry = py; dof = 8; } // looking straight left or right
        while (dof < 8) {
            mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1) { vx = rx; vy = ry; disV = dist(px, py, vx, vy); dof = 8; }
            else { rx += xo; ry += yo; dof += 1; }
        }
        if (disV < disH) { rx = vx; ry = vy; disT = disV; SDL_SetRenderDrawColor(renderer, 0, 240, 0, 255); }
        else { rx = hx; ry = hy; disT = disH; SDL_SetRenderDrawColor(renderer, 0, 160, 0, 255); }



        SDL_RenderDrawLine(renderer, px + 4, py + 4, rx, ry);

        // draw 3d walls
        float ca = pa - ra;
        disT = disT * cos(ca);
        float lineH = (64 * 320) / disT; if (lineH > 320) { lineH = 320; }
        float lineO = 160 - (lineH / 2);
        for (int z = 0; z < 8; z++) {
            SDL_RenderDrawLine(renderer, r * 8 + 530 + z, lineO, r * 8 + 530 + z, lineH + lineO);
            Uint8 rc2, g2, b2, a2;
            SDL_GetRenderDrawColor(renderer, &rc2, &g2, &b2, &a2);
            SDL_SetRenderDrawColor(renderer, 130, 130, 130, 255);
            SDL_RenderDrawLine(renderer, r * 8 + 530 + z, lineO + lineH, r * 8 + 530 + z, lineH + lineO * 2);
            SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
            SDL_RenderDrawLine(renderer, r * 8 + 530 + z, 0, r * 8 + 530 + z, lineO);
            SDL_SetRenderDrawColor(renderer, rc2, g2, b2, a2);
        }



        SDL_SetRenderDrawColor(renderer, rc, g, b, a);
        ra += DR; if (ra < 0) { ra += 2 * PI; } if (ra > 2 * PI) { ra -= 2 * PI; }
    }
}

const int tileSize = 64;

bool isWall(int x, int y) {
    int gridX = x / tileSize;
    int gridY = y / tileSize;
    if (gridX >= 0 && gridX < mapX && gridY >= 0 && gridY < mapY) {
        return map[gridY * mapX + gridX] == 1;
    }
    return true;
}

int main(int argc, char* argv[]) {
    double pdx = cos(pa), pdy = sin(pa);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("2D raycaster to 3D rendering", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 512, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

    bool running = true;

    SDL_Event event;

    const int FPS = 60;
    const int FrameDelay = 1000 / FPS;

    Uint32 frameStart;
    int frameTime;

    while (running) {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_a:
                    APressed = true;
                    break;
                case SDLK_d:
                    DPressed = true;
                    break;
                case SDLK_w:
                    WPressed = true;
                    break;
                case SDLK_s:
                    SPressed = true;
                    break;
                }
            }
            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_a:
                    APressed = false;
                    break;
                case SDLK_d:
                    DPressed = false;
                    break;
                case SDLK_w:
                    WPressed = false;
                    break;
                case SDLK_s:
                    SPressed = false;
                    break;
                }
            }
        }

        if (APressed) { pa -= 0.05; if (pa < 0) { pa += 2 * PI + 0.0001; } pdx = cos(pa) * 3; pdy = sin(pa) * 3; }
        if (DPressed) { pa += 0.05; if (pa > 2 * PI) { pa -= 2 * PI + 0.0001; } pdx = cos(pa) * 3; pdy = sin(pa) * 3; }
        int next_px = px + pdx;
        int next_py = py + pdy;

        if (WPressed) {
            if (!isWall(next_px, py)) {
                px = next_px;
            }
            if (!isWall(px, next_py)) {
                py = next_py;
            }
        }
        if (SPressed) {
            next_px = px - pdx;
            next_py = py - pdy;
            if (!isWall(next_px, py)) {
                px = next_px;
            }
            if (!isWall(px, next_py)) {
                py = next_py;
            }
        }

        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderClear(renderer);

        displayMap(renderer, map, mapX, mapY);

        draw_player(renderer, px, py, pa, pdx, pdy);

        drawRays3d(renderer);

        SDL_RenderPresent(renderer);



        // FPS cap
        frameTime = SDL_GetTicks() - frameStart;

        if (FrameDelay > frameTime) {
            SDL_Delay(FrameDelay - frameTime);
        }


    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
