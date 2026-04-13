#include "game/screens/BattleScreen.h"

#include <sstream>
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
    }

    return "전투";
}

// 선택한 행동의 설명 문구를 만든다.
std::string ActionDescription(int actionIndex)
{
    switch (actionIndex)
    {
    case 0:
        return "기본 공격으로 적을 시험한다.";
    case 1:
        return "스킬 슬롯을 둘러보지만 아직 비어 있다.";
    case 2:
        return "아이템 가방을 열어보지만 아직 준비된 소모품이 없다.";
    case 3:
        return "숨을 고르고 다음 턴을 준비한다.";
    case 4:
        return "전투에서 이탈할 기회를 노린다.";
    }

    return "행동을 선택한다.";
}
}

// 전투 화면을 표시하고 현재 전투의 결과를 돌려준다.
BattleResult BattleScreen::Run(
    const Player& player,
    const Enemy& enemy,
    BattleType battleType,
    const ConsoleRenderer& renderer,
    const MenuInput& input) const
{
    const std::vector<std::string> options = {"공격", "스킬", "아이템", "방어", "도주"};
    int selected = 0;

    for (;;)
    {
        std::ostringstream body;
        body << "전투 종류: " << BattleTypeName(battleType) << '\n';
        body << "----------------------------------------\n";
        body << "[플레이어]\n";
        body << player.name << " | HP " << player.hp << '/' << player.maxHp;
        body << " | MP " << player.mp << '/' << player.maxMp << '\n';
        body << "ATK " << player.atk << " | DEF " << player.def << " | GOLD " << player.gold << "\n\n";
        body << "[적]\n";
        body << enemy.name << " | HP " << enemy.hp << " | ATK " << enemy.atk;
        body << " | 보상 " << enemy.goldReward << " Gold\n\n";
        body << "[상태]\n";
        body << ActionDescription(selected) << "\n";
        body << "길 선택에 따라 적의 강함과 보상이 달라진다.\n";

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

        if (action.index == 0)
        {
            return BattleResult::Victory;
        }

        if (action.index == 3)
        {
            return BattleResult::Victory;
        }

        return BattleResult::Victory;
    }
}
