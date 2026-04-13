#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enemy.h"
#include "game/Enums.h"
#include "game/Player.h"

// 전투 화면의 최소 흐름을 담당한다.
class BattleScreen
{
public:
    // 전투 화면을 표시하고 현재 전투의 결과를 돌려준다.
    BattleResult Run(
        Player& player,
        const Enemy& enemy,
        BattleType battleType,
        const ConsoleRenderer& renderer,
        const MenuInput& input) const;
};
