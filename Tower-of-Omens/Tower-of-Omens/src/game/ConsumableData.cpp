#include "game/ConsumableData.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace
{
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

std::string ResolveConsumableCsvPath()
{
    const std::vector<std::string> candidates = {
        "assets/data/items_consumable.csv",
        "../assets/data/items_consumable.csv",
        "../../assets/data/items_consumable.csv",
        "Tower-of-Omens/assets/data/items_consumable.csv",
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

void SyncLegacyCounts(Player& player)
{
    player.potionCount = GetConsumableCount(player, "201");
    player.etherCount = GetConsumableCount(player, "203");
}
}

const std::vector<ConsumableInfo>& LoadConsumableCatalog()
{
    static const std::vector<ConsumableInfo> catalog = []() {
        const std::string path = ResolveConsumableCsvPath();
        const std::string content = path.empty() ? "" : LoadTextFile(path);
        std::vector<ConsumableInfo> items;

        if (content.empty())
        {
            return items;
        }

        std::stringstream lines(content);
        std::string line;
        bool isHeader = true;

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

            if (isHeader)
            {
                isHeader = false;
                continue;
            }

            const std::vector<std::string> columns = ParseCsvLine(line);
            if (columns.size() < 9)
            {
                continue;
            }

            ConsumableInfo item;
            item.id = Trim(columns[0]);
            item.name = Trim(columns[1]);
            item.description = Trim(columns[2]);
            item.type = Trim(columns[3]);
            item.jobRestriction = Trim(columns[4]);
            item.effect = Trim(columns[5]);
            item.value = ToInt(columns[6]);
            item.buyPrice = ToInt(columns[7]);
            item.sellPrice = ToInt(columns[8]);
            items.push_back(item);
        }

        return items;
    }();

    return catalog;
}

int GetConsumableCount(const Player& player, const std::string& id)
{
    for (const ConsumableStack& stack : player.consumables)
    {
        if (stack.id == id)
        {
            return stack.count;
        }
    }

    return 0;
}

void AddConsumable(Player& player, const std::string& id, int amount)
{
    for (ConsumableStack& stack : player.consumables)
    {
        if (stack.id == id)
        {
            stack.count += amount;
            SyncLegacyCounts(player);
            return;
        }
    }

    player.consumables.push_back({id, amount});
    SyncLegacyCounts(player);
}

bool ConsumeConsumable(Player& player, const std::string& id, int amount)
{
    for (ConsumableStack& stack : player.consumables)
    {
        if (stack.id == id && stack.count >= amount)
        {
            stack.count -= amount;
            SyncLegacyCounts(player);
            return true;
        }
    }

    return false;
}

std::vector<ConsumableInfo> BuildOwnedConsumables(const Player& player)
{
    std::vector<ConsumableInfo> owned;
    for (const ConsumableInfo& item : LoadConsumableCatalog())
    {
        ConsumableInfo copy = item;
        copy.value = item.value;
        const int count = GetConsumableCount(player, item.id);
        if (count > 0)
        {
            owned.push_back(copy);
        }
    }
    return owned;
}

bool ApplyConsumableEffect(Player& player, const ConsumableInfo& item, bool inBattle, std::string& summary)
{
    if (item.effect == "hp")
    {
        if (player.hp >= player.maxHp)
        {
            summary = "HP가 가득 차 있어 사용할 수 없다.";
            return false;
        }
        player.hp = std::min(player.maxHp, player.hp + item.value);
        summary = item.name + "을(를) 사용해 HP를 회복했다.";
        return true;
    }

    if (item.effect == "mp")
    {
        if (player.mp >= player.maxMp)
        {
            summary = "MP가 가득 차 있어 사용할 수 없다.";
            return false;
        }
        player.mp = std::min(player.maxMp, player.mp + item.value);
        summary = item.name + "을(를) 사용해 MP를 회복했다.";
        return true;
    }

    if (item.effect == "hp_mp")
    {
        if (player.hp >= player.maxHp && player.mp >= player.maxMp)
        {
            summary = "HP와 MP가 모두 가득 차 있어 사용할 수 없다.";
            return false;
        }
        player.hp = std::min(player.maxHp, player.hp + item.value);
        player.mp = std::min(player.maxMp, player.mp + item.value);
        summary = item.name + "을(를) 사용해 HP와 MP를 회복했다.";
        return true;
    }

    if (item.effect == "maxHp")
    {
        player.bonusMaxHp += item.value;
        RefreshDerivedStats(player);
        player.hp = std::min(player.maxHp, player.hp + item.value);
        summary = item.name + "을(를) 사용해 최대 HP가 상승했다.";
        return true;
    }

    if (item.effect == "maxMp")
    {
        player.bonusMaxMp += item.value;
        RefreshDerivedStats(player);
        player.mp = std::min(player.maxMp, player.mp + item.value);
        summary = item.name + "을(를) 사용해 최대 MP가 상승했다.";
        return true;
    }

    if (item.effect == "atk_multiplier")
    {
        if (!inBattle)
        {
            summary = "이 아이템은 전투 중에만 사용할 수 있다.";
            return false;
        }
        player.nextAttackMultiplier = std::max(1, item.value);
        summary = item.name + "을(를) 사용해 다음 공격이 강화되었다.";
        return true;
    }

    summary = "아직 처리되지 않은 아이템 효과다.";
    return false;
}
