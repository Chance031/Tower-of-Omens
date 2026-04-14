#pragma once

#include "game/Enums.h"

#include <algorithm>
#include <string>
#include <vector>

struct ConsumableStack
{
    std::string id;
    int count = 0;
};

// 현재 단계에서 플레이어의 기본 상태를 보관하는 구조체다.
struct Player
{
    std::string name;
    JobClass job = JobClass::Warrior;
    int level = 1;
    int statPoints = 0;
    int floor = 1;
    int hp = 0;
    int mp = 0;
    int gold = 0;
    int strength = 0;
    int agility = 0;
    int intelligence = 0;
    int spirit = 0;
    int atk = 0;
    int def = 0;
    int maxHp = 0;
    int maxMp = 0;
    int bonusAttackPower = 0;
    int bonusDefense = 0;
    int bonusMaxHp = 0;
    int bonusMaxMp = 0;
    int potionCount = 0;
    int etherCount = 0;
    int nextAttackMultiplier = 1;
    std::string weaponName;
    int weaponAtkBonus = 0;
    std::string armorName;
    int armorDefBonus = 0;
    std::string bagWeaponName;
    int bagWeaponAtkBonus = 0;
    std::string bagArmorName;
    int bagArmorDefBonus = 0;
    std::vector<ConsumableStack> consumables;
    std::vector<std::string> relicNames;
};

inline void RefreshDerivedStats(Player& player, bool refillResources = false)
{
    const int previousMaxHp = std::max(1, player.maxHp);
    const int previousMaxMp = std::max(0, player.maxMp);
    const int preservedHp = player.hp;
    const int preservedMp = player.mp;

    const int baseHp = 40 + player.strength * 4 + player.spirit * 2;
    const int baseMp = 10 + player.intelligence * 3 + player.spirit * 2;
    const int baseAttack = (player.job == JobClass::Warrior)
        ? (4 + player.strength + (player.agility / 2))
        : (4 + player.intelligence + (player.spirit / 2));
    const int baseDefense = ((player.job == JobClass::Warrior) ? 4 : 1) + (player.agility / 2) + (player.spirit / 4);

    player.maxHp = std::max(1, baseHp + player.bonusMaxHp);
    player.maxMp = std::max(0, baseMp + player.bonusMaxMp);
    player.atk = std::max(1, baseAttack + player.weaponAtkBonus + player.bonusAttackPower);
    player.def = std::max(0, baseDefense + player.armorDefBonus + player.bonusDefense);

    if (refillResources)
    {
        player.hp = player.maxHp;
        player.mp = player.maxMp;
        return;
    }

    if (previousMaxHp > 0)
    {
        player.hp = std::clamp((preservedHp * player.maxHp) / previousMaxHp, 0, player.maxHp);
    }
    else
    {
        player.hp = player.maxHp;
    }

    if (previousMaxMp > 0)
    {
        player.mp = std::clamp((preservedMp * player.maxMp) / previousMaxMp, 0, player.maxMp);
    }
    else
    {
        player.mp = player.maxMp;
    }
}
