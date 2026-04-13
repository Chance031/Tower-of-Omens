#pragma once

#include "game/Enemy.h"
#include "game/Enums.h"

// 전투 종류와 길 선택에 맞는 적 생성을 담당한다.
class EnemyFactory
{
public:
    // 전투 종류와 길 선택을 바탕으로 적 데이터를 만든다.
    Enemy Create(BattleType battleType, PathChoice path) const;
};
