#pragma once

#include "game/Enums.h"

#include <string>

// 현재 단계에서 플레이어의 기본 상태를 보관하는 구조체다.
struct Player
{
    std::string name;
    JobClass job = JobClass::Warrior;
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
};
