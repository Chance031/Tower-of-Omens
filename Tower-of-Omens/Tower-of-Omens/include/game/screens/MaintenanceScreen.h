#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <string>

struct MaintenanceResult
{
    GameState nextState = GameState::FloorSelect;
    std::string summary;
};

class MaintenanceScreen
{
public:
    MaintenanceResult Run(Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const;
};