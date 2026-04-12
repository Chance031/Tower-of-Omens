#include "game/screens/TitleScreen.h"

#include <vector>

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

        const int result = input.ReadMenuSelection(static_cast<int>(options.size()));
        if (result > 0)
        {
            return result == 1;
        }

        if (result == 0)
        {
            return false;
        }

        selected = -result - 1;
    }
}
