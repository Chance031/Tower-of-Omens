#include "game/ConsumableData.h"

#include "game/CsvUtils.h"

#include <algorithm>
#include <sstream>

namespace
{
std::string Trim(const std::string& value)
{
    return csv::Trim(value);
}

std::vector<std::string> ParseCsvLine(const std::string& line)
{
    return csv::ParseCsvLine(line);
}

int ToInt(const std::string& value, int fallback = 0)
{
    return csv::ToInt(value, fallback);
}

std::string LoadTextFile(const std::string& path)
{
    return csv::LoadTextFile(path);
}

std::string ResolveConsumableCsvPath()
{
    return csv::ResolveCsvPath("items_consumable.csv");
}

}

// items_consumable.csv를 최초 1회 파싱해 전체 소모품 목록을 반환한다. (lazy-static)
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

// 플레이어 인벤토리에서 해당 id의 소모품 개수를 반환한다. 없으면 0.
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

// 소모품 수량을 amount만큼 증감한다. 0 미만으로는 내려가지 않는다.
// amount가 양수인데 스택이 없으면 새로 추가한다.
void AddConsumable(Player& player, const std::string& id, int amount)
{
    for (ConsumableStack& stack : player.consumables)
    {
        if (stack.id == id)
        {
            stack.count = std::max(0, stack.count + amount);
            return;
        }
    }

    if (amount > 0)
    {
        player.consumables.push_back({id, amount});
    }
}

// 소모품을 amount개 소모한다. 보유량이 충분하면 차감 후 true, 아니면 false.
bool ConsumeConsumable(Player& player, const std::string& id, int amount)
{
    for (ConsumableStack& stack : player.consumables)
    {
        if (stack.id == id && stack.count >= amount)
        {
            stack.count -= amount;
            return true;
        }
    }

    return false;
}

// 카탈로그 순서대로 플레이어가 보유 중인 소모품 목록만 추려 반환한다.
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

// 아이템의 effect 키에 따라 효과를 플레이어에게 적용한다.
// inBattle이 false면 전투 전용 아이템 사용을 막는다.
// 성공 여부와 결과 요약 문자열을 반환한다.
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
