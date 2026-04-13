#include "game/Game.h"

#include "engine/platform/MenuInput.h"
#include "game/screens/BattleScreen.h"
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
    : m_state(GameState::Title),
      m_pendingBattleType(BattleType::Normal)
{
}

// 게임 실행 전 필요한 초기 상태를 준비한다.
void Game::Initialize()
{
    m_state = GameState::Title;
    m_player = {};
    m_pendingBattleType = BattleType::Normal;
    m_renderer.Initialize();
}

// 현재 게임 상태에 맞는 흐름을 실행한다.
void Game::Run()
{
    MenuInput menuInput;
    TitleScreen titleScreen;
    JobSelectScreen jobSelectScreen;
    FloorLoopScreen floorLoopScreen;
    BattleScreen battleScreen;
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
                m_pendingBattleType = DetermineBattleType(*result.selectedPath);
                messageScreen.Show(
                    m_renderer,
                    menuInput,
                    "길 선택",
                    PathName(*result.selectedPath) + "으로 진입한다. 전투 화면으로 이동한다.");
                m_state = GameState::Battle;
            }
            break;
        }

        case GameState::Battle:
        {
            const Enemy enemy = CreateEnemy(m_pendingBattleType);
            const BattleResult result = battleScreen.Run(m_player, enemy, m_pendingBattleType, m_renderer, menuInput);

            if (result == BattleResult::Defeat)
            {
                m_state = GameState::GameOver;
                break;
            }

            if (result == BattleResult::Escape)
            {
                messageScreen.Show(m_renderer, menuInput, "전투 이탈", "전투 화면 뼈대에서는 도주 시 타이틀로 돌아간다.");
                m_state = GameState::Title;
                break;
            }

            ++m_player.floor;
            messageScreen.Show(m_renderer, menuInput, "전투 결과", "전투에서 승리했다. 다음 층으로 이동한다.");
            m_state = GameState::FloorLoop;
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
        m_player.atk = 15;
        m_player.def = 10;
        m_player.maxHp = 130;
        m_player.maxMp = 30;
    }
    else
    {
        m_player.atk = 25;
        m_player.def = 4;
        m_player.maxHp = 90;
        m_player.maxMp = 80;
    }

    m_player.hp = m_player.maxHp;
    m_player.mp = m_player.maxMp;
}

// 직업 enum 값을 화면에 표시할 이름으로 변환한다.
std::string Game::JobName(JobClass job) const
{
    return (job == JobClass::Warrior) ? "전사" : "마법사";
}

// 길 선택 결과를 바탕으로 다음 전투 종류를 결정한다.
BattleType Game::DetermineBattleType(PathChoice path) const
{
    switch (path)
    {
    case PathChoice::Normal:
    case PathChoice::Safe:
        return BattleType::Normal;
    case PathChoice::Dangerous:
        return BattleType::Elite;
    case PathChoice::Unknown:
        return BattleType::Event;
    }

    return BattleType::Normal;
}

// 전투 종류에 맞는 임시 적 데이터를 생성한다.
Enemy Game::CreateEnemy(BattleType battleType) const
{
    switch (battleType)
    {
    case BattleType::Normal:
        return {"슬라임", 30, 6};
    case BattleType::Elite:
        return {"오우거", 60, 14};
    case BattleType::Event:
        return {"그림자 환영", 45, 10};
    }

    return {"슬라임", 30, 6};
}
