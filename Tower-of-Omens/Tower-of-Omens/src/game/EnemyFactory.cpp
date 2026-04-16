#include "game/EnemyFactory.h"

#include "game/CsvUtils.h"

#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
struct EnemyDefinition
{
    int id = 0;
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

int FloorBonus(int floor, int step)
{
    if (floor <= 1)
    {
        return 0;
    }

    return (floor - 1) * step;
}

std::string ResolveEnemyBaseCsvPath()
{
    return csv::ResolveCsvPath("enemy_base.csv");
}

std::vector<EnemyDefinition> LoadEnemyDefinitions()
{
    const std::string path = ResolveEnemyBaseCsvPath();
    if (path.empty())
    {
        return {};
    }

    const std::string content = csv::LoadTextFile(path);
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

        if (csv::Trim(line).empty())
        {
            continue;
        }

        const std::vector<std::string> columns = csv::ParseCsvLine(line);
        if (isHeader)
        {
            headerMap = csv::BuildHeaderMap(columns);
            isHeader = false;
            continue;
        }

        EnemyDefinition definition;
        definition.id = csv::ToInt(csv::GetColumn(columns, headerMap, "id"), 0);
        definition.name = csv::GetColumn(columns, headerMap, "name");
        definition.battleType = csv::GetColumn(columns, headerMap, "battle_type");
        definition.path = csv::GetColumn(columns, headerMap, "path");
        definition.baseHp = csv::ToInt(csv::GetColumn(columns, headerMap, "base_hp"), 0);
        definition.baseAtk = csv::ToInt(csv::GetColumn(columns, headerMap, "base_atk"), 0);
        definition.goldReward = csv::ToInt(csv::GetColumn(columns, headerMap, "gold_reward"), 0);
        definition.floorMin = csv::ToInt(csv::GetColumn(columns, headerMap, "floor_min"), 1);
        definition.floorMax = csv::ToInt(csv::GetColumn(columns, headerMap, "floor_max"), 999);
        definition.description = csv::GetColumn(columns, headerMap, "description");

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
        enemy.hp += FloorBonus(floor, 6);
        enemy.atk += FloorBonus(floor, 2);
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
        return {0, "Training Slime", 20, 5, 5};
    }

    const EnemyDefinition& selected = *candidates[RandomIndex(static_cast<int>(candidates.size()))];
    return BuildEnemyFromDefinition(selected, battleType, floor);
}
