#include "game/screens/MaintenanceScreen.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace
{
struct ShopItem
{
    std::string name;
    std::string description;
    int price = 0;
    int ownedCount = 0;
};

struct InventoryItem
{
    std::string name;
    std::string description;
    int count = 0;
};

// 최근 정비 로그만 남기도록 새 항목을 추가한다.
void PushMaintenanceLog(std::vector<std::string>& logs, const std::string& line)
{
    logs.push_back(line);

    const std::size_t maxLogCount = 6;
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

// 현재 직업에 맞는 상점 물품 목록을 만든다.
std::vector<ShopItem> BuildShopItems(const Player& player)
{
    std::vector<ShopItem> items;
    items.push_back({"회복약", "HP를 35 회복하는 소모품이다.", 15, player.potionCount});
    items.push_back({"마나약", "MP를 20 회복하는 소모품이다.", 15, player.etherCount});

    if (player.job == JobClass::Warrior)
    {
        items.push_back({"예리한 검", "공격력 +3이 붙은 예비 무기다.", 40, player.bagWeaponName.empty() ? 0 : 1});
        items.push_back({"강철 방패", "방어력 +3이 붙은 예비 방어구다.", 40, player.bagArmorName.empty() ? 0 : 1});
    }
    else
    {
        items.push_back({"집중의 지팡이", "공격력 +4가 붙은 예비 무기다.", 40, player.bagWeaponName.empty() ? 0 : 1});
        items.push_back({"마력 망토", "방어력 +3이 붙은 예비 방어구다.", 40, player.bagArmorName.empty() ? 0 : 1});
    }

    return items;
}

// 현재 플레이어의 인벤토리 목록을 만든다.
std::vector<InventoryItem> BuildInventoryItems(const Player& player)
{
    return {
        {"회복약", "HP를 35 회복하는 소모품이다.", player.potionCount},
        {"마나약", "MP를 20 회복하는 소모품이다.", player.etherCount},
        {player.bagWeaponName.empty() ? "예비 무기 없음" : player.bagWeaponName,
         player.bagWeaponName.empty() ? "현재 인벤토리에 예비 무기가 없다." : "장착 시 무기 보너스가 적용된다.",
         player.bagWeaponName.empty() ? 0 : 1},
        {player.bagArmorName.empty() ? "예비 방어구 없음" : player.bagArmorName,
         player.bagArmorName.empty() ? "현재 인벤토리에 예비 방어구가 없다." : "장착 시 방어구 보너스가 적용된다.",
         player.bagArmorName.empty() ? 0 : 1}
    };
}

// 정비 메인 화면의 본문 문자열을 만든다.
std::string ComposeMainBody(const Player& player, bool canRecover, const std::vector<std::string>& logs)
{
    std::ostringstream body;
    body << "현재 층: " << player.floor << " | 레벨: " << player.level << '\n';
    body << "플레이어: " << player.name << '\n';
    body << "HP: " << player.hp << '/' << player.maxHp;
    body << " | MP: " << player.mp << '/' << player.maxMp << '\n';
    body << "ATK: " << player.atk << " | DEF: " << player.def << " | Gold: " << player.gold << '\n';
    body << "분배 가능 포인트: " << player.statPoints << '\n';
    body << "무기: " << player.weaponName << " | 방어구: " << player.armorName << "\n\n";
    body << "이번 정비 방문에서 회복은 " << (canRecover ? "가능" : "불가") << "하다.\n\n";
    body << "[정비 기록]\n";
    body << ComposeLogText(logs);
    return body.str();
}

// 회복 화면의 본문 문자열을 만든다.
std::string ComposeRecoverBody(const Player& player, bool canRecover)
{
    std::ostringstream body;
    body << "HP: " << player.hp << '/' << player.maxHp << '\n';
    body << "MP: " << player.mp << '/' << player.maxMp << "\n\n";
    body << "전체 회복: HP와 MP를 모두 가득 채운다.\n";
    body << "체력 회복: HP만 가득 채운다.\n";
    body << "마나 회복: MP만 가득 채운다.\n\n";
    body << "이번 정비에서 회복 가능 여부: " << (canRecover ? "가능" : "불가") << '\n';
    return body.str();
}

// 상태창 메인 본문 문자열을 만든다.
std::string ComposeStatusHubBody(const Player& player)
{
    std::ostringstream body;
    body << "레벨: " << player.level << '\n';
    body << "분배 가능 포인트: " << player.statPoints << "\n\n";
    body << "HP: " << player.hp << '/' << player.maxHp << '\n';
    body << "MP: " << player.mp << '/' << player.maxMp << '\n';
    body << "ATK: " << player.atk << "\n";
    body << "DEF: " << player.def << "\n\n";
    body << "상태창에서는 스탯 분배와 인벤토리 확인을 진행할 수 있다.\n";
    return body.str();
}

// 스탯 분배 화면의 본문 문자열을 만든다.
std::string ComposeStatBody(const Player& player)
{
    std::ostringstream body;
    body << "분배 가능 포인트: " << player.statPoints << "\n\n";
    body << "현재 능력치\n";
    body << "최대 HP: " << player.maxHp << '\n';
    body << "최대 MP: " << player.maxMp << '\n';
    body << "ATK: " << player.atk << '\n';
    body << "DEF: " << player.def << "\n\n";
    body << "원하는 능력치에 포인트를 분배한다.\n";
    return body.str();
}

// 인벤토리 화면의 본문 문자열을 만든다.
std::string ComposeInventoryBody(const Player& player, const InventoryItem& item)
{
    std::ostringstream body;
    body << "[장착 중]\n";
    body << "무기: " << player.weaponName << " (ATK +" << player.weaponAtkBonus << ")\n";
    body << "방어구: " << player.armorName << " (DEF +" << player.armorDefBonus << ")\n\n";
    body << "[선택한 아이템]\n";
    body << "이름: " << item.name << '\n';
    body << "설명: " << item.description << '\n';
    body << "수량: " << item.count << "\n\n";
    body << "선택한 아이템을 사용하거나 장착한다.\n";
    body << "ESC를 누르면 상태창으로 돌아간다.\n";
    return body.str();
}

// 상점 화면의 본문 문자열을 만든다.
std::string ComposeShopBody(const Player& player, const ShopItem& item)
{
    std::ostringstream body;
    body << "Gold: " << player.gold << "\n\n";
    body << "[선택한 물품]\n";
    body << "이름: " << item.name << '\n';
    body << "설명: " << item.description << '\n';
    body << "가격: " << item.price << " Gold\n";
    body << "보유 수량: " << item.ownedCount << "\n\n";
    body << "구매한 장비는 상태창의 인벤토리에서 장착할 수 있다.\n";
    body << "ESC를 누르면 정비 메인으로 돌아간다.\n";
    return body.str();
}
}

// 정비 화면을 표시하고 선택 결과를 플레이어 상태에 반영한다.
MaintenanceResult MaintenanceScreen::Run(Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const
{
    const std::vector<std::string> options = {"회복", "상점", "상태창", "출발"};
    int selected = 0;
    int recoverSelected = 0;
    int statusHubSelected = 0;
    int statSelected = 0;
    int inventorySelected = 0;
    int shopSelected = 0;
    bool canRecover = true;
    std::vector<std::string> maintenanceLogs;

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame("정비", ComposeMainBody(player, canRecover, maintenanceLogs), options, selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type == MenuResultType::Cancel)
        {
            return {GameState::FloorLoop, "정비를 마치고 길 선택으로 향한다."};
        }

        switch (action.index)
        {
        case 0:
        {
            const std::vector<std::string> recoverOptions = {"전체 회복", "체력 회복", "마나 회복", "뒤로"};
            for (;;)
            {
                renderer.Present(renderer.ComposeMenuFrame("회복", ComposeRecoverBody(player, canRecover), recoverOptions, recoverSelected));
                const MenuAction recoverAction = input.ReadMenuSelection(recoverSelected, static_cast<int>(recoverOptions.size()));
                if (recoverAction.type == MenuResultType::Move)
                {
                    recoverSelected = recoverAction.index;
                    continue;
                }

                if (recoverAction.type == MenuResultType::Cancel || recoverAction.index == 3)
                {
                    break;
                }

                if (!canRecover)
                {
                    PushMaintenanceLog(maintenanceLogs, "이번 정비에서는 더 이상 회복할 수 없다.");
                    continue;
                }

                if (recoverAction.index == 0)
                {
                    player.hp = player.maxHp;
                    player.mp = player.maxMp;
                    canRecover = false;
                    PushMaintenanceLog(maintenanceLogs, "전체 회복으로 HP와 MP를 모두 회복했다.");
                    continue;
                }

                if (recoverAction.index == 1)
                {
                    player.hp = player.maxHp;
                    canRecover = false;
                    PushMaintenanceLog(maintenanceLogs, "체력을 모두 회복했다.");
                    continue;
                }

                player.mp = player.maxMp;
                canRecover = false;
                PushMaintenanceLog(maintenanceLogs, "마나를 모두 회복했다.");
            }
            break;
        }

        case 1:
        {
            for (;;)
            {
                const std::vector<ShopItem> shopItems = BuildShopItems(player);
                std::vector<std::string> shopOptions;
                for (const ShopItem& item : shopItems)
                {
                    shopOptions.push_back(item.name);
                }
                shopOptions.push_back("뒤로");

                if (shopSelected >= static_cast<int>(shopOptions.size()))
                {
                    shopSelected = 0;
                }

                const ShopItem& currentItem = shopItems[std::min(shopSelected, static_cast<int>(shopItems.size()) - 1)];
                renderer.Present(renderer.ComposeMenuFrame("상점", ComposeShopBody(player, currentItem), shopOptions, shopSelected));
                const MenuAction shopAction = input.ReadMenuSelection(shopSelected, static_cast<int>(shopOptions.size()));
                if (shopAction.type == MenuResultType::Move)
                {
                    shopSelected = shopAction.index;
                    continue;
                }

                if (shopAction.type == MenuResultType::Cancel || shopAction.index == static_cast<int>(shopOptions.size()) - 1)
                {
                    break;
                }

                const ShopItem& item = shopItems[shopAction.index];
                if (player.gold < item.price)
                {
                    PushMaintenanceLog(maintenanceLogs, "Gold가 부족해 " + item.name + "을(를) 구매할 수 없다.");
                    continue;
                }

                if (shopAction.index == 0)
                {
                    player.gold -= item.price;
                    ++player.potionCount;
                    PushMaintenanceLog(maintenanceLogs, "회복약을 구매했다.");
                    continue;
                }

                if (shopAction.index == 1)
                {
                    player.gold -= item.price;
                    ++player.etherCount;
                    PushMaintenanceLog(maintenanceLogs, "마나약을 구매했다.");
                    continue;
                }

                if (shopAction.index == 2)
                {
                    if (!player.bagWeaponName.empty())
                    {
                        PushMaintenanceLog(maintenanceLogs, "예비 무기 칸이 가득 차 있다.");
                        continue;
                    }
                    player.gold -= item.price;
                    player.bagWeaponName = item.name;
                    player.bagWeaponAtkBonus = (player.job == JobClass::Warrior) ? 3 : 4;
                    PushMaintenanceLog(maintenanceLogs, item.name + "을(를) 구매해 인벤토리에 넣었다.");
                    continue;
                }

                if (!player.bagArmorName.empty())
                {
                    PushMaintenanceLog(maintenanceLogs, "예비 방어구 칸이 가득 차 있다.");
                    continue;
                }
                player.gold -= item.price;
                player.bagArmorName = item.name;
                player.bagArmorDefBonus = 3;
                PushMaintenanceLog(maintenanceLogs, item.name + "을(를) 구매해 인벤토리에 넣었다.");
            }
            break;
        }

        case 2:
        {
            const std::vector<std::string> statusHubOptions = {"스탯 분배", "인벤토리", "뒤로"};
            for (;;)
            {
                renderer.Present(renderer.ComposeMenuFrame("상태창", ComposeStatusHubBody(player), statusHubOptions, statusHubSelected));
                const MenuAction statusAction = input.ReadMenuSelection(statusHubSelected, static_cast<int>(statusHubOptions.size()));
                if (statusAction.type == MenuResultType::Move)
                {
                    statusHubSelected = statusAction.index;
                    continue;
                }

                if (statusAction.type == MenuResultType::Cancel || statusAction.index == 2)
                {
                    break;
                }

                if (statusAction.index == 0)
                {
                    const std::vector<std::string> statOptions = {"HP +10", "MP +8", "ATK +2", "DEF +2", "뒤로"};
                    for (;;)
                    {
                        renderer.Present(renderer.ComposeMenuFrame("스탯 분배", ComposeStatBody(player), statOptions, statSelected));
                        const MenuAction statAction = input.ReadMenuSelection(statSelected, static_cast<int>(statOptions.size()));
                        if (statAction.type == MenuResultType::Move)
                        {
                            statSelected = statAction.index;
                            continue;
                        }

                        if (statAction.type == MenuResultType::Cancel || statAction.index == 4)
                        {
                            break;
                        }

                        if (player.statPoints <= 0)
                        {
                            PushMaintenanceLog(maintenanceLogs, "분배할 스탯 포인트가 없다.");
                            continue;
                        }

                        --player.statPoints;
                        switch (statAction.index)
                        {
                        case 0:
                            player.maxHp += 10;
                            player.hp += 10;
                            PushMaintenanceLog(maintenanceLogs, "최대 HP를 10 올렸다.");
                            break;
                        case 1:
                            player.maxMp += 8;
                            player.mp += 8;
                            PushMaintenanceLog(maintenanceLogs, "최대 MP를 8 올렸다.");
                            break;
                        case 2:
                            player.atk += 2;
                            PushMaintenanceLog(maintenanceLogs, "ATK를 2 올렸다.");
                            break;
                        case 3:
                            player.def += 2;
                            PushMaintenanceLog(maintenanceLogs, "DEF를 2 올렸다.");
                            break;
                        }
                    }
                    continue;
                }

                for (;;)
                {
                    const std::vector<InventoryItem> inventoryItems = BuildInventoryItems(player);
                    std::vector<std::string> inventoryOptions;
                    for (const InventoryItem& item : inventoryItems)
                    {
                        inventoryOptions.push_back(item.name);
                    }
                    inventoryOptions.push_back("뒤로");

                    if (inventorySelected >= static_cast<int>(inventoryOptions.size()))
                    {
                        inventorySelected = 0;
                    }

                    const InventoryItem& currentItem = inventoryItems[std::min(inventorySelected, static_cast<int>(inventoryItems.size()) - 1)];
                    renderer.Present(renderer.ComposeMenuFrame("인벤토리", ComposeInventoryBody(player, currentItem), inventoryOptions, inventorySelected));
                    const MenuAction inventoryAction = input.ReadMenuSelection(inventorySelected, static_cast<int>(inventoryOptions.size()));
                    if (inventoryAction.type == MenuResultType::Move)
                    {
                        inventorySelected = inventoryAction.index;
                        continue;
                    }

                    if (inventoryAction.type == MenuResultType::Cancel || inventoryAction.index == static_cast<int>(inventoryOptions.size()) - 1)
                    {
                        break;
                    }

                    switch (inventoryAction.index)
                    {
                    case 0:
                        if (player.potionCount <= 0)
                        {
                            PushMaintenanceLog(maintenanceLogs, "회복약이 부족하다.");
                            continue;
                        }
                        if (player.hp >= player.maxHp)
                        {
                            PushMaintenanceLog(maintenanceLogs, "HP가 가득 차 있어 회복약을 사용할 수 없다.");
                            continue;
                        }
                        player.hp = std::min(player.maxHp, player.hp + 35);
                        --player.potionCount;
                        PushMaintenanceLog(maintenanceLogs, "인벤토리에서 회복약을 사용했다.");
                        continue;

                    case 1:
                        if (player.etherCount <= 0)
                        {
                            PushMaintenanceLog(maintenanceLogs, "마나약이 부족하다.");
                            continue;
                        }
                        if (player.mp >= player.maxMp)
                        {
                            PushMaintenanceLog(maintenanceLogs, "MP가 가득 차 있어 마나약을 사용할 수 없다.");
                            continue;
                        }
                        player.mp = std::min(player.maxMp, player.mp + 20);
                        --player.etherCount;
                        PushMaintenanceLog(maintenanceLogs, "인벤토리에서 마나약을 사용했다.");
                        continue;

                    case 2:
                        if (player.bagWeaponName.empty())
                        {
                            PushMaintenanceLog(maintenanceLogs, "장착할 예비 무기가 없다.");
                            continue;
                        }
                        player.atk -= player.weaponAtkBonus;
                        player.atk += player.bagWeaponAtkBonus;
                        std::swap(player.weaponName, player.bagWeaponName);
                        std::swap(player.weaponAtkBonus, player.bagWeaponAtkBonus);
                        PushMaintenanceLog(maintenanceLogs, "예비 무기를 장착했다.");
                        continue;

                    case 3:
                        if (player.bagArmorName.empty())
                        {
                            PushMaintenanceLog(maintenanceLogs, "장착할 예비 방어구가 없다.");
                            continue;
                        }
                        player.def -= player.armorDefBonus;
                        player.def += player.bagArmorDefBonus;
                        std::swap(player.armorName, player.bagArmorName);
                        std::swap(player.armorDefBonus, player.bagArmorDefBonus);
                        PushMaintenanceLog(maintenanceLogs, "예비 방어구를 장착했다.");
                        continue;
                    }
                }
            }
            break;
        }

        default:
            return {GameState::FloorLoop, "정비를 마치고 길 선택으로 향한다."};
        }
    }
}
