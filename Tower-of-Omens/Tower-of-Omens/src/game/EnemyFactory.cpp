#include "game/EnemyFactory.h"

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
struct EnemyDefinition
{
    std::string id;
    std::string name;
    std::string battleType;
    std::string path;
    int baseHp = 0;
    int baseAtk = 0;
    int goldReward = 0;
    int floorMin = 1;
    int floorMax = 999;
    std::string description;
};

// 현재 층을 바탕으로 적 스탯 보정값을 계산한다.
int FloorBonus(int floor, int step)
{
    if (floor <= 1)
    {
        return 0;
    }

    return (floor - 1) * step;
}

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
        "assets/data/enemy_base.csv",
        "../assets/data/enemy_base.csv",
        "../../assets/data/enemy_base.csv",
        "Tower-of-Omens/assets/data/enemy_base.csv",
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

std::vector<EnemyDefinition> LoadEnemyDefinitions()
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

    std::vector<EnemyDefinition> definitions;
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

        EnemyDefinition definition;
        definition.id = GetColumn(columns, headerMap, "id");
        definition.name = GetColumn(columns, headerMap, "name");
        definition.battleType = GetColumn(columns, headerMap, "battle_type");
        definition.path = GetColumn(columns, headerMap, "path");
        definition.baseHp = ToInt(GetColumn(columns, headerMap, "base_hp"), 0);
        definition.baseAtk = ToInt(GetColumn(columns, headerMap, "base_atk"), 0);
        definition.goldReward = ToInt(GetColumn(columns, headerMap, "gold_reward"), 0);
        definition.floorMin = ToInt(GetColumn(columns, headerMap, "floor_min"), 1);
        definition.floorMax = ToInt(GetColumn(columns, headerMap, "floor_max"), 999);
        definition.description = GetColumn(columns, headerMap, "description");

        if (!definition.name.empty())
        {
            definitions.push_back(definition);
        }
    }

    return definitions;
}

const std::vector<EnemyDefinition>& EnemyRegistry()
{
    static const std::vector<EnemyDefinition> definitions = LoadEnemyDefinitions();
    return definitions;
}

std::string BattleTypeKey(BattleType battleType)
{
    switch (battleType)
    {
    case BattleType::Normal:
        return "Normal";
    case BattleType::Elite:
        return "Elite";
    case BattleType::Event:
        return "Event";
    case BattleType::Boss:
        return "Boss";
    }

    return "Normal";
}

std::string PathChoiceKey(PathChoice path)
{
    switch (path)
    {
    case PathChoice::Normal:
        return "Normal";
    case PathChoice::Safe:
        return "Safe";
    case PathChoice::Dangerous:
        return "Dangerous";
    case PathChoice::Unknown:
        return "Unknown";
    }

    return "Any";
}

bool MatchesBattleType(const EnemyDefinition& definition, BattleType battleType)
{
    return definition.battleType == BattleTypeKey(battleType);
}

bool MatchesPath(const EnemyDefinition& definition, PathChoice path)
{
    return definition.path == "Any" || definition.path == PathChoiceKey(path);
}

bool MatchesFloor(const EnemyDefinition& definition, int floor)
{
    return floor >= definition.floorMin && floor <= definition.floorMax;
}

int RandomIndex(int maxExclusive)
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    std::uniform_int_distribution<int> distribution(0, maxExclusive - 1);
    return distribution(generator);
}

Enemy BuildEnemyFromDefinition(const EnemyDefinition& definition, BattleType battleType, int floor)
{
    Enemy enemy;
    enemy.id = definition.id;
    enemy.name = definition.name;
    enemy.hp = definition.baseHp;
    enemy.atk = definition.baseAtk;
    enemy.goldReward = definition.goldReward;

    if (battleType != BattleType::Boss)
    {
        enemy.hp        += FloorBonus(floor, 6);
        enemy.atk       += FloorBonus(floor, 2);
        enemy.goldReward += FloorBonus(floor, 3);
    }

    return enemy;
}
}

Enemy EnemyFactory::Create(BattleType battleType, PathChoice path, int floor) const
{
    std::vector<const EnemyDefinition*> candidates;
    for (const EnemyDefinition& definition : EnemyRegistry())
    {
        if (!MatchesBattleType(definition, battleType) || !MatchesFloor(definition, floor))
        {
            continue;
        }

        if (!MatchesPath(definition, path))
        {
            continue;
        }

        candidates.push_back(&definition);
    }

    if (candidates.empty())
    {
        for (const EnemyDefinition& definition : EnemyRegistry())
        {
            if (!MatchesBattleType(definition, battleType) || !MatchesFloor(definition, floor))
            {
                continue;
            }

            candidates.push_back(&definition);
        }
    }

    if (candidates.empty())
    {
        return {"fallback_enemy", "Training Slime", 20, 5, 5};
    }

    const EnemyDefinition& selected = *candidates[RandomIndex(static_cast<int>(candidates.size()))];
    return BuildEnemyFromDefinition(selected, battleType, floor);
}
