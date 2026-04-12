#include "engine/platform/MenuInput.h"

#include <conio.h>

int MenuInput::ReadMenuSelection(int optionCount) const
{
    int selected = 0;

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

void MenuInput::WaitForAnyKey() const
{
    _getch();
}
