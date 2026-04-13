#include "game/screens/BattleScreen.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace
{
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

// 현재 선택한 행동의 설명 문구를 만든다.
std::string ActionDescription(int actionIndex)
{
    switch (actionIndex)
    {
    case 0:
        return "무기를 휘둘러 적을 공격한다.";
    case 1:
        return "MP 10을 소모해 강한 일격을 가한다.";
    case 2:
        return "회복약이나 마나약을 사용한다.";
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
        body << "회복약 " << player.potionCount << " | 마나약 " << player.etherCount << "\n\n";
        body << "[적]\n";
        body << enemy.name << " | HP " << enemyHp << '/' << enemy.hp << " | ATK " << enemy.atk;
        body << " | 보상 " << enemy.goldReward << " Gold\n\n";
        body << "[행동 설명]\n";
        body << ActionDescription(selected) << "\n\n";
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

        switch (action.index)
        {
        case 0:
            playerDamage = ComputeDamage(player.atk, enemy.atk / 3);
            enemyHp = std::max(0, enemyHp - playerDamage);
            PushBattleLog(battleLogs, "공격이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.");
            break;

        case 1:
            if (player.mp < 10)
            {
                PushBattleLog(battleLogs, "MP가 부족해 스킬을 사용할 수 없다.");
                continue;
            }
            player.mp -= 10;
            playerDamage = ComputeDamage(player.atk + 8, enemy.atk / 4);
            enemyHp = std::max(0, enemyHp - playerDamage);
            PushBattleLog(battleLogs, "스킬이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.");
            break;

        case 2:
            if (player.hp < player.maxHp && player.potionCount > 0)
            {
                player.hp = std::min(player.maxHp, player.hp + 35);
                --player.potionCount;
                PushBattleLog(battleLogs, "회복약을 사용해 HP를 회복했다.");
                continue;
            }

            if (player.mp < player.maxMp && player.etherCount > 0)
            {
                player.mp = std::min(player.maxMp, player.mp + 20);
                --player.etherCount;
                PushBattleLog(battleLogs, "마나약을 사용해 MP를 회복했다.");
                continue;
            }

            PushBattleLog(battleLogs, "사용할 수 있는 소모품이 없다.");
            continue;

        case 3:
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
            clearBody << "회복약 " << player.potionCount << " | 마나약 " << player.etherCount << "\n\n";
            clearBody << "[적]\n";
            clearBody << enemy.name << " | HP 0/" << enemy.hp << " | ATK " << enemy.atk;
            clearBody << " | 보상 " << enemy.goldReward << " Gold\n\n";
            clearBody << "[행동 설명]\n";
            clearBody << ActionDescription(selected) << "\n\n";
            clearBody << "[전투 로그]\n";
            clearBody << ComposeLogText(battleLogs);
            renderer.Present(renderer.ComposeMenuFrame("전투", clearBody.str(), options, selected));
            return BattleResult::Victory;
        }

        const int defenseValue = guarded ? player.def + 6 : player.def;
        const int enemyDamage = ComputeDamage(enemy.atk, defenseValue);
        player.hp = std::max(0, player.hp - enemyDamage);

        if (guarded)
        {
            PushBattleLog(battleLogs, "적의 반격을 받아 " + std::to_string(enemyDamage) + "의 피해를 입었지만 방어로 충격을 줄였다.");
        }
        else
        {
            PushBattleLog(battleLogs, "적의 반격으로 " + std::to_string(enemyDamage) + "의 피해를 입었다.");
        }

        if (player.hp <= 0)
        {
            PushBattleLog(battleLogs, "플레이어가 쓰러졌다.");
            return BattleResult::Defeat;
        }
    }
}
