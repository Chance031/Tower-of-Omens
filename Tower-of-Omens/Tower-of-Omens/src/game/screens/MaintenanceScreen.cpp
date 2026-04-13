#include "game/screens/MaintenanceScreen.h"

#include <sstream>
#include <vector>

namespace
{
// 정비 비용을 고정값으로 반환한다.
int MaintenanceCost()
{
    return 10;
}

// 소모품 구매 비용을 반환한다.
int SupplyCost()
{
    return 15;
}
}

// 정비 화면을 표시하고 선택 결과를 플레이어 상태에 반영한다.
MaintenanceResult MaintenanceScreen::Run(Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const
{
    const std::vector<std::string> options = {"휴식", "회복약 구매", "마나약 구매", "무기 정비", "방어구 정비", "바로 출발"};
    int selected = 0;

    for (;;)
    {
        std::ostringstream body;
        body << "현재 층: " << player.floor << '\n';
        body << "플레이어: " << player.name << '\n';
        body << "HP: " << player.hp << '/' << player.maxHp;
        body << " | MP: " << player.mp << '/' << player.maxMp << '\n';
        body << "ATK: " << player.atk << " | DEF: " << player.def << " | Gold: " << player.gold << '\n';
        body << "회복약: " << player.potionCount << " | 마나약: " << player.etherCount << "\n\n";
        body << "전투를 마친 뒤 다음 탐색을 준비한다.\n";
        body << "휴식: HP / MP를 전부 회복한다.\n";
        body << "회복약 구매: Gold 15를 소모해 회복약 1개를 얻는다.\n";
        body << "마나약 구매: Gold 15를 소모해 마나약 1개를 얻는다.\n";
        body << "무기 정비: Gold 10을 소모해 ATK +2를 얻는다.\n";
        body << "방어구 정비: Gold 10을 소모해 DEF +2를 얻는다.\n";

        renderer.Present(renderer.ComposeMenuFrame("정비", body.str(), options, selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type == MenuResultType::Cancel)
        {
            return {GameState::FloorLoop, "정비를 생략하고 길 선택으로 이동한다."};
        }

        switch (action.index)
        {
        case 0:
            player.hp = player.maxHp;
            player.mp = player.maxMp;
            return {GameState::FloorLoop, "휴식을 마치고 체력을 정돈했다."};

        case 1:
            if (player.gold >= SupplyCost())
            {
                player.gold -= SupplyCost();
                ++player.potionCount;
                return {GameState::FloorLoop, "회복약을 구매해 소지품에 추가했다."};
            }
            return {GameState::Maintenance, "Gold가 부족해 회복약을 구매할 수 없다."};

        case 2:
            if (player.gold >= SupplyCost())
            {
                player.gold -= SupplyCost();
                ++player.etherCount;
                return {GameState::FloorLoop, "마나약을 구매해 소지품에 추가했다."};
            }
            return {GameState::Maintenance, "Gold가 부족해 마나약을 구매할 수 없다."};

        case 3:
            if (player.gold >= MaintenanceCost())
            {
                player.gold -= MaintenanceCost();
                player.atk += 2;
                return {GameState::FloorLoop, "무기를 정비해 공격력이 상승했다."};
            }
            return {GameState::Maintenance, "Gold가 부족해 무기 정비를 진행할 수 없다."};

        case 4:
            if (player.gold >= MaintenanceCost())
            {
                player.gold -= MaintenanceCost();
                player.def += 2;
                return {GameState::FloorLoop, "방어구를 정비해 방어력이 상승했다."};
            }
            return {GameState::Maintenance, "Gold가 부족해 방어구 정비를 진행할 수 없다."};

        default:
            return {GameState::FloorLoop, "정비를 마치고 길 선택으로 향한다."};
        }
    }
}
