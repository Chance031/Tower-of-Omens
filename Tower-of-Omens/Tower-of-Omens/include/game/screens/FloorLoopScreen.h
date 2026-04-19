#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <optional>

// 층 이동 후 다음 상태와 선택된 경로를 담는다.
struct FloorLoopResult
{
    GameState nextState = GameState::FloorSelect; // 다음으로 전환할 게임 상태
    std::optional<PathChoice> selectedPath;       // 선택된 경로 유형 (보스층은 nullopt)
};

// 층 경로 선택 화면을 담당한다. 10층 이상이면 즉시 보스 전투로 전환한다.
class FloorLoopScreen
{
public:
    // 경로 선택 화면을 표시하고 선택 결과를 반환한다.
    FloorLoopResult Run(const Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const;
};