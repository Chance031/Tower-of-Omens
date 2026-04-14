#include "game/screens/EventScreen.h"

#include "game/ConsumableData.h"
#include "game/screens/MessageScreen.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

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

std::string ResolveCsvPath()
{
    const std::vector<std::string> candidates = {
        "assets/data/events_relic.csv",
        "../assets/data/events_relic.csv",
        "../../assets/data/events_relic.csv",
        "Tower-of-Omens/assets/data/events_relic.csv",
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
    std::unordered_map<std::string, std::size_t> indices;
    for (std::size_t i = 0; i < headers.size(); ++i)
    {
        indices[Trim(headers[i])] = i;
    }
    return indices;
}

std::string GetColumn(
    const std::vector<std::string>& columns,
    const std::unordered_map<std::string, std::size_t>& headers,
    const std::string& name)
{
    const auto found = headers.find(name);
    if (found == headers.end())
    {
        return "";
    }

    const std::size_t index = found->second;
    if (index >= columns.size())
    {
        return "";
    }

    return Trim(columns[index]);
}

std::vector<EventDefinition> LoadEventDefinitionsFromCsv()
{
    const std::string path = ResolveCsvPath();
    if (path.empty())
    {
        return {};
    }

    const std::string content = LoadTextFile(path);
    if (content.empty())
    {
        return {};
    }

    std::vector<EventDefinition> events;
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

        EventDefinition event;
        event.id = ToInt(GetColumn(columns, headerMap, "id"));
        event.name = GetColumn(columns, headerMap, "name");
        event.flavorText = GetColumn(columns, headerMap, "flavor_text");
        event.weight = ToInt(GetColumn(columns, headerMap, "weight"), 100);
        event.minFloor = ToInt(GetColumn(columns, headerMap, "min_floor"), 1);
        event.maxFloor = ToInt(GetColumn(columns, headerMap, "max_floor"), 999);

        const int choiceCount = std::max(0, ToInt(GetColumn(columns, headerMap, "choice_count"), 0));
        for (int i = 1; i <= choiceCount; ++i)
        {
            EventChoice choice;
            choice.label = GetColumn(columns, headerMap, "choice" + std::to_string(i) + "_label");
            choice.effect = GetColumn(columns, headerMap, "choice" + std::to_string(i) + "_effect");
            choice.resultText = GetColumn(columns, headerMap, "choice" + std::to_string(i) + "_result_text");

            if (!choice.label.empty())
            {
                event.choices.push_back(choice);
            }
        }

        if (!event.name.empty() && !event.flavorText.empty() && !event.choices.empty())
        {
            events.push_back(event);
        }
    }

    return events;
}

const std::vector<EventDefinition>& EventRegistry()
{
    static const std::vector<EventDefinition> definitions = LoadEventDefinitionsFromCsv();
    return definitions;
}

std::string ComposeEventBody(const EventDefinition& event, const EventChoice& selectedChoice)
{
    std::ostringstream body;
    body << event.flavorText << "\n\n";
    body << "[선택한 행동]\n";
    body << selectedChoice.label << "\n\n";
    body << "[안내]\n";
    body << "방향키로 선택하고 Enter로 진행한다.";
    return body.str();
}

void AddRelic(Player& player, const std::string& relicName)
{
    if (std::find(player.relicNames.begin(), player.relicNames.end(), relicName) == player.relicNames.end())
    {
        player.relicNames.push_back(relicName);
    }
}

std::string ApplyEffects(Player& player, const std::string& effectText)
{
    const std::vector<std::string> effects = ParseCsvLine(std::string("\"") + effectText + '"');
    const std::vector<std::string> splitByPipe = [&effectText]()
    {
        std::vector<std::string> parts;
        std::stringstream stream(effectText);
        std::string item;
        while (std::getline(stream, item, '|'))
        {
            parts.push_back(item);
        }
        return parts;
    }();

    std::ostringstream summary;
    bool shouldRefreshDerivedStats = false;
    for (const std::string& rawEffect : splitByPipe)
    {
        const std::string effect = Trim(rawEffect);
        if (effect.empty() || effect == "none")
        {
            continue;
        }

        const std::size_t separator = effect.find(':');
        if (separator == std::string::npos)
        {
            continue;
        }

        const std::string key = Trim(effect.substr(0, separator));
        const std::string value = Trim(effect.substr(separator + 1));
        const int amount = ToInt(value, 0);

        if (key == "relic")
        {
            AddRelic(player, value);
            summary << "- 유물 획득: " << value << '\n';
        }
        else if (key == "hp")
        {
            player.hp = std::clamp(player.hp + amount, 0, player.maxHp);
            summary << "- HP " << ((amount >= 0) ? "+" : "") << amount << '\n';
        }
        else if (key == "mp")
        {
            player.mp = std::clamp(player.mp + amount, 0, player.maxMp);
            summary << "- MP " << ((amount >= 0) ? "+" : "") << amount << '\n';
        }
        else if (key == "gold")
        {
            player.gold = std::max(0, player.gold + amount);
            summary << "- Gold " << ((amount >= 0) ? "+" : "") << amount << '\n';
        }
        else if (key == "atk")
        {
            player.bonusAttackPower += amount;
            summary << "- ATK " << ((amount >= 0) ? "+" : "") << amount << '\n';
            shouldRefreshDerivedStats = true;
        }
        else if (key == "def")
        {
            player.bonusDefense += amount;
            summary << "- DEF " << ((amount >= 0) ? "+" : "") << amount << '\n';
            shouldRefreshDerivedStats = true;
        }
        else if (key == "maxHp")
        {
            player.bonusMaxHp += amount;
            summary << "- 최대 HP " << ((amount >= 0) ? "+" : "") << amount << '\n';
            shouldRefreshDerivedStats = true;
        }
        else if (key == "maxMp")
        {
            player.bonusMaxMp += amount;
            summary << "- 최대 MP " << ((amount >= 0) ? "+" : "") << amount << '\n';
            shouldRefreshDerivedStats = true;
        }
        else if (key == "potion")
        {
            player.potionCount = std::max(0, player.potionCount + amount);
            AddConsumable(player, "201", amount);
            summary << "- 회복약 " << ((amount >= 0) ? "+" : "") << amount << '\n';
        }
        else if (key == "ether")
        {
            player.etherCount = std::max(0, player.etherCount + amount);
            AddConsumable(player, "203", amount);
            summary << "- 에테르 " << ((amount >= 0) ? "+" : "") << amount << '\n';
        }
    }

    if (shouldRefreshDerivedStats)
    {
        RefreshDerivedStats(player);
    }

    return summary.str();
}

std::vector<const EventDefinition*> BuildEligibleEvents(const Player& player)
{
    std::vector<const EventDefinition*> eligible;
    for (const EventDefinition& event : EventRegistry())
    {
        if (player.floor < event.minFloor || player.floor > event.maxFloor)
        {
            continue;
        }

        eligible.push_back(&event);
    }

    return eligible;
}

const EventDefinition* SelectEvent(const Player& player)
{
    const std::vector<const EventDefinition*> eligible = BuildEligibleEvents(player);
    if (eligible.empty())
    {
        return nullptr;
    }

    int totalWeight = 0;
    for (const EventDefinition* event : eligible)
    {
        totalWeight += std::max(1, event->weight);
    }

    static std::random_device seed;
    static std::mt19937 generator(seed());
    std::uniform_int_distribution<int> distribution(1, totalWeight);
    int roll = distribution(generator);

    for (const EventDefinition* event : eligible)
    {
        roll -= std::max(1, event->weight);
        if (roll <= 0)
        {
            return event;
        }
    }

    return eligible.back();
}
}

GameState EventScreen::Run(
    Player& player,
    MessageScreen& messageScreen,
    const ConsoleRenderer& renderer,
    const MenuInput& input) const
{
    const EventDefinition* selectedEvent = SelectEvent(player);
    if (selectedEvent == nullptr)
    {
        messageScreen.Show(renderer, input, "이벤트", "현재 층에서 표시할 수 있는 이벤트가 없다.");
        ++player.floor;
        return GameState::Prep;
    }

    int selected = 0;
    std::vector<std::string> options;
    for (const EventChoice& choice : selectedEvent->choices)
    {
        options.push_back(choice.label);
    }

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame(
            selectedEvent->name,
            ComposeEventBody(*selectedEvent, selectedEvent->choices[selected]),
            options,
            selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type == MenuResultType::Cancel)
        {
            continue;
        }

        const EventChoice& chosen = selectedEvent->choices[action.index];
        const std::string effectSummary = ApplyEffects(player, chosen.effect);

        std::ostringstream resultBody;
        resultBody << chosen.resultText;
        if (!effectSummary.empty())
        {
            resultBody << "\n\n[적용 결과]\n" << effectSummary;
        }

        messageScreen.Show(renderer, input, selectedEvent->name, resultBody.str());
        ++player.floor;
        return GameState::Prep;
    }
}
