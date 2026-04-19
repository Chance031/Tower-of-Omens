#pragma once

// 메뉴 입력 결과의 유형을 나타낸다.
enum class MenuResultType
{
    Confirm,
    Cancel,
    Move
};

// 메뉴 입력 해석 결과를 담는 구조체다.
struct MenuAction
{
    MenuResultType type = MenuResultType::Cancel;
    int index = 0;
};

// 메뉴 이동에 필요한 키 입력을 해석한다.
class MenuInput
{
public:
    // 현재 선택 위치와 항목 수를 받아 키 입력을 해석하고 결과를 반환한다.
    MenuAction ReadMenuSelection(int currentSelected, int optionCount) const;

    // 아무 키가 입력될 때까지 대기한다.
    void WaitForAnyKey() const;
};
