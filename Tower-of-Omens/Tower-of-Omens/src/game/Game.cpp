#include "game/Game.h"

#include "engine/platform/MenuInput.h"
#include "game/screens/BattleScreen.h"
#include "game/screens/FloorLoopScreen.h"
#include "game/screens/JobSelectScreen.h"
#include "game/screens/MaintenanceScreen.h"
#include "game/screens/MessageScreen.h"
#include "game/screens/TitleScreen.h"

#include <algorithm>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace
{
struct RewardItem
{
    std::string name;
    std::string description;
};

struct RelicDefinition
{
    std::string name;
    std::string description;
};

// 선택한 길 종류를 화면에 표시할 이름으로 변환한다.
std::string PathName(PathChoice path)
{
    switch (path)
    {
    case PathChoice::Normal:
        return "일반 전투";
    case PathChoice::Safe:
        return "안정적인 길";
    case PathChoice::Dangerous:
        return "엘리트 전투";
    case PathChoice::Unknown:
        return "이벤트 전투";
    }

    return "알 수 없는 길";
}

// 보상과 유물 선택에 사용할 무작위 정수 값을 만든다.
int RandomIndex(int minValue, int maxValue)
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(generator);
}

// 일반 전투 보상 후보 풀을 만든다.
std::vector<RewardItem> BuildRewardPool()
{
    return {
        {"회복약 꾸러미", "회복약 2개를 획득한다."},
        {"마나약 꾸러미", "마나약 2개를 획득한다."},
        {"강화 숫돌", "공격력이 2 상승한다."},
        {"수호 부적", "방어력이 2 상승한다."},
        {"생명의 파편", "최대 HP가 12 상승하고 HP를 12 회복한다."},
        {"마력의 파편", "최대 MP가 10 상승하고 MP를 10 회복한다."}
    };
}

// 엘리트 보상으로 사용할 유물 풀을 만든다.
std::vector<RelicDefinition> BuildRelicPool()
{
    return {
        {"붉은 전쟁문장", "공격력이 3 상승한다."},
        {"수호자의 매듭", "방어력이 3 상승한다."},
        {"생명의 구슬", "최대 HP가 20 상승하고 HP를 20 회복한다."},
        {"마력의 수정", "최대 MP가 18 상승하고 MP를 18 회복한다."}
    };
}

// 전리품 선택 화면의 본문을 만든다.
std::string ComposeRewardBody(
    const Player& player,
    const Enemy& enemy,
    BattleType battleType,
    const RewardItem& selectedReward)
{
    std::ostringstream body;
    body << "[전투 승리]\n";
    body << enemy.name << "을(를) 쓰러뜨렸다.\n";
    body << "획득 Gold: " << enemy.goldReward << "\n";
    body << "현재 Gold: " << player.gold << "\n\n";
    body << "[선택한 전리품]\n";
    body << selectedReward.name << '\n';
    body << selectedReward.description << "\n\n";
    if (battleType == BattleType::Elite)
    {
        body << "엘리트 전투를 돌파하면 선택 보상 외에도 유물 1종을 추가로 획득한다.\n";
    }
    else
    {
        body << "전리품 3종 중 하나를 선택해 다음 탐험을 준비한다.\n";
    }
    return body.str();
}

// 적용된 전리품 결과를 요약한다.
std::string ApplyRewardItem(Player& player, const RewardItem& reward)
{
    if (reward.name == "회복약 꾸러미")
    {
        player.potionCount += 2;
        return "회복약 2개를 획득했다.";
    }

    if (reward.name == "마나약 꾸러미")
    {
        player.etherCount += 2;
        return "마나약 2개를 획득했다.";
    }

    if (reward.name == "강화 숫돌")
    {
        player.atk += 2;
        return "공격력이 2 상승했다.";
    }

    if (reward.name == "수호 부적")
    {
        player.def += 2;
        return "방어력이 2 상승했다.";
    }

    if (reward.name == "생명의 파편")
    {
        player.maxHp += 12;
        player.hp = std::min(player.maxHp, player.hp + 12);
        return "최대 HP가 12 상승하고 HP를 12 회복했다.";
    }

    player.maxMp += 10;
    player.mp = std::min(player.maxMp, player.mp + 10);
    return "최대 MP가 10 상승하고 MP를 10 회복했다.";
}

// 유물 효과를 플레이어에게 적용한다.
std::string ApplyRelicReward(Player& player)
{
    std::vector<RelicDefinition> availableRelics;
    for (const RelicDefinition& relic : BuildRelicPool())
    {
        if (std::find(player.relicNames.begin(), player.relicNames.end(), relic.name) == player.relicNames.end())
        {
            availableRelics.push_back(relic);
        }
    }

    if (availableRelics.empty())
    {
        player.gold += 30;
        return "이미 모든 유물을 보유하고 있어 추가 Gold 30을 획득했다.";
    }

    const RelicDefinition& relic = availableRelics[RandomIndex(0, static_cast<int>(availableRelics.size()) - 1)];
    player.relicNames.push_back(relic.name);

    if (relic.name == "붉은 전쟁문장")
    {
        player.atk += 3;
    }
    else if (relic.name == "수호자의 매듭")
    {
        player.def += 3;
    }
    else if (relic.name == "생명의 구슬")
    {
        player.maxHp += 20;
        player.hp = std::min(player.maxHp, player.hp + 20);
    }
    else
    {
        player.maxMp += 18;
        player.mp = std::min(player.maxMp, player.mp + 18);
    }

    return "유물 '" + relic.name + "' 획득: " + relic.description;
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
            ResolveBattleReward(enemy, m_pendingBattleType, m_pendingPathChoice, menuInput);
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
    m_player.level = 1;
    m_player.statPoints = 0;
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
        m_player.weaponName = "훈련용 검";
        m_player.armorName = "견습 방어구";
    }
    else
    {
        m_player.atk = 25;
        m_player.def = 4;
        m_player.maxHp = 90;
        m_player.maxMp = 80;
        m_player.potionCount = 1;
        m_player.etherCount = 2;
        m_player.weaponName = "초심자 지팡이";
        m_player.armorName = "견습 로브";
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
    ++m_player.level;
    m_player.statPoints += 3;

    std::ostringstream growth;
    growth << "\n레벨이 " << m_player.level << "이(가) 되었고, 분배 가능한 스탯 포인트 3을 획득했다.";

    if (m_player.job == JobClass::Warrior && m_player.level == 5)
    {
        growth << "\n전사의 두 번째 기술 '파쇄 돌격'이 해금되었다.";
    }
    else if (m_player.job == JobClass::Mage && m_player.level == 5)
    {
        growth << "\n마법사의 두 번째 기술 '운석 낙하'가 해금되었다.";
    }

    return growth.str();
}

// 승리한 전투의 보상을 플레이어에게 반영하고 요약을 만든다.
std::string Game::ResolveBattleReward(const Enemy& enemy, BattleType battleType, PathChoice path, const MenuInput& input)
{
    m_player.gold += enemy.goldReward;

    std::vector<RewardItem> rewardPool = BuildRewardPool();
    for (std::size_t i = rewardPool.size(); i > 1; --i)
    {
        const int swapIndex = RandomIndex(0, static_cast<int>(i) - 1);
        std::swap(rewardPool[i - 1], rewardPool[swapIndex]);
    }
    rewardPool.resize(3);

    std::vector<std::string> rewardOptions;
    for (const RewardItem& reward : rewardPool)
    {
        rewardOptions.push_back(reward.name);
    }

    int selected = 0;
    for (;;)
    {
        m_renderer.Present(m_renderer.ComposeMenuFrame(
            "전리품 선택",
            ComposeRewardBody(m_player, enemy, battleType, rewardPool[selected]),
            rewardOptions,
            selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(rewardOptions.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type != MenuResultType::Confirm)
        {
            continue;
        }

        break;
    }

    std::ostringstream summary;
    summary << enemy.name << "을(를) 쓰러뜨리고 Gold " << enemy.goldReward << "을 획득했다.";
    summary << "\n선택 보상: " << rewardPool[selected].name << " - " << ApplyRewardItem(m_player, rewardPool[selected]);

    if (path == PathChoice::Unknown || battleType == BattleType::Event)
    {
        m_player.hp = std::min(m_player.maxHp, m_player.hp + 10);
        m_player.mp = std::min(m_player.maxMp, m_player.mp + 10);
        summary << "\n이벤트 여파로 HP와 MP를 10 회복했다.";
    }

    if (battleType == BattleType::Elite)
    {
        summary << "\n" << ApplyRelicReward(m_player);
    }

    summary << ApplyJobGrowth();

    m_renderer.Present(m_renderer.ComposeMenuFrame("전투 보상", summary.str() + "\n\n정비층으로 이동한다.", {}, 0));
    input.WaitForAnyKey();
    return summary.str();
}
