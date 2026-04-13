#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <optional>

// 층 진행 화면의 처리 결과를 나타낸다.
struct FloorLoopResult
{
    GameState nextState = GameState::FloorLoop;
    std::optional<PathChoice> selectedPath;
};

// 층 진행 화면의 다음 상태를 돌려준다.
class FloorLoopScreen
{
public:
    FloorLoopResult Run(const Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const;
};
