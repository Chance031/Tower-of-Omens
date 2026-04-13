#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"
#include "game/Player.h"

class MessageScreen;

// 이벤트 연출과 보상 적용을 담당한다.
class EventScreen
{
public:
    GameState RunObservationEvent(
        Player& player,
        MessageScreen& messageScreen,
        const ConsoleRenderer& renderer,
        const MenuInput& input) const;
};