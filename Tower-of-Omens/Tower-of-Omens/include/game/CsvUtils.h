#pragma once

#define NOMINMAX
#include <Windows.h>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace csv
{
// 문자열 앞뒤의 공백, 탭, 캐리지 리턴을 제거한다.
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

// CSV 한 줄을 쉼표 구분자로 파싱해 컬럼 배열로 반환한다. 따옴표 처리를 지원한다.
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

// 문자열을 int로 변환한다. 변환 실패 시 fallback을 반환한다.
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

inline double ToDouble(const std::string& value, double fallback = 0.0)
{
    try
    {
        return std::stod(Trim(value));
    }
    catch (...)
    {
        return fallback;
    }
}

// UTF-8 문자열을 Windows 콘솔 인코딩(CP949)으로 변환한다.
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

// 파일을 읽어 문자열로 반환한다. UTF-8 BOM 제거 및 콘솔 인코딩 변환을 수행한다.
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

// 실행 위치에 따라 CSV 파일의 실제 경로를 찾아 반환한다. 찾지 못하면 빈 문자열.
inline std::string ResolveCsvPath(const std::string& fileName)
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

// 헤더 컬럼 배열을 {이름 → 인덱스} 맵으로 변환한다.
inline std::unordered_map<std::string, std::size_t> BuildHeaderMap(const std::vector<std::string>& headers)
{
    std::unordered_map<std::string, std::size_t> map;
    for (std::size_t i = 0; i < headers.size(); ++i)
    {
        map[Trim(headers[i])] = i;
    }
    return map;
}

// 헤더 맵에서 key에 해당하는 컬럼 값을 찾아 반환한다. 없으면 빈 문자열.
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
