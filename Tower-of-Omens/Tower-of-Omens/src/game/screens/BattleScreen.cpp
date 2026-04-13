#include "game/screens/BattleScreen.h"

#include <algorithm>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace
{
struct SkillDefinition
{
    std::string name;
    std::string description;
    int mpCost = 0;
    int attackBonus = 0;
    bool grantsGuard = false;
};

struct ItemDefinition
{
    std::string name;
    std::string description;
    int count = 0;
};

enum class EnemyIntent
{
    Attack,
    Guard,
    Recover
};

std::string BattleTypeName(BattleType battleType)
{
    switch (battleType)
    {
    case BattleType::Normal:
        return "일반 전투";
    case BattleType::Elite:
        return "엘리트 전투";
    case BattleType::Event:
        return "특수 전투";
    case BattleType::Boss:
        return "보스 전투";
    }

    return "전투";
}

std::string PassiveName(JobClass job)
{
    return (job == JobClass::Warrior) ? "불굴" : "마력 순환";
}

std::string PassiveDescription(JobClass job)
{
    return (job == JobClass::Warrior)
        ? "받는 피해가 항상 2 감소한다."
        : "행동 후 MP를 3 회복한다.";
}

bool HasObservationRelic(const Player& player)
{
    return std::find(player.relicNames.begin(), player.relicNames.end(), "관찰 유물") != player.relicNames.end();
}

std::string EnemyIntentName(EnemyIntent intent)
{
    switch (intent)
    {
    case EnemyIntent::Attack:
        return "공격";
    case EnemyIntent::Guard:
        return "방어";
    case EnemyIntent::Recover:
        return "회복";
    }

    return "알 수 없음";
}

std::string EnemyIntentDescription(EnemyIntent intent)
{
    switch (intent)
    {
    case EnemyIntent::Attack:
        return "다음 턴에 플레이어를 노리고 공격을 준비 중이다.";
    case EnemyIntent::Guard:
        return "다음 턴에 몸을 웅크리고 피해를 줄일 생각이다.";
    case EnemyIntent::Recover:
        return "다음 턴에 숨을 고르며 체력을 회복하려 한다.";
    }

    return "의도를 읽을 수 없다.";
}

std::string MakeBar(int current, int maximum, int width, char filled, char empty)
{
    if (maximum <= 0)
    {
        return std::string(width, empty);
    }

    const int clampedCurrent = std::max(0, std::min(current, maximum));
    const int filledCount = (clampedCurrent * width) / maximum;
    return std::string(filledCount, filled) + std::string(width - filledCount, empty);
}

int RandomPercent()
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    static std::uniform_int_distribution<int> distribution(1, 100);
    return distribution(generator);
}

EnemyIntent RollEnemyIntent(int enemyHp, int enemyMaxHp, BattleType battleType)
{
    const int hpRate = (enemyMaxHp <= 0) ? 100 : (enemyHp * 100) / enemyMaxHp;
    const int roll = RandomPercent();

    if (hpRate <= 35)
    {
        if (roll <= 30)
        {
            return EnemyIntent::Recover;
        }
        if (roll <= 55)
        {
            return EnemyIntent::Guard;
        }
        return EnemyIntent::Attack;
    }

    if (battleType == BattleType::Boss)
    {
        if (roll <= 20)
        {
            return EnemyIntent::Guard;
        }
        return EnemyIntent::Attack;
    }

    if (battleType == BattleType::Elite)
    {
        if (roll <= 25)
        {
            return EnemyIntent::Guard;
        }
        return EnemyIntent::Attack;
    }

    if (roll <= 20)
    {
        return EnemyIntent::Guard;
    }

    return EnemyIntent::Attack;
}

std::vector<SkillDefinition> BuildSkillList(JobClass job, int level)
{
    std::vector<SkillDefinition> skills;

    if (job == JobClass::Warrior)
    {
        skills.push_back({"강철 태세", "MP 8 소모. 피해를 주고 이번 턴 방어를 강화한다.", 8, 5, true});
        if (level >= 5)
        {
            skills.push_back({"파쇄 돌격", "MP 14 소모. 더 큰 피해를 주고 자세를 다잡는다.", 14, 12, true});
        }
    }
    else
    {
        skills.push_back({"마력 폭발", "MP 14 소모. 강한 마법 피해를 준다.", 14, 16, false});
        if (level >= 5)
        {
            skills.push_back({"운석 낙하", "MP 20 소모. 압도적인 마법 피해를 준다.", 20, 24, false});
        }
    }

    return skills;
}

std::vector<ItemDefinition> BuildItemList(const Player& player)
{
    return {
        {"회복약", "HP를 35 회복한다.", player.potionCount},
        {"마나약", "MP를 20 회복한다.", player.etherCount}
    };
}

std::string ActionDescription(JobClass job, int level, int actionIndex)
{
    switch (actionIndex)
    {
    case 0:
        return "무기를 휘둘러 적을 공격한다.";
    case 1:
        return (job == JobClass::Warrior)
            ? ((level >= 5) ? "사용 가능한 전투 기술을 선택한다. 강철 태세와 파쇄 돌격을 사용할 수 있다." : "사용 가능한 전투 기술을 선택한다. 현재는 강철 태세를 사용할 수 있다.")
            : ((level >= 5) ? "사용 가능한 마법 기술을 선택한다. 마력 폭발과 운석 낙하를 사용할 수 있다." : "사용 가능한 마법 기술을 선택한다. 현재는 마력 폭발을 사용할 수 있다.");
    case 2:
        return "사용 가능한 소모품 목록을 열어 하나를 선택한다.";
    case 3:
        return "방어 자세를 취해 받는 피해를 줄인다.";
    case 4:
        return "전투에서 이탈할 기회를 노린다.";
    }

    return "행동을 선택한다.";
}

int ComputeDamage(int attack, int defense)
{
    return std::max(1, attack - defense);
}

void PushBattleLog(std::vector<std::string>& logs, const std::string& line)
{
    logs.push_back(line);

    const std::size_t maxLogCount = 5;
    if (logs.size() > maxLogCount)
    {
        logs.erase(logs.begin());
    }
}

std::string ComposeLogText(const std::vector<std::string>& logs)
{
    std::ostringstream stream;
    for (const std::string& line : logs)
    {
        stream << "- " << line << '\n';
    }

    return stream.str();
}

std::string ComposeStatusHeadline(const Player& player)
{
    std::ostringstream body;
    body << "[현재 상태] HP " << player.hp << '/' << player.maxHp;
    body << " | MP " << player.mp << '/' << player.maxMp;
    body << " | 회복약 " << player.potionCount;
    body << " | 마나약 " << player.etherCount << '\n';
    return body.str();
}

std::string ComposePlayerPanel(const Player& player)
{
    std::ostringstream body;
    body << "[플레이어 패널]\n";
    body << player.name << " | 층 " << player.floor << " | Lv " << player.level << '\n';
    body << "HP [" << MakeBar(player.hp, player.maxHp, 20, '#', '.') << "] " << player.hp << '/' << player.maxHp << '\n';
    body << "MP [" << MakeBar(player.mp, player.maxMp, 20, '@', '.') << "] " << player.mp << '/' << player.maxMp << '\n';
    body << "ATK " << player.atk << " | DEF " << player.def << " | GOLD " << player.gold << '\n';
    body << "회복약 " << player.potionCount << " | 마나약 " << player.etherCount << '\n';
    body << "패시브 " << PassiveName(player.job) << " - " << PassiveDescription(player.job) << '\n';
    return body.str();
}

std::string ComposeEnemyPanel(
    const Player& player,
    const Enemy& enemy,
    int enemyHp,
    BattleType battleType,
    EnemyIntent nextIntent)
{
    std::ostringstream body;
    body << "[적 패널]\n";
    body << enemy.name << " | " << BattleTypeName(battleType) << '\n';
    body << "HP [" << MakeBar(enemyHp, enemy.hp, 20, '#', '.') << "] " << enemyHp << '/' << enemy.hp << '\n';
    body << "ATK " << enemy.atk << " | 보상 " << enemy.goldReward << " Gold\n";

    if (HasObservationRelic(player))
    {
        body << "다음 행동: " << EnemyIntentName(nextIntent) << "\n";
        body << EnemyIntentDescription(nextIntent) << '\n';
    }
    else if (battleType == BattleType::Boss)
    {
        body << "심연이 꿈틀거린다. 물러설 곳은 없다.\n";
    }
    else if (battleType == BattleType::Elite)
    {
        body << "보통 적보다 날카로운 기세가 느껴진다.\n";
    }
    else
    {
        body << "숨을 고르고 적의 움직임을 살핀다.\n";
    }

    return body.str();
}

std::string ComposeBattleBody(
    const Player& player,
    const Enemy& enemy,
    int enemyHp,
    BattleType battleType,
    EnemyIntent nextIntent,
    int selected,
    const std::vector<std::string>& battleLogs)
{
    std::ostringstream body;
    body << ComposeStatusHeadline(player);
    body << ComposePlayerPanel(player) << '\n';
    body << "------------------------------------------------------------\n";
    body << ComposeEnemyPanel(player, enemy, enemyHp, battleType, nextIntent) << '\n';
    body << "------------------------------------------------------------\n";
    body << "[현재 행동]\n";
    body << ActionDescription(player.job, player.level, selected) << "\n\n";
    body << "[전투 로그]\n";
    body << ComposeLogText(battleLogs);
    return body.str();
}

std::string ComposeSkillMenuBody(const Player& player, const SkillDefinition& skill)
{
    std::ostringstream body;
    body << ComposeStatusHeadline(player);
    body << "[선택한 스킬]\n";
    body << skill.name << '\n';
    body << skill.description << '\n';
    body << "필요 MP: " << skill.mpCost << "\n\n";
    body << ComposePlayerPanel(player);
    body << "ESC를 누르면 전투 메뉴로 돌아간다.\n";
    return body.str();
}

std::string ComposeItemMenuBody(const Player& player, const ItemDefinition& item)
{
    std::ostringstream body;
    body << ComposeStatusHeadline(player);
    body << "[선택한 아이템]\n";
    body << item.name << " | 보유 수량: " << item.count << '\n';
    body << item.description << "\n\n";
    body << ComposePlayerPanel(player);
    body << "ESC를 누르면 전투 메뉴로 돌아간다.\n";
    return body.str();
}
}

BattleResult BattleScreen::Run(
    Player& player,
    const Enemy& enemy,
    BattleType battleType,
    const ConsoleRenderer& renderer,
    const MenuInput& input) const
{
    const std::vector<std::string> options = {"공격", "스킬", "아이템", "방어", "도주"};
    int selected = 0;
    int enemyHp = enemy.hp;
    std::vector<std::string> battleLogs;
    EnemyIntent pendingEnemyIntent = RollEnemyIntent(enemyHp, enemy.hp, battleType);

    if (battleType == BattleType::Boss)
    {
        PushBattleLog(battleLogs, "10층 최상부에서 심연의 징조가 모습을 드러냈다.");
        PushBattleLog(battleLogs, "물러설 곳은 없다. 이 전투의 승패가 탐험의 끝을 결정한다.");
    }
    else
    {
        PushBattleLog(battleLogs, "전투가 시작되었다.");
    }

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame(
            "전투",
            ComposeBattleBody(player, enemy, enemyHp, battleType, pendingEnemyIntent, selected, battleLogs),
            options,
            selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type == MenuResultType::Cancel)
        {
            if (battleType == BattleType::Boss)
            {
                PushBattleLog(battleLogs, "보스전에서는 도망칠 수 없다.");
                continue;
            }
            return BattleResult::Escape;
        }

        if (action.index == 4)
        {
            if (battleType == BattleType::Boss)
            {
                PushBattleLog(battleLogs, "심연의 징조가 퇴로를 막았다.");
                continue;
            }
            return BattleResult::Escape;
        }

        bool guarded = false;
        bool enemyGuarded = (pendingEnemyIntent == EnemyIntent::Guard);
        int playerDamage = 0;
        bool performedAction = false;

        switch (action.index)
        {
        case 0:
            performedAction = true;
            playerDamage = ComputeDamage(player.atk, enemyGuarded ? (enemy.atk / 2) : (enemy.atk / 3));
            enemyHp = std::max(0, enemyHp - playerDamage);
            PushBattleLog(battleLogs, "공격이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.");
            break;

        case 1:
        {
            const std::vector<SkillDefinition> skills = BuildSkillList(player.job, player.level);
            int skillSelected = 0;

            for (;;)
            {
                std::vector<std::string> skillOptions;
                for (const SkillDefinition& skill : skills)
                {
                    skillOptions.push_back(skill.name);
                }

                renderer.Present(renderer.ComposeMenuFrame(
                    "스킬 선택",
                    ComposeSkillMenuBody(player, skills[skillSelected]),
                    skillOptions,
                    skillSelected));

                const MenuAction skillAction = input.ReadMenuSelection(skillSelected, static_cast<int>(skillOptions.size()));
                if (skillAction.type == MenuResultType::Move)
                {
                    skillSelected = skillAction.index;
                    continue;
                }

                if (skillAction.type == MenuResultType::Cancel)
                {
                    break;
                }

                const SkillDefinition& skill = skills[skillAction.index];
                performedAction = true;

                if (player.mp < skill.mpCost)
                {
                    PushBattleLog(battleLogs, "MP가 부족해 " + skill.name + "을(를) 사용할 수 없다.");
                    break;
                }

                player.mp -= skill.mpCost;
                guarded = skill.grantsGuard;
                playerDamage = ComputeDamage(player.atk + skill.attackBonus, enemyGuarded ? (enemy.atk / 2) : (enemy.atk / 6));
                enemyHp = std::max(0, enemyHp - playerDamage);
                PushBattleLog(battleLogs, skill.name + "이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.");
                if (skill.grantsGuard)
                {
                    PushBattleLog(battleLogs, "스킬의 여파로 자세를 가다듬고 방어를 강화했다.");
                }
                break;
            }

            if (!performedAction)
            {
                continue;
            }
            break;
        }

        case 2:
        {
            const std::vector<ItemDefinition> items = BuildItemList(player);
            int itemSelected = 0;

            for (;;)
            {
                std::vector<std::string> itemOptions;
                for (const ItemDefinition& item : items)
                {
                    itemOptions.push_back(item.name);
                }

                renderer.Present(renderer.ComposeMenuFrame(
                    "아이템 선택",
                    ComposeItemMenuBody(player, items[itemSelected]),
                    itemOptions,
                    itemSelected));

                const MenuAction itemAction = input.ReadMenuSelection(itemSelected, static_cast<int>(itemOptions.size()));
                if (itemAction.type == MenuResultType::Move)
                {
                    itemSelected = itemAction.index;
                    continue;
                }

                if (itemAction.type == MenuResultType::Cancel)
                {
                    break;
                }

                performedAction = true;
                if (itemAction.index == 0)
                {
                    if (player.potionCount <= 0)
                    {
                        PushBattleLog(battleLogs, "회복약이 부족하다.");
                        break;
                    }

                    if (player.hp >= player.maxHp)
                    {
                        PushBattleLog(battleLogs, "HP가 가득 차 있어 회복약을 사용할 수 없다.");
                        break;
                    }

                    player.hp = std::min(player.maxHp, player.hp + 35);
                    --player.potionCount;
                    PushBattleLog(battleLogs, "회복약을 사용해 HP를 회복했다.");
                    break;
                }

                if (player.etherCount <= 0)
                {
                    PushBattleLog(battleLogs, "마나약이 부족하다.");
                    break;
                }

                if (player.mp >= player.maxMp)
                {
                    PushBattleLog(battleLogs, "MP가 가득 차 있어 마나약을 사용할 수 없다.");
                    break;
                }

                player.mp = std::min(player.maxMp, player.mp + 20);
                --player.etherCount;
                PushBattleLog(battleLogs, "마나약을 사용해 MP를 회복했다.");
                break;
            }

            if (!performedAction)
            {
                continue;
            }
            break;
        }

        case 3:
            performedAction = true;
            guarded = true;
            PushBattleLog(battleLogs, "방어 자세를 취했다.");
            break;

        default:
            break;
        }

        if (enemyGuarded && enemyHp > 0)
        {
            PushBattleLog(battleLogs, enemy.name + "이(가) 충격을 줄일 자세를 갖추고 있었다.");
        }

        if (enemyHp <= 0)
        {
            PushBattleLog(battleLogs, enemy.name + "을(를) 쓰러뜨렸다.");
            renderer.Present(renderer.ComposeMenuFrame(
                "전투 승리",
                ComposeBattleBody(player, enemy, enemyHp, battleType, pendingEnemyIntent, selected, battleLogs),
                options,
                selected));
            return BattleResult::Victory;
        }

        if (pendingEnemyIntent == EnemyIntent::Recover)
        {
            const int recoverAmount = (battleType == BattleType::Boss) ? 18 : 12;
            const int previousHp = enemyHp;
            enemyHp = std::min(enemy.hp, enemyHp + recoverAmount);
            PushBattleLog(battleLogs, enemy.name + "이(가) 몸을 추슬러 HP를 " + std::to_string(enemyHp - previousHp) + " 회복했다.");
        }
        else if (pendingEnemyIntent == EnemyIntent::Guard)
        {
            PushBattleLog(battleLogs, enemy.name + "이(가) 방어 자세를 취했다. 다음 턴에는 피해가 줄어들 수 있다.");
        }
        else
        {
            const int defenseValue = guarded ? player.def + 6 : player.def;
            int enemyDamage = ComputeDamage(enemy.atk, defenseValue);

            if (player.job == JobClass::Warrior)
            {
                enemyDamage = std::max(1, enemyDamage - 2);
            }

            player.hp = std::max(0, player.hp - enemyDamage);

            if (guarded)
            {
                PushBattleLog(battleLogs, "적의 반격을 받아 " + std::to_string(enemyDamage) + "의 피해를 입었지만 방어로 충격을 줄였다.");
            }
            else
            {
                PushBattleLog(battleLogs, "적의 반격으로 " + std::to_string(enemyDamage) + "의 피해를 입었다.");
            }
        }

        if (player.job == JobClass::Mage && performedAction)
        {
            const int recoveredMp = std::min(3, player.maxMp - player.mp);
            if (recoveredMp > 0)
            {
                player.mp += recoveredMp;
                PushBattleLog(battleLogs, "마력 순환으로 MP를 " + std::to_string(recoveredMp) + " 회복했다.");
            }
        }

        if (player.hp <= 0)
        {
            PushBattleLog(battleLogs, "플레이어가 쓰러졌다.");
            return BattleResult::Defeat;
        }

        pendingEnemyIntent = RollEnemyIntent(enemyHp, enemy.hp, battleType);
    }
}