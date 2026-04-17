#pragma once

#include "game/CsvUtils.h"
#include "game/Enemy.h"
#include "game/Enums.h"
#include "game/Player.h"
#include "game/battle/BattleCalculations.h"
#include "game/battle/BattleTypes.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace battle
{
struct EnemyStatusPattern
{
    int enemyId = 0;
    std::vector<std::string> battleTypes;
    std::string statusType;
    int applyDifficulty = 0;
    int durationTurns = 0;
    std::string triggerCondition;
    int triggerChance = 0;
};

inline std::string ResolveStatusCsvPath(const std::string& fileName)
{
    return csv::ResolveCsvPath(fileName);
}

inline std::vector<std::string> SplitByPipe(const std::string& value)
{
    std::vector<std::string> parts;
    std::stringstream stream(value);
    std::string part;
    while (std::getline(stream, part, '|'))
    {
        parts.push_back(csv::Trim(part));
    }
    return parts;
}

inline std::vector<EnemyStatusPattern> LoadEnemyStatusPatterns()
{
    const std::string path = ResolveStatusCsvPath("enemy_status_patterns.csv");
    if (path.empty())
    {
        return {};
    }

    const std::string content = csv::LoadTextFile(path);
    if (content.empty())
    {
        return {};
    }

    std::vector<EnemyStatusPattern> patterns;
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

        EnemyStatusPattern pattern;
        pattern.enemyId = csv::ToInt(csv::GetColumn(columns, headerMap, "enemy_id"), 0);
        pattern.battleTypes = SplitByPipe(csv::GetColumn(columns, headerMap, "battle_type"));
        pattern.statusType = csv::GetColumn(columns, headerMap, "status_type");
        pattern.applyDifficulty = csv::ToInt(csv::GetColumn(columns, headerMap, "apply_difficulty"), 0);
        pattern.durationTurns = csv::ToInt(csv::GetColumn(columns, headerMap, "duration_turns"), 0);
        pattern.triggerCondition = csv::GetColumn(columns, headerMap, "trigger_condition");
        pattern.triggerChance = csv::ToInt(csv::GetColumn(columns, headerMap, "trigger_chance"), 0);

        if (pattern.enemyId > 0 && pattern.statusType != "none")
        {
            patterns.push_back(pattern);
        }
    }

    return patterns;
}

inline const std::vector<EnemyStatusPattern>& EnemyStatusPatternRegistry()
{
    static const std::vector<EnemyStatusPattern> patterns = LoadEnemyStatusPatterns();
    return patterns;
}

inline void ApplyPlayerStatus(Player& player, const std::string& statusName, int turns)
{
    if (statusName == "화상")
    {
        player.burnTurns = std::max(player.burnTurns, turns);
    }
    else if (statusName == "습기")
    {
        player.wetTurns = std::max(player.wetTurns, turns);
    }
    else if (statusName == "속박")
    {
        player.bindTurns = std::max(player.bindTurns, turns);
    }
    else if (statusName == "경직")
    {
        player.staggerTurns = std::max(player.staggerTurns, turns);
    }
}

inline void ApplyEnemyStatus(EnemyStatusState& status, const std::string& statusName, int turns)
{
    if (statusName == "화상")
    {
        status.burnTurns = std::max(status.burnTurns, turns);
    }
    else if (statusName == "습기")
    {
        status.wetTurns = std::max(status.wetTurns, turns);
    }
    else if (statusName == "속박")
    {
        status.bindTurns = std::max(status.bindTurns, turns);
    }
    else if (statusName == "경직")
    {
        status.staggerTurns = std::max(status.staggerTurns, turns);
    }
}

inline void DecayPlayerStatuses(Player& player)
{
    player.burnTurns = std::max(0, player.burnTurns - 1);
    player.wetTurns = std::max(0, player.wetTurns - 1);
    player.bindTurns = std::max(0, player.bindTurns - 1);
}

inline void DecayEnemyStatuses(EnemyStatusState& status)
{
    status.burnTurns = std::max(0, status.burnTurns - 1);
    status.wetTurns = std::max(0, status.wetTurns - 1);
    status.bindTurns = std::max(0, status.bindTurns - 1);
}

inline std::string BattleTypeKey(BattleType battleType)
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

inline bool PatternMatchesBattleType(const EnemyStatusPattern& pattern, BattleType battleType)
{
    const std::string key = BattleTypeKey(battleType);
    return std::find(pattern.battleTypes.begin(), pattern.battleTypes.end(), key) != pattern.battleTypes.end();
}

inline bool EvaluateTriggerCondition(
    const EnemyStatusPattern& pattern,
    const Enemy& enemy,
    int enemyHp,
    int turnCount)
{
    if (pattern.triggerCondition == "always")
    {
        return true;
    }

    if (pattern.triggerCondition == "hp_below_50")
    {
        return enemy.hp > 0 && (enemyHp * 100) / enemy.hp <= 50;
    }

    if (pattern.triggerCondition == "hp_below_70")
    {
        return enemy.hp > 0 && (enemyHp * 100) / enemy.hp <= 70;
    }

    if (pattern.triggerCondition == "every_2turns")
    {
        return turnCount > 0 && (turnCount % 2 == 0);
    }

    if (pattern.triggerCondition == "every_3turns")
    {
        return turnCount > 0 && (turnCount % 3 == 0);
    }

    if (pattern.triggerCondition == "phase2")
    {
        return enemy.hp > 0 && (enemyHp * 100) / enemy.hp <= 50;
    }

    return false;
}

template <typename LogFn>
inline void TryApplyEnemyPatterns(
    Player& player,
    const Enemy& enemy,
    BattleType battleType,
    int enemyHp,
    int turnCount,
    LogFn&& pushLog)
{
    for (const EnemyStatusPattern& pattern : EnemyStatusPatternRegistry())
    {
        if (pattern.enemyId != enemy.id || !PatternMatchesBattleType(pattern, battleType))
        {
            continue;
        }

        if (!EvaluateTriggerCondition(pattern, enemy, enemyHp, turnCount))
        {
            continue;
        }

        if (pattern.triggerChance > 0 && RandomPercent() > pattern.triggerChance)
        {
            continue;
        }

        const D20Check resistCheck = MakeD20Check(player.spirit, pattern.applyDifficulty);
        if (resistCheck.success)
        {
            pushLog(enemy.name + "의 " + pattern.statusType + " 저항 성공: " + FormatD20Check(resistCheck));
            continue;
        }

        ApplyPlayerStatus(player, pattern.statusType, pattern.durationTurns);
        pushLog(
            enemy.name + "이(가) " + pattern.statusType + " 부여 성공: " + FormatD20Check(resistCheck) +
            " (" + std::to_string(pattern.durationTurns) + "턴)");
    }
}
}
