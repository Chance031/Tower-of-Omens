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
inline int RandomPercent()
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    static std::uniform_int_distribution<int> distribution(1, 100);
    return distribution(generator);
}

inline int RollDie(int minValue, int maxValue)
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(generator);
}

inline int StatModifier(int stat)
{
    return static_cast<int>(std::lround((static_cast<double>(stat) - 10.0) / 2.0));
}

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

inline int PlayerAccuracyStat(const Player& player)
{
    return (player.job == JobClass::Warrior) ? player.agility : player.intelligence;
}

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

inline int ComputePlayerDamage(const Player& player, int enemyDefense, int skillBonus = 0)
{
    const int statPower = (player.job == JobClass::Warrior)
        ? (player.strength + (player.agility / 2))
        : (player.intelligence + (player.spirit / 2));
    return std::max(1, player.atk + skillBonus + (statPower / 3) - enemyDefense);
}

inline int ComputeEnemyDamage(const Enemy& enemy, int defenseValue)
{
    return std::max(1, enemy.atk - defenseValue);
}

inline int RecoveryAmountFromSpirit(const Player& player, bool hpRecovery)
{
    const int statBase = hpRecovery ? player.spirit : player.intelligence;
    return std::max(4, 4 + (statBase / 3));
}

inline int StatusBurnDamage(BattleType battleType)
{
    return (battleType == BattleType::Boss) ? 8 : 6;
}
}
