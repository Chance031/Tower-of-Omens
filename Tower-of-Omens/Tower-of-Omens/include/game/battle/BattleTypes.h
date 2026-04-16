#pragma once

#include <string>

namespace battle
{
struct SkillDefinition
{
    std::string name;
    std::string description;
    int mpCost = 0;
    int attackBonus = 0;
    bool grantsGuard = false;
};

struct ItemDefinition
{
    std::string name;
    std::string description;
    int count = 0;
};

struct D20Check
{
    int roll = 0;
    int modifier = 0;
    int situationalBonus = 0;
    int total = 0;
    int target = 0;
    bool success = false;
};

struct EnemyStatusState
{
    int burnTurns = 0;
    int wetTurns = 0;
    int bindTurns = 0;
    int staggerTurns = 0;
};
}
