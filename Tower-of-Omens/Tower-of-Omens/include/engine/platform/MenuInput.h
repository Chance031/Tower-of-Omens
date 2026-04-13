#pragma once

// 메뉴 입력 결과의 종류를 나타낸다.
enum class MenuResultType
{
    Confirm,
    Cancel,
    Move
};

// 메뉴 입력 해석 결과를 나타낸다.
struct MenuAction
{
    MenuResultType type = MenuResultType::Cancel;
    int index = 0;
};

// 메뉴 이동에 필요한 키 입력 해석만 담당한다.
class MenuInput
{
public:
    // 현재 선택 위치를 기준으로 다음 입력 결과를 해석한다.
    MenuAction ReadMenuSelection(int currentSelected, int optionCount) const;

    // 아무 키나 입력될 때까지 대기한다.
    void WaitForAnyKey() const;
};
