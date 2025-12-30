#pragma once

#include "system.hpp"
#include "../types.hpp"
#include "../components.hpp"

#include <windows.h>
#include <array>
#include <iostream>
#include <cstdlib>
#include <cstring>

constexpr int WIDTH = 80;
constexpr int HEIGHT = 24;

class RenderSystem : public ISystem {
public:
    RenderSystem() {
        hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
        system("cls");
        hideCursor();
        drawBorder();
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

        // 파티클 출력 (테두리 안쪽, y+1 위치)
        DWORD written;
        for (int y = 0; y < HEIGHT; ++y) {
            COORD pos = {1, static_cast<SHORT>(y + 1)};  // x+1, y+1 (테두리 안쪽)
            WriteConsoleOutputCharacterA(
                hConsole_, &buffer_[y * WIDTH], WIDTH, pos, &written);
        }
    }

private:
    HANDLE hConsole_;
    std::array<char, WIDTH * HEIGHT> buffer_{};

    void hideCursor() {
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 1;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole_, &info);
    }

    void drawBorder() {
        DWORD written;
        
        // 상단 테두리: +----...----+
        char topLine[WIDTH + 3];
        topLine[0] = '+';
        std::memset(topLine + 1, '-', WIDTH);
        topLine[WIDTH + 1] = '+';
        topLine[WIDTH + 2] = '\0';
        COORD pos = {0, 0};
        WriteConsoleOutputCharacterA(hConsole_, topLine, WIDTH + 2, pos, &written);

        // 좌우 테두리: |
        for (int y = 1; y <= HEIGHT; ++y) {
            COORD leftPos = {0, static_cast<SHORT>(y)};
            WriteConsoleOutputCharacterA(hConsole_, "|", 1, leftPos, &written);
            COORD rightPos = {WIDTH + 1, static_cast<SHORT>(y)};
            WriteConsoleOutputCharacterA(hConsole_, "|", 1, rightPos, &written);
        }

        // 하단 테두리: +----...----+
        pos = {0, HEIGHT + 1};
        WriteConsoleOutputCharacterA(hConsole_, topLine, WIDTH + 2, pos, &written);

        // 안내 메시지
        const char* msg = "  Click to spawn particles! (Ctrl+C to exit)";
        pos = {0, HEIGHT + 2};
        WriteConsoleOutputCharacterA(hConsole_, msg, static_cast<DWORD>(strlen(msg)), pos, &written);
    }
};