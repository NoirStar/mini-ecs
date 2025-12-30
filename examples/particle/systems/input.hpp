#pragma once

#include "system.hpp"
#include "../types.hpp"
#include "../components.hpp"

#include <windows.h>

class InputSystem : public ISystem {
public:
    InputSystem() {
        input_ = GetStdHandle(STD_INPUT_HANDLE);

        DWORD mode;
        GetConsoleMode(input_, &mode);
        
        // QuickEdit 비활성화 + 마우스 활성화
        mode &= ~ENABLE_QUICK_EDIT_MODE;   // QuickEdit 끄기
        mode |= ENABLE_MOUSE_INPUT;        // 마우스 켜기
        mode |= ENABLE_EXTENDED_FLAGS;     // 이게 있어야 QuickEdit 비활성화 적용됨
        
        SetConsoleMode(input_, mode);
    }
    ~InputSystem() = default;

    void update(GameWorld& world, float dt) override {
        INPUT_RECORD record;
        DWORD events;

        if (PeekConsoleInput(input_, &record, 1, &events) && events > 0) {
            ReadConsoleInput(input_, &record, 1, &events);

            if (record.EventType == MOUSE_EVENT) {
                bool isPressed = (record.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED);
                
                // 엣지 감지: 안눌림 → 눌림 순간만!
                if (isPressed && !wasPressed_) {
                    float x = static_cast<float>(record.Event.MouseEvent.dwMousePosition.X - 1);
                    float y = static_cast<float>(record.Event.MouseEvent.dwMousePosition.Y - 1);
                    
                    Entity entity = world.createEntity();
                    world.addComponent(entity, SpawnRequest{x, y, 50});
                }
                wasPressed_ = isPressed;
            }
        }
    }

private:
    HANDLE input_{};
    bool wasPressed_{false};
};