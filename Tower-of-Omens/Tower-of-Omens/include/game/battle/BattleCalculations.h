#pragma once

#include "game/Enemy.h"
#include "game/Enums.h"
#include "game/Player.h"
#include "game/battle/BattleTypes.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>
#include <string>

namespace battle
{
// 1~100 사이의 난수를 반환한다.
inline int RandomPercent()
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    static std::uniform_int_distribution<int> distribution(1, 100);
    return distribution(generator);
}

// minValue~maxValue 범위의 주사위를 굴려 결과를 반환한다.
inline int RollDie(int minValue, int maxValue)
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(generator);
}

// 스탯 수치를 d20 수정치로 변환한다. (stat - 10) / 2, 반올림.
inline int StatModifier(int stat)
{
    return static_cast<int>(std::lround((static_cast<double>(stat) - 10.0) / 2.0));
}

// d20을 굴려 스탯 수정치와 상황 보너스를 더한 뒤 목표값과 비교해 판정 결과를 반환한다.
inline D20Check MakeD20Check(int stat, int target, int situationalBonus = 0)
{
    D20Check check;
    check.roll = RollDie(1, 20);
    check.modifier = StatModifier(stat);
    check.situationalBonus = situationalBonus;
    check.total = check.roll + check.modifier + check.situationalBonus;
    check.target = target;
    check.success = check.total >= check.target;
    return check;
}

inline std::string FormatD20Check(const D20Check& check)
{
    std::ostringstream stream;
    stream << "d20(" << check.roll << ")";
    if (check.modifier != 0)
    {
        stream << (check.modifier > 0 ? " +" : " ") << check.modifier;
    }
    if (check.situationalBonus != 0)
    {
        stream << (check.situationalBonus > 0 ? " +" : " ") << check.situationalBonus;
    }
    stream << " = " << check.total << " vs " << check.target;
    return stream.str();
}

// 직업에 따라 명중 판정에 사용할 스탯을 반환한다. 전사=AGI, 마법사=INT.
inline int PlayerAccuracyStat(const Player& player)
{
    return (player.job == JobClass::Warrior) ? player.agility : player.intelligence;
}

// 전투 유형에 따라 기본 명중 난도를 반환한다.
inline int BaseAttackDifficulty(BattleType battleType)
{
    switch (battleType)
    {
    case BattleType::Elite:
        return 12;
    case BattleType::Boss:
        return 14;
    case BattleType::Event:
        return 11;
    case BattleType::Normal:
    default:
        return 10;
    }
}

// 플레이어의 ATK, 스탯 기여, 스킬 보너스, 적 방어력을 종합해 최종 데미지를 계산한다.
inline int ComputePlayerDamage(const Player& player, int enemyDefense, int skillBonus = 0)
{
    const int statPower = (player.job == JobClass::Warrior)
        ? (player.strength + (player.agility / 2))
        : (player.intelligence + (player.spirit / 2));
    return std::max(1, player.atk + skillBonus + (statPower / 3) - enemyDefense);
}

// 적 ATK에서 플레이어 방어력을 빼 최종 데미지를 계산한다. 최솟값은 1.
inline int ComputeEnemyDamage(const Enemy& enemy, int defenseValue)
{
    return std::max(1, enemy.atk - defenseValue);
}

// 전투 후 자연 회복량을 계산한다. hpRecovery=true면 HP, false면 MP 회복량.
inline int RecoveryAmountFromSpirit(const Player& player, bool hpRecovery)
{
    const int statBase = hpRecovery ? player.spirit : player.intelligence;
    return std::max(4, 4 + (statBase / 3));
}

// 화상 상태이상의 턴당 피해량을 반환한다. 보스전에서는 더 강하다.
inline int StatusBurnDamage(BattleType battleType)
{
    return (battleType == BattleType::Boss) ? 8 : 6;
}
}
