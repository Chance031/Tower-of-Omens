#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <string>

// 정비 화면 종료 후 다음 상태와 처리 내용을 담는다.
struct MaintenanceResult
{
    GameState nextState = GameState::FloorSelect; // 다음으로 전환할 게임 상태
    std::string summary;                          // 정비 결과 요약 텍스트
};

// 전투 후 정비 화면을 담당한다. 회복, 상점, 인벤토리 관리를 처리한다.
class MaintenanceScreen
{
public:
    // 정비 화면을 표시하고 결과를 반환한다.
    MaintenanceResult Run(Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const;
};