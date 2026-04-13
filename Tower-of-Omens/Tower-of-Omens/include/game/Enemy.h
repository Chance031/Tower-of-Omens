#pragma once

#include <string>

// 전투 화면에서 사용할 적의 기본 정보를 나타낸다.
struct Enemy
{
    std::string name;
    int hp = 0;
    int atk = 0;
};
