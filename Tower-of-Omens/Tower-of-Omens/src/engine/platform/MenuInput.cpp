#include "engine/platform/MenuInput.h"

#include <conio.h>

// 메뉴 선택에 필요한 키 입력을 읽어 결과로 변환한다.
int MenuInput::ReadMenuSelection(int currentSelected, int optionCount) const
{
    int selected = currentSelected;

    for (;;)
    {
        const int key = _getch();
        if (key == 13)
        {
            return selected + 1;
        }

        if (key == 27)
        {
            return 0;
        }

        if (key == 0 || key == 224)
        {
            const int extended = _getch();
            if (extended == 72)
            {
                selected = (selected - 1 + optionCount) % optionCount;
                return -(selected + 1);
            }

            if (extended == 80)
            {
                selected = (selected + 1) % optionCount;
                return -(selected + 1);
            }
        }
    }
}

// 아무 키나 입력될 때까지 대기한다.
void MenuInput::WaitForAnyKey() const
{
    _getch();
}
