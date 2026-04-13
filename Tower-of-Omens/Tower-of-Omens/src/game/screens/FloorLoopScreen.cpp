#include "game/screens/FloorLoopScreen.h"

#include "engine/platform/MenuInput.h"

#include <sstream>
#include <vector>

// 층 진행 화면을 표시하고 다음 이동 방향을 결정한다.
FloorLoopResult FloorLoopScreen::Run(const Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const
{
    if (player.floor == 1)
    {
        return {GameState::FloorLoop, PathChoice::Normal};
    }

    const std::vector<std::string> options =
    {
        "안정적인 길",
        "강한 기척",
        "미지의 길"
    };
    int selected = 0;

    std::ostringstream body;
    body << "현재 층: " << player.floor << '\n';
    body << "플레이어: " << player.name << '\n';
    body << "HP: " << player.hp << " | MP: " << player.mp << " | Gold: " << player.gold << "\n\n";
    body << "이번 층에서 이동할 길을 선택한다.\n";

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame("층 진행", body.str(), options, selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Confirm)
        {
            switch (action.index)
            {
            case 0:
                return {GameState::FloorLoop, PathChoice::Safe};
            case 1:
                return {GameState::FloorLoop, PathChoice::Dangerous};
            default:
                return {GameState::FloorLoop, PathChoice::Unknown};
            }
        }

        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
        }
    }
}
