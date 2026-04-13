#include "game/screens/MessageScreen.h"

#include <string>

// 단순 메시지 화면을 표시하고 다음 입력을 기다린다.
void MessageScreen::Show(
    const ConsoleRenderer& renderer,
    const MenuInput& input,
    const std::string& title,
    const std::string& body) const
{
    renderer.Present(renderer.ComposeMenuFrame(title, body + "\n\n아무 키나 누르면 계속한다.", {}, 0));
    input.WaitForAnyKey();
}
