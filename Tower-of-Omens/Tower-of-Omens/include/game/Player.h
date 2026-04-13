#pragma once

#include "game/Enums.h"

#include <string>
#include <vector>

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
    int atk = 0;
    int def = 0;
    int maxHp = 0;
    int maxMp = 0;
    int potionCount = 0;
    int etherCount = 0;
    std::string weaponName;
    int weaponAtkBonus = 0;
    std::string armorName;
    int armorDefBonus = 0;
    std::string bagWeaponName;
    int bagWeaponAtkBonus = 0;
    std::string bagArmorName;
    int bagArmorDefBonus = 0;
    std::vector<std::string> relicNames;
};
