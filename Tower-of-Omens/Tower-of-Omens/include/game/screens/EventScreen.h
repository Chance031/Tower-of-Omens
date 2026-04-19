#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <string>
#include <vector>

class MessageScreen;

// 이벤트 하나의 선택지 정보를 담는다.
struct EventChoice
{
    std::string label;      // 선택지 표시 텍스트
    std::string effect;     // 적용할 효과 문자열 (파이프로 구분)
    std::string resultText; // 선택 후 표시할 결과 메시지
};

// CSV에서 불러온 이벤트 하나의 전체 정의를 담는다.
struct EventDefinition
{
    int id = 0;               // 고유 식별자
    std::string name;         // 이벤트 제목
    std::string flavorText;   // 상황 설명 텍스트
    int weight = 100;         // 등장 가중치 (높을수록 자주 등장)
    int minFloor = 1;         // 등장 최소 층
    int maxFloor = 999;       // 등장 최대 층
    std::vector<EventChoice> choices; // 선택지 목록
};

// CSV로 정의된 이벤트를 불러와 현재 층에 맞는 이벤트를 처리한다.
class EventScreen
{
public:
    GameState Run(
        Player& player,
        MessageScreen& messageScreen,
        const ConsoleRenderer& renderer,
        const MenuInput& input) const;
};