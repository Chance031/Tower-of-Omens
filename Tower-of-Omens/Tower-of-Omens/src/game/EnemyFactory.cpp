#include "game/EnemyFactory.h"

#include "game/CsvUtils.h"

#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
// CSV에서 읽어들인 적 한 종류의 원본 데이터를 담는다.
struct EnemyDefinition
{
    int id = 0;
    std::string name;
    std::string battleType; // "Normal" / "Elite" / "Event" / "Boss"
    std::string path;       // "Any" / "Normal" / "Dangerous" / "Unknown"
    int baseHp = 0;
    int baseAtk = 0;
    int goldReward = 0;
    int floorMin = 1;  // 등장 최소 층
    int floorMax = 999;// 등장 최대 층
    std::string description;
};

// 층 스케일링 보너스를 계산한다. 1층은 0, 이후 층마다 step씩 증가.
int FloorBonus(int floor, int step)
{
    if (floor <= 1)
    {
        return 0;
    }

    return (floor - 1) * step;
}

// enemy_base.csv 파일 경로를 반환한다.
std::string ResolveEnemyBaseCsvPath()
{
    return csv::ResolveCsvPath("enemy_base.csv");
}

// CSV 파일에서 전체 적 정의 목록을 파싱해 반환한다.
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

// 적 정의 목록을 최초 1회 로드해 반환한다. (lazy-static)
const std::vector<EnemyDefinition>& EnemyRegistry()
{
    static const std::vector<EnemyDefinition> definitions = LoadEnemyDefinitions();
    return definitions;
}

// BattleType 열거값을 CSV의 battle_type 문자열로 변환한다.
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

// PathChoice 열거값을 CSV의 path 문자열로 변환한다.
std::string PathChoiceKey(PathChoice path)
{
    switch (path)
    {
    case PathChoice::Normal:
        return "Normal";
    case PathChoice::Dangerous:
        return "Dangerous";
    case PathChoice::Unknown:
        return "Unknown";
    }

    return "Any";
}

// 적 정의의 전투 유형이 요청과 일치하는지 확인한다.
bool MatchesBattleType(const EnemyDefinition& definition, BattleType battleType)
{
    return definition.battleType == BattleTypeKey(battleType);
}

// 적 정의의 경로가 요청과 일치하는지 확인한다. "Any"는 항상 통과.
bool MatchesPath(const EnemyDefinition& definition, PathChoice path)
{
    return definition.path == "Any" || definition.path == PathChoiceKey(path);
}

// 적 정의의 등장 층 범위에 현재 층이 포함되는지 확인한다.
bool MatchesFloor(const EnemyDefinition& definition, int floor)
{
    return floor >= definition.floorMin && floor <= definition.floorMax;
}

// 0 이상 maxExclusive 미만의 난수 인덱스를 반환한다.
int RandomIndex(int maxExclusive)
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    std::uniform_int_distribution<int> distribution(0, maxExclusive - 1);
    return distribution(generator);
}

// 적 정의와 현재 층을 바탕으로 스케일링이 적용된 Enemy 인스턴스를 생성한다.
// 보스는 층 스케일링을 적용하지 않는다.
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

// 전투 유형·경로·층을 조건으로 후보를 필터링해 무작위로 적을 생성한다.
// 경로 일치 후보가 없으면 경로를 무시하고 재시도하며, 그래도 없으면 기본 슬라임을 반환한다.
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
