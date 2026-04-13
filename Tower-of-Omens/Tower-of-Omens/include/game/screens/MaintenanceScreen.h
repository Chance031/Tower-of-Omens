#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <string>

// 정비 화면의 처리 결과를 나타낸다.
struct MaintenanceResult
{
    GameState nextState = GameState::FloorLoop;
    std::string summary;
};

// 전투 후 정비 화면의 선택과 능력치 보정을 담당한다.
class MaintenanceScreen
{
public:
    // 정비 화면을 표시하고 선택 결과를 플레이어 상태에 반영한다.
    MaintenanceResult Run(Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const;
};
