#include "game/screens/MessageScreen.h"

#include <sstream>

void MessageScreen::Show(
    const ConsoleRenderer& renderer,
    const MenuInput& input,
    const std::string& title,
    const std::string& body) const
{
    std::ostringstream frame;
    frame << '[' << title << "]\n";
    frame << body << "\n\n";
    frame << "Press any key to continue.\n";

    renderer.Present(frame.str());
    input.WaitForAnyKey();
}
