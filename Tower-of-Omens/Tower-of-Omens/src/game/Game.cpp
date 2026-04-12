#include "game/Game.h"

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
    while (m_state != GameState::Exit)
    {
        switch (m_state)
        {
        case GameState::Title:
            m_state = m_titleScreen.Run(m_renderer, m_menuInput) ? GameState::JobSelect : GameState::Exit;
            break;

        case GameState::JobSelect:
        {
            const auto selectedJob = m_jobSelectScreen.Run(m_renderer, m_menuInput);
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
            m_state = m_floorLoopScreen.Run(m_player, m_renderer, m_menuInput) ? GameState::Title : GameState::Exit;
            break;

        case GameState::GameOver:
            m_messageScreen.Show(m_renderer, m_menuInput, "Game Over", "Returning to title...");
            m_state = GameState::Title;
            break;

        case GameState::Clear:
            m_messageScreen.Show(m_renderer, m_menuInput, "Clear", "Returning to title...");
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
    return (job == JobClass::Warrior) ? "Warrior" : "Mage";
}
