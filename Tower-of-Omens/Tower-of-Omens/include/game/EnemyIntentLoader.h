#pragma once

#include "game/CsvUtils.h"
#include "game/EnemyIntentData.h"

#include <sstream>
#include <unordered_map>

inline std::unordered_map<int, EnemyIntentData> LoadEnemyIntents(const std::string& csvPath)
{
    const std::string content = csv::LoadTextFile(csvPath);
    if (content.empty())
    {
        return {};
    }

    std::unordered_map<int, EnemyIntentData> intentMap;
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

        EnemyIntentData data;
        data.enemyId = csv::ToInt(csv::GetColumn(columns, headerMap, "id"), 0);
        data.biasAttack = csv::ToInt(csv::GetColumn(columns, headerMap, "intent_bias_attack"), 7);
        data.biasGuard = csv::ToInt(csv::GetColumn(columns, headerMap, "intent_bias_guard"), 1);
        data.biasRecover = csv::ToInt(csv::GetColumn(columns, headerMap, "intent_bias_recover"), 2);
        data.thresholdHp = csv::ToFloat(csv::GetColumn(columns, headerMap, "intent_threshold_hp"), 0.0f);

        if (data.enemyId > 0)
        {
            intentMap[data.enemyId] = data;
        }
    }

    return intentMap;
}

inline EnemyIntentData FindIntentData(
    const std::unordered_map<int, EnemyIntentData>& intentMap,
    int enemyId)
{
    const auto found = intentMap.find(enemyId);
    if (found != intentMap.end())
    {
        return found->second;
    }

    EnemyIntentData fallback;
    fallback.enemyId = enemyId;
    return fallback;
}

inline EnemyIntent DecideEnemyIntent(
    const EnemyIntentData& data,
    int currentHp,
    int maxHp,
    int d20Roll)
{
    int attackWeight = data.biasAttack;
    int guardWeight = data.biasGuard;
    int recoverWeight = data.biasRecover;

    const float hpRatio = (maxHp <= 0) ? 1.0f : static_cast<float>(currentHp) / static_cast<float>(maxHp);
    if (data.thresholdHp > 0.0f && hpRatio <= data.thresholdHp)
    {
        guardWeight *= 2;
        recoverWeight *= 2;
    }

    if (attackWeight <= 0 && guardWeight <= 0 && recoverWeight <= 0)
    {
        attackWeight = 1;
    }

    const int totalWeight = attackWeight + guardWeight + recoverWeight;
    const int scaledRoll = ((d20Roll - 1) % totalWeight) + 1;
    if (scaledRoll <= attackWeight)
    {
        return EnemyIntent::Attack;
    }

    if (scaledRoll <= attackWeight + guardWeight)
    {
        return EnemyIntent::Guard;
    }

    return EnemyIntent::Recover;
}
