#include "game/screens/JobSelectScreen.h"

#include <vector>

// 직업 선택 화면을 표시하고 선택된 직업을 반환한다.
std::optional<JobClass> JobSelectScreen::Run(const ConsoleRenderer& renderer, const MenuInput& input) const
{
    const std::vector<std::string> options = {"전사", "마법사"};
    int selected = 0;

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame(
            "직업 선택",
            "탑에 도전할 직업을 고른다.",
            options,
            selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Cancel)
        {
            return std::nullopt;
        }

        if (action.type == MenuResultType::Confirm)
        {
            return (action.index == 0) ? JobClass::Warrior : JobClass::Mage;
        }

        selected = action.index;
    }
}
