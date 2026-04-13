#pragma once

// Tower of Omens에서 사용하는 상위 게임 상태다.
enum class GameState
{
    Title,
    JobSelect,
    Maintenance,
    FloorLoop,
    Battle,
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

// 층에서 선택할 수 있는 길의 종류다.
enum class PathChoice
{
    Normal,
    Safe,
    Dangerous,
    Unknown
};

// 전투의 종류를 나타낸다.
enum class BattleType
{
    Normal,
    Elite,
    Event,
    Boss
};

// 전투 화면이 돌려주는 결과를 나타낸다.
enum class BattleResult
{
    Victory,
    Defeat,
    Escape
};
