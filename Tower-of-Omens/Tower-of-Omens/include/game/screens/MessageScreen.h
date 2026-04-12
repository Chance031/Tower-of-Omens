#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"

#include <string>

// 단순 메시지 화면을 보여주고 키 입력을 기다린다.
class MessageScreen
{
public:
    void Show(
        const ConsoleRenderer& renderer,
        const MenuInput& input,
        const std::string& title,
        const std::string& body) const;
};
