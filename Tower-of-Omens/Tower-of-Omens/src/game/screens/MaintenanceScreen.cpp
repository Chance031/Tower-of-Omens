#include "game/screens/MaintenanceScreen.h"

#include <sstream>
#include <string>
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

// 최근 정비 로그만 남기도록 새 항목을 추가한다.
void PushMaintenanceLog(std::vector<std::string>& logs, const std::string& line)
{
    logs.push_back(line);

    const std::size_t maxLogCount = 5;
    if (logs.size() > maxLogCount)
    {
        logs.erase(logs.begin());
    }
}

// 로그 목록을 화면 출력용 문자열로 합친다.
std::string ComposeLogText(const std::vector<std::string>& logs)
{
    if (logs.empty())
    {
        return "- 아직 정비 기록이 없다.\n";
    }

    std::ostringstream stream;
    for (const std::string& line : logs)
    {
        stream << "- " << line << '\n';
    }

    return stream.str();
}
}

// 정비 화면을 표시하고 선택 결과를 플레이어 상태에 반영한다.
MaintenanceResult MaintenanceScreen::Run(Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const
{
    const std::vector<std::string> options = {"휴식", "회복약 구매", "마나약 구매", "무기 정비", "방어구 정비", "바로 출발"};
    int selected = 0;
    bool canRest = true;
    std::vector<std::string> maintenanceLogs;

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
        body << "휴식: HP / MP를 전부 회복한다. 이번 정비 방문 중 1회만 가능하다.\n";
        body << "회복약 구매: Gold 15를 소모해 회복약 1개를 얻는다.\n";
        body << "마나약 구매: Gold 15를 소모해 마나약 1개를 얻는다.\n";
        body << "무기 정비: Gold 10을 소모해 ATK +2를 얻는다.\n";
        body << "방어구 정비: Gold 10을 소모해 DEF +2를 얻는다.\n\n";
        body << "[정비 기록]\n";
        body << ComposeLogText(maintenanceLogs);
        if (!canRest)
        {
            body << "\n이번 정비에서는 이미 휴식을 마쳤다.\n";
        }

        renderer.Present(renderer.ComposeMenuFrame("정비", body.str(), options, selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type == MenuResultType::Cancel)
        {
            return {GameState::FloorLoop, maintenanceLogs.empty() ? "정비를 마치고 길 선택으로 향한다." : "정비를 마치고 길 선택으로 향한다."};
        }

        switch (action.index)
        {
        case 0:
            if (!canRest)
            {
                PushMaintenanceLog(maintenanceLogs, "이번 정비에서는 더 이상 휴식할 수 없다.");
                continue;
            }

            player.hp = player.maxHp;
            player.mp = player.maxMp;
            canRest = false;
            PushMaintenanceLog(maintenanceLogs, "휴식을 마치고 체력과 마나를 모두 회복했다.");
            continue;

        case 1:
            if (player.gold < SupplyCost())
            {
                PushMaintenanceLog(maintenanceLogs, "Gold가 부족해 회복약을 구매할 수 없다.");
                continue;
            }

            player.gold -= SupplyCost();
            ++player.potionCount;
            PushMaintenanceLog(maintenanceLogs, "회복약을 구매해 소지품에 추가했다.");
            continue;

        case 2:
            if (player.gold < SupplyCost())
            {
                PushMaintenanceLog(maintenanceLogs, "Gold가 부족해 마나약을 구매할 수 없다.");
                continue;
            }

            player.gold -= SupplyCost();
            ++player.etherCount;
            PushMaintenanceLog(maintenanceLogs, "마나약을 구매해 소지품에 추가했다.");
            continue;

        case 3:
            if (player.gold < MaintenanceCost())
            {
                PushMaintenanceLog(maintenanceLogs, "Gold가 부족해 무기 정비를 진행할 수 없다.");
                continue;
            }

            player.gold -= MaintenanceCost();
            player.atk += 2;
            PushMaintenanceLog(maintenanceLogs, "무기를 정비해 공격력이 상승했다.");
            continue;

        case 4:
            if (player.gold < MaintenanceCost())
            {
                PushMaintenanceLog(maintenanceLogs, "Gold가 부족해 방어구 정비를 진행할 수 없다.");
                continue;
            }

            player.gold -= MaintenanceCost();
            player.def += 2;
            PushMaintenanceLog(maintenanceLogs, "방어구를 정비해 방어력이 상승했다.");
            continue;

        default:
            return {GameState::FloorLoop, maintenanceLogs.empty() ? "정비를 마치고 길 선택으로 향한다." : "정비를 마치고 길 선택으로 향한다."};
        }
    }
}
