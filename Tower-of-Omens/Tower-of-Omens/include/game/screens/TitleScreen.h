#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"

// 타이틀 화면의 선택 결과를 돌려준다.
class TitleScreen
{
public:
    bool Run(const ConsoleRenderer& renderer, const MenuInput& input) const;
};
