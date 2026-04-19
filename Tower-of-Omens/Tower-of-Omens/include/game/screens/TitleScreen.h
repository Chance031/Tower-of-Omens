#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"

// 타이틀 화면에서 시작/종료 여부를 반환한다.
class TitleScreen
{
public:
    bool Run(const ConsoleRenderer& renderer, const MenuInput& input) const;
};
