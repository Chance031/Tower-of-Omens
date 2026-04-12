#pragma once

// 게임 전반에서 공통으로 사용하는 enum 타입을 모아 둔 헤더다.
// 다른 시스템에서도 가볍게 포함할 수 있도록 최소 선언만 유지한다.

// 게임의 큰 흐름을 나타내는 상위 상태다.
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
