#include "game/Game.h"

#include "engine/platform/MenuInput.h"
#include "game/CsvUtils.h"
#include "game/screens/BattleScreen.h"
#include "game/screens/EventScreen.h"
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

std::string ResolveEnemyBaseCsvPath()
{
    return csv::ResolveCsvPath("enemy_base.csv");
}

void SetConsumableCount(Player& player, const std::string& id, int count)
{
    for (ConsumableStack& stack : player.consumables)
    {
        if (stack.id == id)
        {
            stack.count = count;
            return;
        }
    }

    player.consumables.push_back({id, count});
}

void AddConsumableCount(Player& player, const std::string& id, int amount)
{
    for (ConsumableStack& stack : player.consumables)
    {
        if (stack.id == id)
        {
            stack.count += amount;
            return;
        }
    }

    player.consumables.push_back({id, amount});
}


std::string PathName(PathChoice path)
{
    switch (path)
    {
    case PathChoice::Normal:
        return "일반 전투";

    case PathChoice::Dangerous:
        return "엘리트 전투";
    case PathChoice::Unknown:
        return "이벤트";
    }

    return "알 수 없는 길";
}

std::string PathTransitionMessage(PathChoice path, BattleType battleType)
{
    switch (battleType)
    {
    case BattleType::Elite:
        return PathName(path) + "으로 진입한다. 강한 적의 기척이 느껴진다.";
    case BattleType::Event:
        return PathName(path) + "으로 진입한다. 수상한 기운이 감돈다.";
    case BattleType::Boss:
        return "탑의 최상층이다. 심연의 징조가 기다리고 있다.";
    case BattleType::Normal:
    default:
        return PathName(path) + "으로 진입한다. 다음 전투를 준비한다.";
    }
}

int RandomIndex(int minValue, int maxValue)
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(generator);
}

std::vector<RewardItem> BuildRewardPool()
{
    return {
        {"회복약 꾸러미", "회복약 2개를 획득한다."},
        {"마나약 꾸러미", "마나약 2개를 획득한다."},
        {"힘의 파편", "STR을 1 상승시킨다."},
        {"민첩의 파편", "AGI를 1 상승시킨다."},
        {"지능의 파편", "INT를 1 상승시킨다."},
        {"정신의 파편", "MND를 1 상승시킨다."}
    };
}

std::vector<RelicDefinition> BuildRelicPool()
{
    return {
        {"붉은 전쟁문장", "공격력이 3 상승한다."},
        {"수호자의 매듭", "방어력이 3 상승한다."},
        {"생명의 구슬", "최대 HP가 20 상승하고 HP를 20 회복한다."},
        {"마력의 수정", "최대 MP가 18 상승하고 MP를 18 회복한다."}
    };
}

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

std::string ComposeGameOverBody(const Player& player)
{
    std::ostringstream body;
    body << "[탐험 종료]\n";
    body << player.name << " | 도달 층 " << player.floor << " | 레벨 " << player.level << '\n';
    body << "보유 Gold " << player.gold << " | 유물 " << player.relicNames.size() << "개\n\n";
    body << "심연의 탑에서 쓰러졌다.\n";
    body << "직업 선택으로 돌아가 다시 도전하거나 타이틀로 복귀할 수 있다.\n";
    return body.str();
}

std::string ApplyRewardItem(Player& player, const RewardItem& reward)
{
    if (reward.name == "회복약 꾸러미")
    {
        player.potionCount += 2;
        AddConsumableCount(player, "201", 2);
        return "회복약 2개를 획득했다.";
    }

    if (reward.name == "마나약 꾸러미")
    {
        player.etherCount += 2;
        AddConsumableCount(player, "203", 2);
        return "마나약 2개를 획득했다.";
    }

    if (reward.name == "힘의 파편")
    {
        ++player.strength;
        RefreshDerivedStats(player);
        return "STR이 1 상승했다.";
    }

    if (reward.name == "민첩의 파편")
    {
        ++player.agility;
        RefreshDerivedStats(player);
        return "AGI가 1 상승했다.";
    }

    if (reward.name == "지능의 파편")
    {
        ++player.intelligence;
        RefreshDerivedStats(player);
        return "INT가 1 상승했다.";
    }

    ++player.spirit;
    RefreshDerivedStats(player);
    return "MND가 1 상승했다.";
}

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
        player.bonusAttackPower += 3;
    }
    else if (relic.name == "수호자의 매듭")
    {
        player.bonusDefense += 3;
    }
    else if (relic.name == "생명의 구슬")
    {
        player.bonusMaxHp += 20;
    }
    else
    {
        player.bonusMaxMp += 18;
    }

    RefreshDerivedStats(player);

    return "유물 획득: " + relic.name + " - " + relic.description;
}
}

Game::Game()
    : m_state(GameState::Init),
      m_pendingBattleType(BattleType::Normal),
      m_pendingPathChoice(PathChoice::Normal),
      m_lastBattleType(BattleType::Normal),
      m_lastResolvedPathChoice(PathChoice::Normal)
{
}

void Game::Initialize()
{
    m_state = GameState::Init;
    m_player = {};
    m_pendingBattleType = BattleType::Normal;
    m_pendingPathChoice = PathChoice::Normal;
    m_lastEnemy = {};
    m_lastBattleType = BattleType::Normal;
    m_lastResolvedPathChoice = PathChoice::Normal;
    m_renderer.Initialize();
}

void Game::Run()
{
    MenuInput menuInput;
    TitleScreen titleScreen;
    JobSelectScreen jobSelectScreen;
    MaintenanceScreen maintenanceScreen;
    FloorLoopScreen floorLoopScreen;
    BattleScreen battleScreen;
    EventScreen eventScreen;
    MessageScreen messageScreen;

    while (m_state != GameState::Exit)
    {
        switch (m_state)
        {
        case GameState::Init:
            m_state = GameState::Title;
            break;

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

        case GameState::FloorSelect:
        {
            if (m_player.floor >= 10)
            {
                m_pendingPathChoice = PathChoice::Normal;
                m_pendingBattleType = BattleType::Boss;
                messageScreen.Show(m_renderer, menuInput, "10층", PathTransitionMessage(PathChoice::Normal, BattleType::Boss));
                m_state = GameState::Boss;
                break;
            }

            const FloorLoopResult result = floorLoopScreen.Run(m_player, m_renderer, menuInput);
            if (!result.selectedPath.has_value())
            {
                m_state = result.nextState;
                break;
            }

            m_pendingPathChoice = *result.selectedPath;
            m_pendingBattleType = DetermineBattleType(*result.selectedPath);
            messageScreen.Show(m_renderer, menuInput, "길 선택", PathTransitionMessage(*result.selectedPath, m_pendingBattleType));
            m_state = (m_pendingBattleType == BattleType::Event) ? GameState::Event : GameState::Battle;
            break;
        }

        case GameState::Battle:
            m_state = RunEncounterState(m_pendingBattleType, battleScreen, messageScreen, menuInput);
            break;

        case GameState::Event:
            m_state = eventScreen.Run(m_player, messageScreen, m_renderer, menuInput);
            break;

        case GameState::Reward:
            ResolveBattleReward(m_lastEnemy, m_lastBattleType, m_lastResolvedPathChoice, menuInput);
            m_state = GameState::Prep;
            break;

        case GameState::Prep:
        {
            const MaintenanceResult result = maintenanceScreen.Run(m_player, m_renderer, menuInput);
            if (!result.summary.empty())
            {
                messageScreen.Show(m_renderer, menuInput, "정비 결과", result.summary);
            }
            m_state = result.nextState;
            break;
        }

        case GameState::Boss:
            m_state = RunEncounterState(BattleType::Boss, battleScreen, messageScreen, menuInput);
            break;

        case GameState::GameOver:
            m_state = RunGameOverScreen(menuInput);
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
        m_player.strength = 14;
        m_player.agility = 11;
        m_player.intelligence = 8;
        m_player.spirit = 9;
        m_player.potionCount = 2;
        m_player.etherCount = 1;
        m_player.weaponName = "훈련용 검";
        m_player.weaponAtkBonus = 3;
        m_player.armorName = "견습 방어구";
        m_player.armorDefBonus = 2;
    }
    else
    {
        m_player.strength = 8;
        m_player.agility = 10;
        m_player.intelligence = 14;
        m_player.spirit = 12;
        m_player.potionCount = 1;
        m_player.etherCount = 2;
        m_player.weaponName = "초심자 지팡이";
        m_player.weaponAtkBonus = 5;
        m_player.armorName = "견습 로브";
        m_player.armorDefBonus = 3;
    }

    RefreshDerivedStats(m_player, true);
    m_player.consumables.clear();
    SetConsumableCount(m_player, "201", m_player.potionCount);
    SetConsumableCount(m_player, "203", m_player.etherCount);
}

std::string Game::JobName(JobClass job) const
{
    return (job == JobClass::Warrior) ? "전사" : "마법사";
}

BattleType Game::DetermineBattleType(PathChoice path) const
{
    switch (path)
    {
    case PathChoice::Normal:
        return BattleType::Normal;
    case PathChoice::Dangerous:
        return BattleType::Elite;
    case PathChoice::Unknown:
        return BattleType::Event;
    }

    return BattleType::Normal;
}

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
        growth << "\n마법사의 두 번째 기술 '운석 낙하'이 해금되었다.";
    }

    return growth.str();
}

GameState Game::RunEncounterState(
    BattleType battleType,
    BattleScreen& battleScreen,
    MessageScreen& messageScreen,
    const MenuInput& input)
{
    const Enemy enemy = m_enemyFactory.Create(battleType, m_pendingPathChoice, m_player.floor);
    const std::unordered_map<int, EnemyIntentData> intentMap = LoadEnemyIntents(ResolveEnemyBaseCsvPath());
    const BattleResult result = battleScreen.Run(m_player, enemy, battleType, intentMap, m_renderer, input);

    if (result == BattleResult::Defeat)
    {
        return GameState::GameOver;
    }

    if (result == BattleResult::Escape)
    {
        if (battleType != BattleType::Boss)
        {
            ++m_player.floor;
        }

        messageScreen.Show(m_renderer, input, "전투 이탈", "보상은 얻지 못했지만 목숨은 건졌다. 정비층으로 이동한다.");
        return GameState::Prep;
    }

    if (battleType == BattleType::Boss)
    {
        messageScreen.Show(m_renderer, input, "보스 격파", "심연의 징조를 쓰러뜨렸다. 탑의 정상에 도달했다.");
        return GameState::Clear;
    }

    ++m_player.floor;
    m_lastEnemy = enemy;
    m_lastBattleType = battleType;
    m_lastResolvedPathChoice = m_pendingPathChoice;
    return GameState::Reward;
}

GameState Game::RunGameOverScreen(const MenuInput& input)
{
    const std::vector<std::string> options = {"다시 도전", "타이틀로"};
    int selected = 0;

    for (;;)
    {
        m_renderer.Present(m_renderer.ComposeMenuFrame("Game Over", ComposeGameOverBody(m_player), options, selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type == MenuResultType::Cancel)
        {
            return GameState::Title;
        }

        return (action.index == 0) ? GameState::JobSelect : GameState::Title;
    }
}

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
