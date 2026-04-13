#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <optional>

struct FloorLoopResult
{
    GameState nextState = GameState::FloorSelect;
    std::optional<PathChoice> selectedPath;
};

class FloorLoopScreen
{
public:
    FloorLoopResult Run(const Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const;
};