#include "game/EnemyFactory.h"

namespace
{
// 현재 층을 바탕으로 적 스탯 보정값을 계산한다.
int FloorBonus(int floor, int step)
{
    if (floor <= 1)
    {
        return 0;
    }

    return (floor - 1) * step;
}
}

// 전투 종류와 길 선택, 현재 층을 바탕으로 적 데이터를 만든다.
Enemy EnemyFactory::Create(BattleType battleType, PathChoice path, int floor) const
{
    Enemy enemy;

    if (battleType == BattleType::Boss)
    {
        enemy = {"심연의 징조", 180, 24, 100};
    }
    else if (path == PathChoice::Safe)
    {
        enemy = {"고블린 정찰병", 24, 5, 10};
    }
    else if (path == PathChoice::Dangerous)
    {
        enemy = {"광포한 오우거", 65, 15, 25};
    }
    else if (path == PathChoice::Unknown)
    {
        enemy = {"그림자 환영", 40, 9, 18};
    }
    else
    {
        switch (battleType)
        {
        case BattleType::Normal:
            enemy = {"슬라임", 30, 6, 8};
            break;
        case BattleType::Elite:
            enemy = {"오우거", 60, 14, 20};
            break;
        case BattleType::Event:
            enemy = {"그림자 환영", 45, 10, 16};
            break;
        case BattleType::Boss:
            enemy = {"심연의 징조", 180, 24, 100};
            break;
        default:
            enemy = {"슬라임", 30, 6, 8};
            break;
        }
    }

    if (battleType != BattleType::Boss)
    {
        enemy.hp += FloorBonus(floor, 6);
        enemy.atk += FloorBonus(floor, 2);
        enemy.goldReward += FloorBonus(floor, 3);
    }

    return enemy;
}
