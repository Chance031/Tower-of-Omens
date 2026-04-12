#include "game/screens/MessageScreen.h"

#include <sstream>

// 단순 메시지 화면을 표시하고 다음 입력을 기다린다.
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
