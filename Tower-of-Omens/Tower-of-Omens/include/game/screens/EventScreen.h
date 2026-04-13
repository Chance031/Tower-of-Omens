#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <string>
#include <vector>

class MessageScreen;

struct EventChoice
{
    std::string label;
    std::string effect;
    std::string resultText;
};

struct EventDefinition
{
    int id = 0;
    std::string name;
    std::string flavorText;
    int weight = 100;
    int minFloor = 1;
    int maxFloor = 999;
    std::vector<EventChoice> choices;
};

// CSV에 정의된 이벤트를 불러와 현재 상황에 맞는 이벤트를 실행한다.
class EventScreen
{
public:
    GameState Run(
        Player& player,
        MessageScreen& messageScreen,
        const ConsoleRenderer& renderer,
        const MenuInput& input) const;
};