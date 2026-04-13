#include "game/screens/EventScreen.h"

#include "game/screens/MessageScreen.h"

#include <algorithm>
#include <sstream>
#include <string>

GameState EventScreen::RunObservationEvent(
    Player& player,
    MessageScreen& messageScreen,
    const ConsoleRenderer& renderer,
    const MenuInput& input) const
{
    const std::string observationRelicName = "관찰 유물";
    const bool alreadyOwned =
        std::find(player.relicNames.begin(), player.relicNames.end(), observationRelicName) != player.relicNames.end();

    std::ostringstream body;
    body << "희미한 수정 구슬이 놓인 방을 발견했다.\n";
    body << "구슬 표면에는 적의 다음 움직임을 비추는 문양이 떠오른다.\n\n";

    if (!alreadyOwned)
    {
        player.relicNames.push_back(observationRelicName);
        body << "[획득 유물]\n";
        body << observationRelicName << '\n';
        body << "적이 다음 턴에 공격, 방어, 회복 중 무엇을 할지 미리 볼 수 있다.\n";
    }
    else
    {
        player.gold += 25;
        player.hp = std::min(player.maxHp, player.hp + 10);
        body << "이미 관찰 유물을 보유하고 있다.\n";
        body << "대신 Gold 25와 HP 10을 회복했다.\n";
    }

    messageScreen.Show(renderer, input, "이벤트", body.str());
    ++player.floor;
    return GameState::Prep;
}