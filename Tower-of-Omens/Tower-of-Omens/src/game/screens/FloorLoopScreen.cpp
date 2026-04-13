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

std::vector<RouteOption> BuildRouteOptions(int floor)
{
    std::vector<RouteOption> options;
    const int depth = std::max(0, floor - 2);
    const int eliteChance = std::min(70, 20 + depth * 6);
    const int eventChance = std::min(55, 16 + depth * 4);

    options.push_back({PathChoice::Normal, "일반 전투", "가장 흔한 경로다. 무난한 적과 보상을 만날 가능성이 높다."});

    if (RandomPercent() <= eliteChance)
    {
        options.push_back({PathChoice::Dangerous, "엘리트 전투", "위험하지만 더 강한 적과 큰 보상이 기다린다."});
    }

    if (RandomPercent() <= eventChance)
    {
        options.push_back({PathChoice::Unknown, "이벤트", "전투가 아닌 특별한 사건이 기다리는 수상한 길이다."});
    }

    if (options.size() == 1)
    {
        options.push_back({PathChoice::Normal, "일반 전투", "갈 수 있는 길이 부족할 때 만나는 평범한 전투 경로다."});
    }

    if (options.size() > 3)
    {
        options.resize(3);
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
    body << "일반 전투는 가장 자주 등장한다.\n";
    body << "층이 높아질수록 엘리트 전투와 이벤트가 나타날 가능성이 커진다.\n";
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