#pragma once

// 메뉴 이동에 필요한 키 입력 해석만 담당한다.
class MenuInput
{
public:
    // 현재 선택 위치를 기준으로 다음 입력 결과를 해석한다.
    int ReadMenuSelection(int currentSelected, int optionCount) const;

    // 아무 키나 입력될 때까지 대기한다.
    void WaitForAnyKey() const;
};
