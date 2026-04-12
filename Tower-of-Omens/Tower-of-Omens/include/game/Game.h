#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"
#include "game/screens/FloorLoopScreen.h"
#include "game/screens/JobSelectScreen.h"
#include "game/screens/MessageScreen.h"
#include "game/screens/TitleScreen.h"

#include <string>

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
    MenuInput m_menuInput;
    TitleScreen m_titleScreen;
    JobSelectScreen m_jobSelectScreen;
    FloorLoopScreen m_floorLoopScreen;
    MessageScreen m_messageScreen;

    // 실행 시작 시 플레이어의 기본 능력치를 세팅한다.
    void StartRun(JobClass job);

    // 직업 enum을 출력용 이름으로 변환한다.
    std::string JobName(JobClass job) const;
};
