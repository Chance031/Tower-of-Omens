#include "game/screens/FloorLoopScreen.h"

#include "engine/platform/MenuInput.h"

#include <algorithm>
#include <random>
#include <sstream>
#include <vector>

namespace
{
struct RouteOption
{
    PathChoice path = PathChoice::Normal;
    std::string name;
    std::string description;
};

int RandomPercent()
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    static std::uniform_int_distribution<int> distribution(1, 100);
    return distribution(generator);
}

int RandomIndex(int minValue, int maxValue)
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(generator);
}

RouteOption MakeNormalRoute()
{
    return {PathChoice::Normal, "일반 전투", "가장 흔한 경로다. 무난한 적과 보상을 만날 가능성이 높다."};
}

std::vector<RouteOption> BuildRouteOptions(int floor)
{
    std::vector<RouteOption> options = {MakeNormalRoute(), MakeNormalRoute(), MakeNormalRoute()};

    const int depth = std::max(0, floor - 1);
    const int eliteChance = std::min(45, 12 + depth * 5);
    const int eventChance = std::min(35, 10 + depth * 4);
    const int specialChance = std::min(70, eliteChance + eventChance);

    if (RandomPercent() > specialChance)
    {
        return options;
    }

    const int replaceIndex = RandomIndex(0, static_cast<int>(options.size()) - 1);
    const int roll = RandomPercent();
    if (roll <= eliteChance)
    {
        options[replaceIndex] = {PathChoice::Dangerous, "엘리트 전투", "위험하지만 더 강한 적과 큰 보상이 기다린다."};
    }
    else
    {
        options[replaceIndex] = {PathChoice::Unknown, "이벤트", "전투가 아닌 특별한 사건이 기다리는 수상한 길이다."};
    }

    return options;
}

std::string ComposeRouteBody(const Player& player, const std::vector<RouteOption>& routeOptions, int selected)
{
    std::ostringstream body;
    body << "[탐험 정보]\n";
    body << player.name << " | 현재 층 " << player.floor << " | 레벨 " << player.level << '\n';
    body << "HP " << player.hp << '/' << player.maxHp << " | MP " << player.mp << '/' << player.maxMp << " | Gold " << player.gold << "\n\n";
    body << "[선택한 길]\n";
    body << routeOptions[selected].name << '\n';
    body << routeOptions[selected].description << "\n\n";
    body << "[안내]\n";
    body << "기본적으로 일반 전투 3개가 나타난다.\n";
    body << "확률적으로 한 칸만 엘리트 전투 또는 이벤트로 바뀌며, 일반 전투는 반드시 하나 이상 남는다.\n";
    return body.str();
}
}

FloorLoopResult FloorLoopScreen::Run(const Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const
{
    if (player.floor >= 10)
    {
        return {GameState::Boss, std::nullopt};
    }

    const std::vector<RouteOption> routeOptions = BuildRouteOptions(player.floor);
    std::vector<std::string> optionNames;
    for (const RouteOption& option : routeOptions)
    {
        optionNames.push_back(option.name);
    }

    int selected = 0;

    for (;;)
    {
        renderer.Present(renderer.ComposeMenuFrame("길 선택", ComposeRouteBody(player, routeOptions, selected), optionNames, selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(optionNames.size()));
        if (action.type == MenuResultType::Confirm)
        {
            return {GameState::FloorSelect, routeOptions[action.index].path};
        }

        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
        }
    }
}
