#include "game/screens/FloorLoopScreen.h"

#include <sstream>
#include <vector>

bool FloorLoopScreen::Run(const Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const
{
    const std::vector<std::string> options = {"Back to Title", "Exit"};
    int selected = 0;

    std::ostringstream body;
    body << "Player: " << player.name << '\n';
    body << "Floor: " << player.floor << '\n';
    body << "HP: " << player.hp << " | MP: " << player.mp << " | Gold: " << player.gold << "\n\n";
    body << "Next step: battle / event / reward / preparation will be added next.\n";

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame("Floor Loop", body.str(), options, selected));

        const int result = input.ReadMenuSelection(static_cast<int>(options.size()));
        if (result == 0)
        {
            return true;
        }

        if (result > 0)
        {
            return result == 1;
        }

        selected = -result - 1;
    }
}
