#pragma once

// Tower of Omens에서 사용하는 상위 게임 상태다.
enum class GameState
{
    Title,
    JobSelect,
    FloorLoop,
    GameOver,
    Clear,
    Exit
};

// 플레이어가 선택할 수 있는 직업이다.
enum class JobClass
{
    Warrior,
    Mage
};
