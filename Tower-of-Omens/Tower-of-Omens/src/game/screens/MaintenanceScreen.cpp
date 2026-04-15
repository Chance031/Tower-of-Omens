#include "game/screens/MaintenanceScreen.h"
#include "game/ConsumableData.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
enum class ShopItemType
{
    Consumable,
    Weapon,
    Armor
};

struct ShopItem
{
    ShopItemType type = ShopItemType::Consumable;
    std::string id;
    std::string name;
    std::string description;
    int price = 0;
    int ownedCount = 0;
    int atkBonus = 0;
    int defBonus = 0;
};

struct InventoryItem
{
    std::string id;
    std::string name;
    std::string description;
    int count = 0;
    bool isWeapon = false;
    bool isArmor = false;
};

std::string Trim(const std::string& value)
{
    std::size_t start = 0;
    while (start < value.size() && (value[start] == ' ' || value[start] == '\t' || value[start] == '\r'))
    {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r'))
    {
        --end;
    }

    return value.substr(start, end - start);
}

std::vector<std::string> ParseCsvLine(const std::string& line)
{
    std::vector<std::string> columns;
    std::string current;
    bool inQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i)
    {
        const char ch = line[i];
        if (ch == '"')
        {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                current.push_back('"');
                ++i;
                continue;
            }

            inQuotes = !inQuotes;
            continue;
        }

        if (ch == ',' && !inQuotes)
        {
            columns.push_back(current);
            current.clear();
            continue;
        }

        current.push_back(ch);
    }

    columns.push_back(current);
    return columns;
}

int ToInt(const std::string& value, int fallback = 0)
{
    try
    {
        return std::stoi(Trim(value));
    }
    catch (...)
    {
        return fallback;
    }
}

std::string ConvertUtf8ToConsoleEncoding(const std::string& utf8Text)
{
    if (utf8Text.empty())
    {
        return "";
    }

    const int wideLength = MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), static_cast<int>(utf8Text.size()), nullptr, 0);
    if (wideLength <= 0)
    {
        return utf8Text;
    }

    std::wstring wideText(static_cast<std::size_t>(wideLength), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), static_cast<int>(utf8Text.size()), wideText.data(), wideLength);

    const int encodedLength = WideCharToMultiByte(949, 0, wideText.c_str(), wideLength, nullptr, 0, nullptr, nullptr);
    if (encodedLength <= 0)
    {
        return utf8Text;
    }

    std::string converted(static_cast<std::size_t>(encodedLength), '\0');
    WideCharToMultiByte(949, 0, wideText.c_str(), wideLength, converted.data(), encodedLength, nullptr, nullptr);
    return converted;
}

std::string LoadTextFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    std::string content = buffer.str();
    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF)
    {
        content.erase(0, 3);
    }

    return ConvertUtf8ToConsoleEncoding(content);
}

std::string ResolveCsvPath(const std::string& fileName)
{
    const std::vector<std::string> candidates = {
        "assets/data/" + fileName,
        "../assets/data/" + fileName,
        "../../assets/data/" + fileName,
        "Tower-of-Omens/assets/data/" + fileName,
    };

    for (const std::string& path : candidates)
    {
        std::ifstream file(path, std::ios::binary);
        if (file)
        {
            return path;
        }
    }

    return "";
}

std::unordered_map<std::string, std::size_t> BuildHeaderMap(const std::vector<std::string>& headers)
{
    std::unordered_map<std::string, std::size_t> map;
    for (std::size_t i = 0; i < headers.size(); ++i)
    {
        map[Trim(headers[i])] = i;
    }
    return map;
}

std::string GetColumn(
    const std::vector<std::string>& columns,
    const std::unordered_map<std::string, std::size_t>& headers,
    const std::string& key)
{
    const auto found = headers.find(key);
    if (found == headers.end() || found->second >= columns.size())
    {
        return "";
    }

    return Trim(columns[found->second]);
}

bool JobMatches(const Player& player, const std::string& restriction)
{
    if (restriction.empty() || restriction == "none")
    {
        return true;
    }

    return (restriction == "Warrior" && player.job == JobClass::Warrior) ||
        (restriction == "Mage" && player.job == JobClass::Mage);
}

int CountOwnedCopies(const Player& player, const std::string& name)
{
    int count = 0;
    if (player.weaponName == name || player.bagWeaponName == name)
    {
        ++count;
    }

    if (player.armorName == name || player.bagArmorName == name)
    {
        ++count;
    }

    return count;
}

std::vector<ShopItem> LoadEquipmentShopItems(const Player& player, const std::string& fileName, ShopItemType type)
{
    const std::string path = ResolveCsvPath(fileName);
    if (path.empty())
    {
        return {};
    }

    const std::string content = LoadTextFile(path);
    if (content.empty())
    {
        return {};
    }

    std::vector<ShopItem> items;
    std::stringstream lines(content);
    std::string line;
    bool isHeader = true;
    std::unordered_map<std::string, std::size_t> headerMap;

    while (std::getline(lines, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (Trim(line).empty())
        {
            continue;
        }

        const std::vector<std::string> columns = ParseCsvLine(line);
        if (isHeader)
        {
            headerMap = BuildHeaderMap(columns);
            isHeader = false;
            continue;
        }

        const int buyPrice = ToInt(GetColumn(columns, headerMap, "buy_price"), 0);
        const int floorMin = ToInt(GetColumn(columns, headerMap, "floor_min"), 1);
        const int floorMax = ToInt(GetColumn(columns, headerMap, "floor_max"), 999);
        const std::string jobRestriction = GetColumn(columns, headerMap, "job_restriction");

        if (buyPrice <= 0 || player.floor < floorMin || player.floor > floorMax || !JobMatches(player, jobRestriction))
        {
            continue;
        }

        ShopItem item;
        item.type = type;
        item.id = GetColumn(columns, headerMap, "id");
        item.name = GetColumn(columns, headerMap, "name");
        item.description = GetColumn(columns, headerMap, "description");
        item.price = buyPrice;
        item.ownedCount = CountOwnedCopies(player, item.name);
        item.atkBonus = ToInt(GetColumn(columns, headerMap, "atk_bonus"), 0);
        item.defBonus = ToInt(GetColumn(columns, headerMap, "def_bonus"), 0);

        if (!item.name.empty())
        {
            items.push_back(item);
        }
    }

    return items;
}

std::string MakeBar(int current, int maximum, int width, char filled, char empty)
{
    if (maximum <= 0)
    {
        return std::string(width, empty);
    }

    const int clampedCurrent = std::max(0, std::min(current, maximum));
    const int filledCount = (clampedCurrent * width) / maximum;
    return std::string(filledCount, filled) + std::string(width - filledCount, empty);
}

void PushMaintenanceLog(std::vector<std::string>& logs, const std::string& line)
{
    logs.push_back(line);

    if (logs.size() > 6)
    {
        logs.erase(logs.begin());
    }
}

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

std::string ComposeRelicText(const Player& player)
{
    if (player.relicNames.empty())
    {
        return "없음\n";
    }

    std::ostringstream stream;
    for (const std::string& relicName : player.relicNames)
    {
        stream << "- " << relicName << '\n';
    }

    return stream.str();
}

std::string ComposeStatusPanel(const Player& player)
{
    std::ostringstream body;
    body << "[탐험자 상태]\n";
    body << player.name << " | 층 " << player.floor << " | Lv " << player.level << '\n';
    body << "HP [" << MakeBar(player.hp, player.maxHp, 20, '#', '.') << "] " << player.hp << '/' << player.maxHp << '\n';
    body << "MP [" << MakeBar(player.mp, player.maxMp, 20, '@', '.') << "] " << player.mp << '/' << player.maxMp << '\n';
    body << "STR " << player.strength << " | AGI " << player.agility << " | INT " << player.intelligence << " | MND " << player.spirit << '\n';
    body << "GOLD " << player.gold << " | 스탯 포인트 " << player.statPoints << '\n';
    body << "무기 " << player.weaponName << " (공격 보정 +" << player.weaponAtkBonus << ")\n";
    body << "방어구 " << player.armorName << " (방어 보정 +" << player.armorDefBonus << ")\n";
    body << "상태이상 ";
    if (player.burnTurns <= 0 && player.wetTurns <= 0 && player.bindTurns <= 0 && player.staggerTurns <= 0)
    {
        body << "없음\n";
    }
    else
    {
        bool first = true;
        if (player.burnTurns > 0)
        {
            body << (first ? "" : ", ") << "화상 " << player.burnTurns << "턴";
            first = false;
        }
        if (player.wetTurns > 0)
        {
            body << (first ? "" : ", ") << "습기 " << player.wetTurns << "턴";
            first = false;
        }
        if (player.bindTurns > 0)
        {
            body << (first ? "" : ", ") << "속박 " << player.bindTurns << "턴";
            first = false;
        }
        if (player.staggerTurns > 0)
        {
            body << (first ? "" : ", ") << "경직 " << player.staggerTurns << "턴";
        }
        body << '\n';
    }
    body << "유물 " << player.relicNames.size() << "개\n";
    return body.str();
}

std::vector<ShopItem> BuildShopItems(const Player& player)
{
    std::vector<ShopItem> items;

    for (const ConsumableInfo& consumable : LoadConsumableCatalog())
    {
        if (consumable.buyPrice <= 0)
        {
            continue;
        }

        items.push_back({
            ShopItemType::Consumable,
            consumable.id,
            consumable.name,
            consumable.description,
            consumable.buyPrice,
            GetConsumableCount(player, consumable.id),
            0,
            0});
    }

    const std::vector<ShopItem> weaponItems = LoadEquipmentShopItems(player, "items_weapon.csv", ShopItemType::Weapon);
    const std::vector<ShopItem> armorItems = LoadEquipmentShopItems(player, "items_armor.csv", ShopItemType::Armor);
    items.insert(items.end(), weaponItems.begin(), weaponItems.end());
    items.insert(items.end(), armorItems.begin(), armorItems.end());
    return items;
}

std::vector<InventoryItem> BuildInventoryItems(const Player& player)
{
    std::vector<InventoryItem> items;

    for (const ConsumableInfo& consumable : BuildOwnedConsumables(player))
    {
        const int count = GetConsumableCount(player, consumable.id);
        if (count <= 0)
        {
            continue;
        }

        items.push_back({consumable.id, consumable.name, consumable.description, count, false, false});
    }

    if (!player.bagWeaponName.empty())
    {
        items.push_back({"", player.bagWeaponName, "장착 시 무기 보너스가 적용된다.", 1, true, false});
    }

    if (!player.bagArmorName.empty())
    {
        items.push_back({"", player.bagArmorName, "장착 시 방어구 보너스가 적용된다.", 1, false, true});
    }

    return items;
}

std::string ComposeMainBody(const Player& player, bool canRecover, const std::vector<std::string>& logs)
{
    std::ostringstream body;
    body << ComposeStatusPanel(player) << '\n';
    body << "------------------------------------------------------------\n";
    body << "[유물 목록]\n";
    body << ComposeRelicText(player) << '\n';
    body << "[정비 안내]\n";
    body << "이번 정비 방문에서 회복은 " << (canRecover ? "가능" : "불가") << "하다.\n\n";
    body << "[정비 기록]\n";
    body << ComposeLogText(logs);
    return body.str();
}

std::string ComposeRecoverBody(const Player& player, bool canRecover)
{
    std::ostringstream body;
    body << ComposeStatusPanel(player) << '\n';
    body << "------------------------------------------------------------\n";
    body << "[회복 안내]\n";
    body << "전체 회복: HP와 MP를 모두 가득 채운다.\n";
    body << "체력 회복: HP만 가득 채운다.\n";
    body << "마나 회복: MP만 가득 채운다.\n\n";
    body << "이번 정비에서 회복 가능 여부: " << (canRecover ? "가능" : "불가") << '\n';
    return body.str();
}

std::string ComposeStatusHubBody(const Player& player)
{
    std::ostringstream body;
    body << ComposeStatusPanel(player) << '\n';
    body << "------------------------------------------------------------\n";
    body << "[유물 목록]\n";
    body << ComposeRelicText(player) << '\n';
    body << "[상태창 안내]\n";
    body << "4개 스탯 분배와 인벤토리 관리를 진행할 수 있다.\n";
    return body.str();
}

std::string ComposeStatBody(const Player& player)
{
    std::ostringstream body;
    body << ComposeStatusPanel(player) << '\n';
    body << "------------------------------------------------------------\n";
    body << "[스탯 분배]\n";
    body << "분배 가능 포인트: " << player.statPoints << "\n\n";
    body << "한 번 선택할 때마다 해당 스탯이 1 오른다.\n";
    body << "공격력은 직접 올리지 않으며 STR/INT 기반으로 자동 계산된다.\n";
    return body.str();
}

std::string ComposeInventoryBody(const Player& player, const InventoryItem* item)
{
    std::ostringstream body;
    body << ComposeStatusPanel(player) << '\n';
    body << "------------------------------------------------------------\n";

    if (item == nullptr)
    {
        body << "[인벤토리]\n";
        body << "현재 보유 중인 아이템이 없다.\n";
        return body.str();
    }

    body << "[선택한 아이템]\n";
    body << "이름: " << item->name << '\n';
    body << "설명: " << item->description << '\n';
    body << "수량: " << item->count << "\n\n";
    body << "선택한 아이템을 사용하거나 장착한다.\n";
    body << "ESC를 누르면 상태창으로 돌아간다.\n";
    return body.str();
}

std::string ComposeShopBody(const Player& player, const ShopItem* item)
{
    std::ostringstream body;
    body << ComposeStatusPanel(player) << '\n';
    body << "------------------------------------------------------------\n";

    if (item == nullptr)
    {
        body << "[상점]\n";
        body << "현재 판매 중인 물품이 없다.\n";
        return body.str();
    }

    body << "[선택한 물품]\n";
    body << "이름: " << item->name << '\n';
    body << "설명: " << item->description << '\n';
    body << "가격: " << item->price << " Gold\n";
    body << "보유 수량: " << item->ownedCount << "\n\n";
    body << "상점 판매용 목록만 표시된다.\n";
    body << "ESC를 누르면 정비 메인으로 돌아간다.\n";
    return body.str();
}
}

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
            return {GameState::FloorSelect, "정비를 마치고 길 선택으로 향한다."};
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

                const ShopItem* currentItem = shopItems.empty()
                    ? nullptr
                    : &shopItems[std::min(shopSelected, static_cast<int>(shopItems.size()) - 1)];

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

                if (shopItems.empty())
                {
                    break;
                }

                const ShopItem& item = shopItems[shopAction.index];
                if (player.gold < item.price)
                {
                    PushMaintenanceLog(maintenanceLogs, "Gold가 부족해 " + item.name + "을(를) 구매할 수 없다.");
                    continue;
                }

                if (item.type == ShopItemType::Consumable)
                {
                    player.gold -= item.price;
                    AddConsumable(player, item.id, 1);
                    PushMaintenanceLog(maintenanceLogs, item.name + "을(를) 구매했다.");
                    continue;
                }

                if (item.type == ShopItemType::Weapon)
                {
                    if (!player.bagWeaponName.empty())
                    {
                        PushMaintenanceLog(maintenanceLogs, "예비 무기 칸이 가득 차 있다.");
                        continue;
                    }

                    player.gold -= item.price;
                    player.bagWeaponName = item.name;
                    player.bagWeaponAtkBonus = item.atkBonus;
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
                player.bagArmorDefBonus = item.defBonus;
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
                    const std::vector<std::string> statOptions = {"STR +1", "AGI +1", "INT +1", "MND +1", "뒤로"};
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
                            ++player.strength;
                            PushMaintenanceLog(maintenanceLogs, "STR을 1 올렸다.");
                            break;
                        case 1:
                            ++player.agility;
                            PushMaintenanceLog(maintenanceLogs, "AGI를 1 올렸다.");
                            break;
                        case 2:
                            ++player.intelligence;
                            PushMaintenanceLog(maintenanceLogs, "INT를 1 올렸다.");
                            break;
                        case 3:
                            ++player.spirit;
                            PushMaintenanceLog(maintenanceLogs, "MND를 1 올렸다.");
                            break;
                        default:
                            break;
                        }

                        RefreshDerivedStats(player);
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

                    const InventoryItem* currentItem = inventoryItems.empty()
                        ? nullptr
                        : &inventoryItems[std::min(inventorySelected, static_cast<int>(inventoryItems.size()) - 1)];

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

                    if (inventoryItems.empty())
                    {
                        break;
                    }

                    const InventoryItem& selectedItem = inventoryItems[inventoryAction.index];
                    if (!selectedItem.id.empty())
                    {
                        std::string summary;
                        const std::vector<ConsumableInfo> ownedConsumables = BuildOwnedConsumables(player);
                        const auto foundConsumable = std::find_if(
                            ownedConsumables.begin(),
                            ownedConsumables.end(),
                            [&selectedItem](const ConsumableInfo& consumable)
                            {
                                return consumable.id == selectedItem.id;
                            });

                        if (foundConsumable == ownedConsumables.end())
                        {
                            continue;
                        }

                        if (!ApplyConsumableEffect(player, *foundConsumable, false, summary))
                        {
                            PushMaintenanceLog(maintenanceLogs, summary);
                            continue;
                        }

                        ConsumeConsumable(player, foundConsumable->id, 1);
                        PushMaintenanceLog(maintenanceLogs, summary);
                        continue;
                    }

                    if (selectedItem.isWeapon && !player.bagWeaponName.empty() && selectedItem.name == player.bagWeaponName)
                    {
                        std::swap(player.weaponName, player.bagWeaponName);
                        std::swap(player.weaponAtkBonus, player.bagWeaponAtkBonus);
                        RefreshDerivedStats(player);
                        PushMaintenanceLog(maintenanceLogs, "예비 무기를 장착했다.");
                        continue;
                    }

                    if (selectedItem.isArmor && !player.bagArmorName.empty() && selectedItem.name == player.bagArmorName)
                    {
                        std::swap(player.armorName, player.bagArmorName);
                        std::swap(player.armorDefBonus, player.bagArmorDefBonus);
                        RefreshDerivedStats(player);
                        PushMaintenanceLog(maintenanceLogs, "예비 방어구를 장착했다.");
                        continue;
                    }
                }
            }
            break;
        }

        default:
            return {GameState::FloorSelect, "정비를 마치고 길 선택으로 향한다."};
        }
    }
}
