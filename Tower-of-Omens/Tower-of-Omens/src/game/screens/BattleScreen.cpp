#include "game/screens/BattleScreen.h"

#include <algorithm>
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

// 전투 종류를 화면에 표시할 이름으로 변환한다.
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

// 직업별 패시브 이름을 반환한다.
std::string PassiveName(JobClass job)
{
    return (job == JobClass::Warrior) ? "불굴" : "마력 순환";
}

// 직업별 패시브 설명을 반환한다.
std::string PassiveDescription(JobClass job)
{
    return (job == JobClass::Warrior)
        ? "받는 피해가 항상 2 감소한다."
        : "행동 후 MP를 3 회복한다.";
}

// 현재 층과 직업에 맞는 사용 가능한 스킬 목록을 만든다.
std::vector<SkillDefinition> BuildSkillList(JobClass job, int floor)
{
    std::vector<SkillDefinition> skills;

    if (job == JobClass::Warrior)
    {
        skills.push_back({"강철 태세", "MP 8 소모. 피해를 주고 이번 턴 방어를 강화한다.", 8, 5, true});
        if (floor >= 5)
        {
            skills.push_back({"파쇄 돌격", "MP 14 소모. 더 큰 피해를 주고 자세를 다잡는다.", 14, 12, true});
        }
    }
    else
    {
        skills.push_back({"마력 폭발", "MP 14 소모. 강한 마법 피해를 준다.", 14, 16, false});
        if (floor >= 5)
        {
            skills.push_back({"운석 낙하", "MP 20 소모. 압도적인 마법 피해를 준다.", 20, 24, false});
        }
    }

    return skills;
}

// 현재 플레이어의 아이템 목록을 만든다.
std::vector<ItemDefinition> BuildItemList(const Player& player)
{
    return {
        {"회복약", "HP를 35 회복한다.", player.potionCount},
        {"마나약", "MP를 20 회복한다.", player.etherCount}
    };
}

// 직업과 층수, 선택에 따라 행동 설명 문구를 만든다.
std::string ActionDescription(JobClass job, int floor, int actionIndex)
{
    switch (actionIndex)
    {
    case 0:
        return "무기를 휘둘러 적을 공격한다.";
    case 1:
        return (job == JobClass::Warrior)
            ? ((floor >= 5) ? "사용 가능한 전투 기술을 선택한다. 강철 태세와 파쇄 돌격을 사용할 수 있다." : "사용 가능한 전투 기술을 선택한다. 현재는 강철 태세를 사용할 수 있다.")
            : ((floor >= 5) ? "사용 가능한 마법 기술을 선택한다. 마력 폭발과 운석 낙하를 사용할 수 있다." : "사용 가능한 마법 기술을 선택한다. 현재는 마력 폭발을 사용할 수 있다.");
    case 2:
        return "사용 가능한 소모품 목록을 열어 하나를 선택한다.";
    case 3:
        return "방어 자세를 취해 받는 피해를 줄인다.";
    case 4:
        return "전투에서 이탈할 기회를 노린다.";
    }

    return "행동을 선택한다.";
}

// 공격력과 방어력을 바탕으로 최종 피해량을 계산한다.
int ComputeDamage(int attack, int defense)
{
    return std::max(1, attack - defense);
}

// 최근 로그만 남기도록 새 항목을 추가한다.
void PushBattleLog(std::vector<std::string>& logs, const std::string& line)
{
    logs.push_back(line);

    const std::size_t maxLogCount = 5;
    if (logs.size() > maxLogCount)
    {
        logs.erase(logs.begin());
    }
}

// 로그 목록을 화면 출력용 문자열로 합친다.
std::string ComposeLogText(const std::vector<std::string>& logs)
{
    std::ostringstream stream;
    for (const std::string& line : logs)
    {
        stream << "- " << line << '\n';
    }

    return stream.str();
}

// 스킬 목록 화면의 본문 문자열을 만든다.
std::string ComposeSkillMenuBody(const Player& player, const SkillDefinition& skill)
{
    std::ostringstream body;
    body << "[플레이어]\n";
    body << player.name << " | 현재 층 " << player.floor << '\n';
    body << "HP " << player.hp << '/' << player.maxHp;
    body << " | MP " << player.mp << '/' << player.maxMp << "\n\n";
    body << "[선택한 스킬]\n";
    body << skill.name << '\n';
    body << skill.description << '\n';
    body << "필요 MP: " << skill.mpCost << '\n';
    body << "ESC를 누르면 전투 메뉴로 돌아간다.\n";
    return body.str();
}

// 아이템 목록 화면의 본문 문자열을 만든다.
std::string ComposeItemMenuBody(const Player& player, const ItemDefinition& item)
{
    std::ostringstream body;
    body << "[플레이어]\n";
    body << player.name << " | 현재 층 " << player.floor << '\n';
    body << "HP " << player.hp << '/' << player.maxHp;
    body << " | MP " << player.mp << '/' << player.maxMp << "\n\n";
    body << "[선택한 아이템]\n";
    body << item.name << " | 보유 수량: " << item.count << '\n';
    body << item.description << '\n';
    body << "ESC를 누르면 전투 메뉴로 돌아간다.\n";
    return body.str();
}
}

// 전투 화면을 표시하고 현재 전투의 결과를 돌려준다.
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
    int skillSelected = 0;
    int itemSelected = 0;
    std::vector<std::string> battleLogs;

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
        std::ostringstream body;
        body << "전투 종류: " << BattleTypeName(battleType) << '\n';

        if (battleType == BattleType::Boss)
        {
            body << "========================================\n";
            body << "        B O S S   E N C O U N T E R     \n";
            body << "========================================\n";
        }
        else
        {
            body << "----------------------------------------\n";
        }

        body << "[플레이어]\n";
        body << player.name << " | 현재 층 " << player.floor << '\n';
        body << "HP " << player.hp << '/' << player.maxHp;
        body << " | MP " << player.mp << '/' << player.maxMp << '\n';
        body << "ATK " << player.atk << " | DEF " << player.def << " | GOLD " << player.gold << '\n';
        body << "회복약 " << player.potionCount << " | 마나약 " << player.etherCount << '\n';
        body << "패시브 " << PassiveName(player.job) << ": " << PassiveDescription(player.job) << "\n\n";
        body << "[적]\n";
        body << enemy.name << " | HP " << enemyHp << '/' << enemy.hp << " | ATK " << enemy.atk;
        body << " | 보상 " << enemy.goldReward << " Gold\n\n";
        body << "[행동 설명]\n";
        body << ActionDescription(player.job, player.floor, selected) << "\n\n";
        body << "[전투 로그]\n";
        body << ComposeLogText(battleLogs);

        renderer.Present(renderer.ComposeMenuFrame("전투", body.str(), options, selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type == MenuResultType::Cancel)
        {
            return BattleResult::Escape;
        }

        if (action.index == 4)
        {
            return BattleResult::Escape;
        }

        bool guarded = false;
        int playerDamage = 0;
        bool performedAction = false;

        switch (action.index)
        {
        case 0:
            performedAction = true;
            playerDamage = ComputeDamage(player.atk, enemy.atk / 3);
            enemyHp = std::max(0, enemyHp - playerDamage);
            PushBattleLog(battleLogs, "공격이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.");
            break;

        case 1:
        {
            const std::vector<SkillDefinition> skills = BuildSkillList(player.job, player.floor);
            if (skillSelected >= static_cast<int>(skills.size()))
            {
                skillSelected = 0;
            }

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
                playerDamage = ComputeDamage(player.atk + skill.attackBonus, enemy.atk / (skill.grantsGuard ? 4 : 6));
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
            if (itemSelected >= static_cast<int>(items.size()))
            {
                itemSelected = 0;
            }

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
            continue;
        }

        case 3:
            performedAction = true;
            guarded = true;
            PushBattleLog(battleLogs, "방어 자세를 취했다.");
            break;

        default:
            break;
        }

        if (enemyHp <= 0)
        {
            PushBattleLog(battleLogs, enemy.name + "을(를) 쓰러뜨렸다.");
            std::ostringstream clearBody;
            clearBody << "전투 종류: " << BattleTypeName(battleType) << '\n';
            if (battleType == BattleType::Boss)
            {
                clearBody << "========================================\n";
                clearBody << "           B O S S   F A L L E N        \n";
                clearBody << "========================================\n";
            }
            else
            {
                clearBody << "----------------------------------------\n";
            }
            clearBody << "[플레이어]\n";
            clearBody << player.name << " | 현재 층 " << player.floor << '\n';
            clearBody << "HP " << player.hp << '/' << player.maxHp;
            clearBody << " | MP " << player.mp << '/' << player.maxMp << '\n';
            clearBody << "ATK " << player.atk << " | DEF " << player.def << " | GOLD " << player.gold << '\n';
            clearBody << "회복약 " << player.potionCount << " | 마나약 " << player.etherCount << '\n';
            clearBody << "패시브 " << PassiveName(player.job) << ": " << PassiveDescription(player.job) << "\n\n";
            clearBody << "[적]\n";
            clearBody << enemy.name << " | HP 0/" << enemy.hp << " | ATK " << enemy.atk;
            clearBody << " | 보상 " << enemy.goldReward << " Gold\n\n";
            clearBody << "[행동 설명]\n";
            clearBody << ActionDescription(player.job, player.floor, selected) << "\n\n";
            clearBody << "[전투 로그]\n";
            clearBody << ComposeLogText(battleLogs);
            renderer.Present(renderer.ComposeMenuFrame("전투", clearBody.str(), options, selected));
            return BattleResult::Victory;
        }

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
    }
}
