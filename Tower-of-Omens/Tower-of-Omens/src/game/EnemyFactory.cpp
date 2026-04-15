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

// 적 이름을 기반으로 enemy_base.csv id를 반환한다.
// 향후 CSV 직접 로드 방식으로 교체 예정이며, 현재는 name 매핑으로 유지한다.
int ResolveEnemyId(const std::string& name)
{
    if (name == "슬라임")        return 2001;
    if (name == "고블린 정찰병") return 2002;
    if (name == "광포한 오우거") return 2003;
    if (name == "그림자 환영")   return 2004;
    if (name == "오우거")        return 2005;
    if (name == "독 거미")       return 2006;
    if (name == "해골 궁수")     return 2007;
    if (name == "석화 골렘")     return 2008;
    if (name == "불꽃 정령")     return 2009;
    if (name == "어둠의 기사")   return 2010;
    if (name == "고대 뱀파이어") return 2011;
    if (name == "타락한 마법사") return 2012;
    if (name == "심연의 징조")   return 2013;
    return 0;  // 알 수 없는 적 — FindIntentData fallback 처리
}
}

// 전투 종류와 길 선택, 현재 층을 바탕으로 적 데이터를 만든다.
Enemy EnemyFactory::Create(BattleType battleType, PathChoice path, int floor) const
{
    Enemy enemy;

    if (battleType == BattleType::Boss)
    {
        enemy = {0, "심연의 징조", 180, 24, 100};
    }
    else if (path == PathChoice::Safe)
    {
        enemy = {0, "고블린 정찰병", 24, 5, 10};
    }
    else if (path == PathChoice::Dangerous)
    {
        enemy = {0, "광포한 오우거", 65, 15, 25};
    }
    else if (path == PathChoice::Unknown)
    {
        enemy = {0, "그림자 환영", 40, 9, 18};
    }
    else
    {
        switch (battleType)
        {
        case BattleType::Normal:
            enemy = {0, "슬라임", 30, 6, 8};
            break;
        case BattleType::Elite:
            enemy = {0, "오우거", 60, 14, 20};
            break;
        case BattleType::Event:
feat: EnemyFactory — ResolveEnemyId 추가, enemy.id 세팅            break;
        case BattleType::Boss:
            enemy = {0, "심연의 징조", 180, 24, 100};
            break;
        default:
            enemy = {0, "슬라임", 30, 6, 8};
            break;
        }
    }

    // enemy_base.csv id 매핑
    enemy.id = ResolveEnemyId(enemy.name);

    if (battleType != BattleType::Boss)
    {
        enemy.hp        += FloorBonus(floor, 6);
        enemy.atk       += FloorBonus(floor, 2);
        enemy.goldReward += FloorBonus(floor, 3);
    }

    return enemy;
}
