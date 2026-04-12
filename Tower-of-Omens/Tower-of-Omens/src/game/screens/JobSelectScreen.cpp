#include "game/screens/JobSelectScreen.h"

#include <vector>

std::optional<JobClass> JobSelectScreen::Run(const ConsoleRenderer& renderer, const MenuInput& input) const
{
    const std::vector<std::string> options = {"Warrior", "Mage"};
    int selected = 0;

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame(
            "Choose Job",
            "Choose a job for the first run.",
            options,
            selected));

        const int result = input.ReadMenuSelection(static_cast<int>(options.size()));
        if (result == 0)
        {
            return std::nullopt;
        }

        if (result > 0)
        {
            return (result == 1) ? JobClass::Warrior : JobClass::Mage;
        }

        selected = -result - 1;
    }
}
