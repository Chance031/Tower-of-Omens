#include "game/Game.h"

#include "engine/platform/MenuInput.h"
#include "game/screens/BattleScreen.h"
#include "game/screens/FloorLoopScreen.h"
#include "game/screens/JobSelectScreen.h"
#include "game/screens/MaintenanceScreen.h"
#include "game/screens/MessageScreen.h"
#include "game/screens/TitleScreen.h"

#include <algorithm>
#include <sstream>
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
      m_pendingBattleType(BattleType::Normal),
      m_pendingPathChoice(PathChoice::Normal)
{
}

// 게임 실행 전 필요한 초기 상태를 준비한다.
void Game::Initialize()
{
    m_state = GameState::Title;
    m_player = {};
    m_pendingBattleType = BattleType::Normal;
    m_pendingPathChoice = PathChoice::Normal;
    m_renderer.Initialize();
}

// 현재 게임 상태에 맞는 흐름을 실행한다.
void Game::Run()
{
    MenuInput menuInput;
    TitleScreen titleScreen;
    JobSelectScreen jobSelectScreen;
    MaintenanceScreen maintenanceScreen;
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
            m_pendingPathChoice = PathChoice::Normal;
            m_pendingBattleType = BattleType::Normal;
            m_state = GameState::Battle;
            break;
        }

        case GameState::Maintenance:
        {
            const MaintenanceResult result = maintenanceScreen.Run(m_player, m_renderer, menuInput);
            if (!result.summary.empty())
            {
                messageScreen.Show(m_renderer, menuInput, "정비 결과", result.summary);
            }
            m_state = result.nextState;
            break;
        }

        case GameState::FloorLoop:
        {
            const FloorLoopResult result = floorLoopScreen.Run(m_player, m_renderer, menuInput);
            m_state = result.nextState;

            if (m_player.floor >= 10)
            {
                m_pendingPathChoice = PathChoice::Normal;
                m_pendingBattleType = BattleType::Boss;
                messageScreen.Show(m_renderer, menuInput, "10층", "더 이상 길을 고를 수 없다. 보스전이 시작된다.");
                m_state = GameState::Battle;
                break;
            }

            if (result.selectedPath.has_value())
            {
                m_pendingPathChoice = *result.selectedPath;
                m_pendingBattleType = DetermineBattleType(*result.selectedPath);
                messageScreen.Show(
                    m_renderer,
                    menuInput,
                    "길 선택",
                    PathName(*result.selectedPath) + "으로 진입한다. 다음 전투를 준비한다.");
                m_state = GameState::Battle;
            }
            break;
        }

        case GameState::Battle:
        {
            const Enemy enemy = m_enemyFactory.Create(m_pendingBattleType, m_pendingPathChoice, m_player.floor);
            const BattleResult result = battleScreen.Run(m_player, enemy, m_pendingBattleType, m_renderer, menuInput);

            if (result == BattleResult::Defeat)
            {
                m_state = GameState::GameOver;
                break;
            }

            if (result == BattleResult::Escape)
            {
                if (m_pendingBattleType != BattleType::Boss)
                {
                    ++m_player.floor;
                }

                messageScreen.Show(m_renderer, menuInput, "전투 이탈", "보상은 얻지 못했지만 목숨은 건졌다. 정비층으로 이동한다.");
                m_state = GameState::Maintenance;
                break;
            }

            if (m_pendingBattleType == BattleType::Boss)
            {
                messageScreen.Show(m_renderer, menuInput, "보스 격파", "심연의 징조를 쓰러뜨렸다. 탑의 정상에 도달했다.");
                m_state = GameState::Clear;
                break;
            }

            ++m_player.floor;
            messageScreen.Show(m_renderer, menuInput, "전투 결과", ApplyBattleReward(enemy, m_pendingBattleType, m_pendingPathChoice));
            m_state = GameState::Maintenance;
            break;
        }

        case GameState::GameOver:
            messageScreen.Show(m_renderer, menuInput, "Game Over", "Returning to title...");
            m_state = GameState::Title;
            break;

        case GameState::Clear:
            messageScreen.Show(m_renderer, menuInput, "Clear", "탑을 정복했다. 타이틀로 돌아간다.");
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
        m_player.potionCount = 2;
        m_player.etherCount = 1;
    }
    else
    {
        m_player.atk = 25;
        m_player.def = 4;
        m_player.maxHp = 90;
        m_player.maxMp = 80;
        m_player.potionCount = 1;
        m_player.etherCount = 2;
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

// 승리 후 직업별 성장치를 적용한다.
std::string Game::ApplyJobGrowth()
{
    std::ostringstream growth;

    if (m_player.job == JobClass::Warrior)
    {
        m_player.maxHp += 8;
        m_player.hp = std::min(m_player.maxHp, m_player.hp + 8);
        ++m_player.def;
        growth << "\n전사의 성장으로 최대 HP가 8, DEF가 1 상승했다.";

        if (m_player.floor == 5)
        {
            growth << "\n5층의 벽을 넘으며 전사의 두 번째 기술 '파쇄 돌격'이 해금되었다.";
        }
    }
    else
    {
        m_player.maxMp += 8;
        m_player.mp = std::min(m_player.maxMp, m_player.mp + 8);
        ++m_player.atk;
        growth << "\n마법사의 성장으로 최대 MP가 8, ATK가 1 상승했다.";

        if (m_player.floor == 5)
        {
            growth << "\n5층의 벽을 넘으며 마법사의 두 번째 기술 '운석 낙하'가 해금되었다.";
        }
    }

    return growth.str();
}

// 승리한 전투의 보상을 플레이어에게 반영한다.
std::string Game::ApplyBattleReward(const Enemy& enemy, BattleType battleType, PathChoice path)
{
    m_player.gold += enemy.goldReward;

    std::ostringstream message;
    message << enemy.name << "을(를) 쓰러뜨렸다. Gold " << enemy.goldReward << "을 획득했다.";

    if (path == PathChoice::Unknown || battleType == BattleType::Event)
    {
        m_player.hp = std::min(m_player.maxHp, m_player.hp + 10);
        m_player.mp = std::min(m_player.maxMp, m_player.mp + 10);
        message << "\n미지의 길의 여파로 HP와 MP를 10 회복했다.";
    }
    else if (path == PathChoice::Dangerous || battleType == BattleType::Elite)
    {
        message << "\n강한 기척을 돌파해 더 많은 보상을 얻었다.";
    }
    else if (path == PathChoice::Safe)
    {
        message << "\n안정적인 길답게 무난한 보상을 챙겼다.";
    }
    else
    {
        message << "\n첫 전투를 마치고 정비층으로 향한다.";
    }

    message << ApplyJobGrowth();
    return message.str();
}

