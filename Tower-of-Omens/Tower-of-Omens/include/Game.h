#pragma once

#include "ConsoleRenderer.h"
#include "Enums.h"

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
};

// 게임의 상위 흐름을 관리하는 진입 클래스다.
class Game
{
public:
    Game();

    // 게임 시작 전 상태를 초기화한다.
    void Initialize();

    // 종료 상태가 될 때까지 메인 루프를 실행한다.
    void Run();

private:
    // 현재 게임 상태와 플레이어 데이터를 유지한다.
    GameState m_state;
    Player m_player;
    ConsoleRenderer m_renderer;

    // 각 상위 상태에 대응하는 화면과 처리 함수들이다.
    void ShowTitleScreen();
    void ShowJobSelectScreen();
    void StartRun(JobClass job);
    void RunFloorLoop();
    void ShowGameOverScreen();
    void ShowClearScreen();

    // 공용 입력 보조 함수들이다.
    int PromptMenu(const std::string& title, const std::string& body, const std::string options[], int count) const;
    std::string JobName(JobClass job) const;
};
