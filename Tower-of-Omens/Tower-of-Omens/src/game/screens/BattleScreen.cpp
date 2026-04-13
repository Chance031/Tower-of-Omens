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
        return "아직 사용할 아이템이 없다.";
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
    std::string battleLog = "전투가 시작되었다.";

    for (;;)
    {
        std::ostringstream body;
        body << "전투 종류: " << BattleTypeName(battleType) << '\n';
        body << "----------------------------------------\n";
        body << "[플레이어]\n";
        body << player.name << " | 현재 층 " << player.floor << '\n';
        body << "HP " << player.hp << '/' << player.maxHp;
        body << " | MP " << player.mp << '/' << player.maxMp << '\n';
        body << "ATK " << player.atk << " | DEF " << player.def << " | GOLD " << player.gold << "\n\n";
        body << "[적]\n";
        body << enemy.name << " | HP " << enemyHp << '/' << enemy.hp << " | ATK " << enemy.atk;
        body << " | 보상 " << enemy.goldReward << " Gold\n\n";
        body << "[상태]\n";
        body << ActionDescription(selected) << "\n";
        body << battleLog << "\n";

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
            battleLog = "공격이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.";
            break;

        case 1:
            if (player.mp < 10)
            {
                battleLog = "MP가 부족해 스킬을 사용할 수 없다.";
                continue;
            }
            player.mp -= 10;
            playerDamage = ComputeDamage(player.atk + 8, enemy.atk / 4);
            enemyHp = std::max(0, enemyHp - playerDamage);
            battleLog = "스킬이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.";
            break;

        case 2:
            battleLog = "아직 사용할 아이템이 없다.";
            continue;

        case 3:
            guarded = true;
            battleLog = "방어 자세를 취했다.";
            break;

        default:
            break;
        }

        if (enemyHp <= 0)
        {
            battleLog += " 적을 쓰러뜨렸다.";
            renderer.Present(renderer.ComposeMenuFrame("전투", body.str(), options, selected));
            return BattleResult::Victory;
        }

        const int defenseValue = guarded ? player.def + 6 : player.def;
        const int enemyDamage = ComputeDamage(enemy.atk, defenseValue);
        player.hp = std::max(0, player.hp - enemyDamage);

        if (guarded)
        {
            battleLog += " 적의 반격을 받아 " + std::to_string(enemyDamage) + "의 피해를 입었지만 방어로 충격을 줄였다.";
        }
        else
        {
            battleLog += " 적의 반격으로 " + std::to_string(enemyDamage) + "의 피해를 입었다.";
        }

        if (player.hp <= 0)
        {
            return BattleResult::Defeat;
        }
    }
}
