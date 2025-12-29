#pragma once

#include "system.hpp"
#include "../types.hpp"
#include "../components.hpp"

#include <windows.h>
#include <array>
#include <iostream>
#include <cstdlib>

constexpr int WIDTH = 80;
constexpr int HEIGHT = 24;

class RenderSystem : public ISystem {
public:
    RenderSystem() {
        system("cls");
        hideCursor();
    }
    ~RenderSystem() = default;

    void update(GameWorld& world, float dt) override {
        std::vector<Entity> entities = world.queryEntities<Position, Particle>();

        buffer_.fill(' ');

        for (auto e : entities) {
            Particle* par = world.getComponent<Particle>(e);
            Position* pos = world.getComponent<Position>(e);

            int x = static_cast<int>(pos->x);
            int y = static_cast<int>(pos->y);
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                buffer_[y * WIDTH + x] = par->symbol;
            }
        }

        // 커서를 (0, 0)으로
        COORD pos = {0, 0};
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
        for (int y{}; y < HEIGHT; ++y) {
            for (int x{}; x < WIDTH; ++x) {
                std::cout << buffer_[y * WIDTH + x];
            }
            std::cout << '\n';
        }
    }

private:
    std::array<char, WIDTH * HEIGHT> buffer_{};

    void hideCursor() {
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 1;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(consoleHandle, &info);
    }
};