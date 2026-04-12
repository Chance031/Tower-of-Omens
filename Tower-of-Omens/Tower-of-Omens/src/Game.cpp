#include "Game.h"

#include <conio.h>
#include <iostream>
#include <sstream>

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
    // 현재 상위 상태에 맞는 처리 함수로 분기한다.
    while (m_state != GameState::Exit)
    {
        switch (m_state)
        {
        case GameState::Title:
            ShowTitleScreen();
            break;
        case GameState::JobSelect:
            ShowJobSelectScreen();
            break;
        case GameState::FloorLoop:
            RunFloorLoop();
            break;
        case GameState::GameOver:
            ShowGameOverScreen();
            break;
        case GameState::Clear:
            ShowClearScreen();
            break;
        case GameState::Exit:
            break;
        }
    }

    m_renderer.Shutdown();
}

void Game::ShowTitleScreen()
{
    const std::string options[] = {"New Game", "Exit"};

    std::ostringstream body;
    body << "====================================\n";
    body << "        Tower of Omens\n";
    body << "====================================\n";
    body << "Use Up/Down arrows, Enter to confirm, Esc to go back.\n";

    const int choice = PromptMenu("Title", body.str(), options, 2);
    m_state = (choice == 1) ? GameState::JobSelect : GameState::Exit;
}

void Game::ShowJobSelectScreen()
{
    const std::string options[] =
    {
        "Warrior",
        "Mage"
    };

    const std::string body = "Choose a job for the first run.\n";
    const int choice = PromptMenu("Choose Job", body, options, 2);
    if (choice == 0)
    {
        m_state = GameState::Title;
        return;
    }

    StartRun(choice == 1 ? JobClass::Warrior : JobClass::Mage);
    m_state = GameState::FloorLoop;
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

void Game::RunFloorLoop()
{
    // 다음 단계에서는 이 지점에 층 진행의 세부 시스템을 연결한다.
    std::ostringstream body;
    body << "Player: " << m_player.name << '\n';
    body << "Floor: " << m_player.floor << '\n';
    body << "HP: " << m_player.hp << " | MP: " << m_player.mp << " | Gold: " << m_player.gold << "\n\n";
    body << "Next step: battle / event / reward / preparation will be added next.\n";

    const std::string options[] =
    {
        "Back to Title",
        "Exit"
    };

    const int choice = PromptMenu("Floor Loop", body.str(), options, 2);
    if (choice == 0 || choice == 1)
    {
        m_state = GameState::Title;
    }
    else
    {
        m_state = GameState::Exit;
    }
}

void Game::ShowGameOverScreen()
{
    m_renderer.Present("[Game Over]\nReturning to title...\n");
    m_state = GameState::Title;
}

void Game::ShowClearScreen()
{
    m_renderer.Present("[Clear]\nReturning to title...\n");
    m_state = GameState::Title;
}

int Game::PromptMenu(const std::string& title, const std::string& body, const std::string options[], int count) const
{
    // 방향키와 Enter, Esc로 메뉴 선택을 처리한다.
    int selected = 0;

    for (;;)
    {
        std::ostringstream frame;
        frame << '[' << title << "]\n";
        frame << body;
        frame << "\nUp/Down: Move  Enter: Select  Esc: Back\n\n";

        for (int i = 0; i < count; ++i)
        {
            frame << ((i == selected) ? "> " : "  ");
            frame << options[i] << '\n';
        }

        m_renderer.Present(frame.str());

        const int key = _getch();
        if (key == 13)
        {
            return selected + 1;
        }

        if (key == 27)
        {
            return 0;
        }

        if (key == 0 || key == 224)
        {
            const int extended = _getch();
            if (extended == 72)
            {
                selected = (selected - 1 + count) % count;
            }
            else if (extended == 80)
            {
                selected = (selected + 1) % count;
            }
        }
    }
}

std::string Game::JobName(JobClass job) const
{
    return (job == JobClass::Warrior) ? "Warrior" : "Mage";
}
