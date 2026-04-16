#pragma once

#include "game/EnemyIntentData.h"

#define NOMINMAX
#include <Windows.h>

#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace enemy_intent_loader_detail
{
inline std::string Trim(const std::string& value)
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

inline std::vector<std::string> ParseCsvLine(const std::string& line)
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

inline int ToInt(const std::string& value, int fallback = 0)
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

inline float ToFloat(const std::string& value, float fallback = 0.0f)
{
    try
    {
        return std::stof(Trim(value));
    }
    catch (...)
    {
        return fallback;
    }
}

inline std::string ConvertUtf8ToConsoleEncoding(const std::string& utf8Text)
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

inline std::string LoadTextFile(const std::string& path)
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

inline std::unordered_map<std::string, std::size_t> BuildHeaderMap(const std::vector<std::string>& headers)
{
    std::unordered_map<std::string, std::size_t> map;
    for (std::size_t i = 0; i < headers.size(); ++i)
    {
        map[Trim(headers[i])] = i;
    }
    return map;
}

inline std::string GetColumn(
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
}

inline std::unordered_map<int, EnemyIntentData> LoadEnemyIntents(const std::string& csvPath)
{
    const std::string content = enemy_intent_loader_detail::LoadTextFile(csvPath);
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

        if (enemy_intent_loader_detail::Trim(line).empty())
        {
            continue;
        }

        const std::vector<std::string> columns = enemy_intent_loader_detail::ParseCsvLine(line);
        if (isHeader)
        {
            headerMap = enemy_intent_loader_detail::BuildHeaderMap(columns);
            isHeader = false;
            continue;
        }

        EnemyIntentData data;
        data.enemyId = enemy_intent_loader_detail::ToInt(enemy_intent_loader_detail::GetColumn(columns, headerMap, "id"), 0);
        data.biasAttack = enemy_intent_loader_detail::ToInt(enemy_intent_loader_detail::GetColumn(columns, headerMap, "intent_bias_attack"), 7);
        data.biasGuard = enemy_intent_loader_detail::ToInt(enemy_intent_loader_detail::GetColumn(columns, headerMap, "intent_bias_guard"), 1);
        data.biasRecover = enemy_intent_loader_detail::ToInt(enemy_intent_loader_detail::GetColumn(columns, headerMap, "intent_bias_recover"), 2);
        data.thresholdHp = enemy_intent_loader_detail::ToFloat(enemy_intent_loader_detail::GetColumn(columns, headerMap, "intent_threshold_hp"), 0.0f);

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
