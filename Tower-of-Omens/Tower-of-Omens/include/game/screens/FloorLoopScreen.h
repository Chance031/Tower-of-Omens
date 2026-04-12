#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Player.h"

// 층 진행 화면의 다음 이동 방향을 돌려준다.
class FloorLoopScreen
{
public:
    bool Run(const Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const;
};
