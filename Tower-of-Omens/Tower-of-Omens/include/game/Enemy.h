#pragma once

#include <string>

// 전투 화면에서 사용할 적의 기본 정보를 나타낸다.
struct Enemy
{
    int         id         = 0;   // enemy_base.csv의 id
    std::string name;
    int         hp         = 0;
    int         atk        = 0;
    int         goldReward = 0;
    int         intentBiasAttack = 0;
    int         intentBiasGuard = 0;
    int         intentBiasRecover = 0;
    double      intentThresholdHp = 0.0;
};
