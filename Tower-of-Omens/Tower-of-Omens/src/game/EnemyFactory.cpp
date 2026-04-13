#include "game/EnemyFactory.h"

// 전투 종류와 길 선택을 바탕으로 적 데이터를 만든다.
Enemy EnemyFactory::Create(BattleType battleType, PathChoice path) const
{
    if (path == PathChoice::Safe)
    {
        return {"고블린 정찰병", 24, 5, 10};
    }

    if (path == PathChoice::Dangerous)
    {
        return {"광포한 오우거", 65, 15, 25};
    }

    if (path == PathChoice::Unknown)
    {
        return {"그림자 환영", 40, 9, 18};
    }

    switch (battleType)
    {
    case BattleType::Normal:
        return {"슬라임", 30, 6, 8};
    case BattleType::Elite:
        return {"오우거", 60, 14, 20};
    case BattleType::Event:
        return {"그림자 환영", 45, 10, 16};
    }

    return {"슬라임", 30, 6, 8};
}
