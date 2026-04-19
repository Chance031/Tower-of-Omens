#pragma once

// 게임 전체의 상태를 나타낸다.
enum class GameState
{
    Init,       // 초기화
    Title,      // 타이틀 화면
    JobSelect,  // 직업 선택
    FloorSelect,// 층 경로 선택
    Battle,     // 일반/엘리트 전투
    Event,      // 이벤트
    Reward,     // 보상 선택
    Prep,       // 정비 화면
    Boss,       // 보스 전투
    GameOver,   // 게임 오버
    Clear,      // 클리어
    Exit        // 종료
};

// 플레이어가 선택할 수 있는 직업을 나타낸다.
enum class JobClass
{
    Warrior, // 전사
    Mage     // 마법사
};

// 각 층에서 선택할 수 있는 경로 유형을 나타낸다.
enum class PathChoice
{
    Normal,    // 일반 전투
    Dangerous, // 엘리트 전투
    Unknown    // 이벤트
};

// 전투의 종류를 나타낸다.
enum class BattleType
{
    Normal, // 일반 전투
    Elite,  // 엘리트 전투
    Event,  // 이벤트 전투
    Boss    // 보스 전투
};

// 전투 화면이 반환하는 결과를 나타낸다.
enum class BattleResult
{
    Victory, // 승리
    Defeat,  // 패배
    Escape   // 도주
};