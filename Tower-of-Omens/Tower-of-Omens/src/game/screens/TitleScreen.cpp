#include "game/screens/TitleScreen.h"

#include <vector>

// 타이틀 화면을 표시하고 시작 여부를 반환한다.
bool TitleScreen::Run(const ConsoleRenderer& renderer, const MenuInput& input) const
{
    const std::vector<std::string> options = {"New Game", "Exit"};
    int selected = 0;

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame(
            "Title",
            "====================================\n        Tower of Omens\n====================================",
            options,
            selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Cancel)
        {
            return false;
        }

        if (action.type == MenuResultType::Confirm)
        {
            return action.index == 0;
        }

        selected = action.index;
    }
}
