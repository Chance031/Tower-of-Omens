#include "game/Game.h"

Game::Game()
    : m_state(GameState::Title)
{
}

void Game::Initialize()
{
    // 새 실행을 시작할 수 있도록 기본 상태로 되돌린다.
    m_state = GameState::Title;
    m_player = {};
    m_renderer.Initialize();
}

void Game::Run()
{
    // 현재 상위 상태에 맞는 처리 객체로 분기한다.
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

void Game::StartRun(JobClass job)
{
    // 직업 선택 결과를 바탕으로 첫 층 진입용 기본 능력치를 세팅한다.
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

std::string Game::JobName(JobClass job) const
{
    return (job == JobClass::Warrior) ? "Warrior" : "Mage";
}
