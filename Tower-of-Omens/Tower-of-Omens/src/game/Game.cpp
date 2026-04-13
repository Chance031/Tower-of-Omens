#include "game/Game.h"

#include "engine/platform/MenuInput.h"
#include "game/screens/FloorLoopScreen.h"
#include "game/screens/JobSelectScreen.h"
#include "game/screens/MessageScreen.h"
#include "game/screens/TitleScreen.h"

#include <string>

namespace
{
// 선택한 길 종류를 화면에 표시할 이름으로 변환한다.
std::string PathName(PathChoice path)
{
    switch (path)
    {
    case PathChoice::Normal:
        return "일반 던전";
    case PathChoice::Safe:
        return "안정적인 길";
    case PathChoice::Dangerous:
        return "강한 기척";
    case PathChoice::Unknown:
        return "미지의 길";
    }

    return "알 수 없는 길";
}
}

// 게임 객체를 기본 상태로 생성한다.
Game::Game()
    : m_state(GameState::Title)
{
}

// 게임 실행 전 필요한 초기 상태를 준비한다.
void Game::Initialize()
{
    m_state = GameState::Title;
    m_player = {};
    m_renderer.Initialize();
}

// 현재 게임 상태에 맞는 흐름을 실행한다.
void Game::Run()
{
    MenuInput menuInput;
    TitleScreen titleScreen;
    JobSelectScreen jobSelectScreen;
    FloorLoopScreen floorLoopScreen;
    MessageScreen messageScreen;

    while (m_state != GameState::Exit)
    {
        switch (m_state)
        {
        case GameState::Title:
            m_state = titleScreen.Run(m_renderer, menuInput) ? GameState::JobSelect : GameState::Exit;
            break;

        case GameState::JobSelect:
        {
            const auto selectedJob = jobSelectScreen.Run(m_renderer, menuInput);
            if (!selectedJob.has_value())
            {
                m_state = GameState::Title;
                break;
            }

            StartRun(*selectedJob);
            m_state = GameState::FloorLoop;
            break;
        }

        case GameState::FloorLoop:
        {
            const FloorLoopResult result = floorLoopScreen.Run(m_player, m_renderer, menuInput);
            m_state = result.nextState;

            if (result.selectedPath.has_value())
            {
                messageScreen.Show(
                    m_renderer,
                    menuInput,
                    "길 선택",
                    PathName(*result.selectedPath) + "으로 진입한다. 다음 단계에서 이 길에 맞는 전투나 이벤트를 연결할 예정이다.");
                m_state = GameState::FloorLoop;
            }
            break;
        }

        case GameState::GameOver:
            messageScreen.Show(m_renderer, menuInput, "Game Over", "Returning to title...");
            m_state = GameState::Title;
            break;

        case GameState::Clear:
            messageScreen.Show(m_renderer, menuInput, "Clear", "Returning to title...");
            m_state = GameState::Title;
            break;

        case GameState::Exit:
            break;
        }
    }

    m_renderer.Shutdown();
}

// 직업 선택 결과를 바탕으로 플레이어의 시작 능력치를 설정한다.
void Game::StartRun(JobClass job)
{
    m_player = {};
    m_player.job = job;
    m_player.name = JobName(job);
    m_player.floor = 1;
    m_player.gold = 50;

    if (job == JobClass::Warrior)
    {
        m_player.hp = 130;
        m_player.mp = 30;
    }
    else
    {
        m_player.hp = 90;
        m_player.mp = 80;
    }
}

// 직업 enum 값을 화면에 표시할 이름으로 변환한다.
std::string Game::JobName(JobClass job) const
{
    return (job == JobClass::Warrior) ? "전사" : "마법사";
}
