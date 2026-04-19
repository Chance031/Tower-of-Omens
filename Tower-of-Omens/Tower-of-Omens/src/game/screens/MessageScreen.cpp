#include "game/screens/MessageScreen.h"

#include <string>

// 메시지 화면을 표시하고 키 입력을 기다린다.
void MessageScreen::Show(
    const ConsoleRenderer& renderer,
    const MenuInput& input,
    const std::string& title,
    const std::string& body) const
{
    renderer.Present(renderer.ComposeMenuFrame(title, body + "\n\n기타 키를 누르면 진행한다.", {}, 0));
    input.WaitForAnyKey();
}
