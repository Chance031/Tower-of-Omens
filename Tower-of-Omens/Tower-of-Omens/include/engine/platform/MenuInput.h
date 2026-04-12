#pragma once

// 메뉴 이동에 필요한 키 입력 해석만 담당한다.
class MenuInput
{
public:
    int ReadMenuSelection(int optionCount) const;
    void WaitForAnyKey() const;
};
