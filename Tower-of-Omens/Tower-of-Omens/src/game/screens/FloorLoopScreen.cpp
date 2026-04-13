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

// 층 진행 선택지에 사용할 무작위 정수 값을 만든다.
int RandomPercent()
{
    static std::random_device seed;
    static std::mt19937 generator(seed());
    static std::uniform_int_distribution<int> distribution(1, 100);
    return distribution(generator);
}

// 이번 층에서 표시할 길 목록을 만든다.
std::vector<RouteOption> BuildRouteOptions(int floor)
{
    std::vector<RouteOption> options;
    const int depth = std::max(0, floor - 2);
    const int secondNormalChance = std::max(30, 75 - depth * 5);
    const int eliteChance = std::min(70, 18 + depth * 6);
    const int eventChance = std::min(55, 14 + depth * 4);

    options.push_back({PathChoice::Normal, "일반 전투", "가장 흔한 경로다. 무난한 적과 보상을 만날 가능성이 높다."});

    if (RandomPercent() <= secondNormalChance)
    {
        options.push_back({PathChoice::Normal, "일반 전투", "비교적 평이한 통로다. 큰 변수 없이 전투를 맞이할 가능성이 높다."});
    }

    if (RandomPercent() <= eliteChance)
    {
        options.push_back({PathChoice::Dangerous, "엘리트 전투", "거친 숨결과 묵직한 발소리가 울린다. 위험하지만 큰 보상이 기다린다."});
    }

    if (RandomPercent() <= eventChance)
    {
        options.push_back({PathChoice::Unknown, "이벤트 전투", "기묘한 마력이 새어 나온다. 예측하기 어려운 전개가 기다릴지 모른다."});
    }

    if (options.size() == 1)
    {
        options.push_back({PathChoice::Normal, "일반 전투", "안정적인 발걸음으로 지나갈 수 있는 평범한 갈림길이다."});
    }

    if (options.size() > 3)
    {
        options.resize(3);
    }

    return options;
}

// 길 목록 설명을 화면 출력용 문자열로 합친다.
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
    body << "일반 전투는 자주 등장하고, 엘리트와 이벤트는 확률적으로만 모습을 드러낸다.\n";
    body << "층이 높아질수록 위험한 길이 열릴 가능성이 점점 커진다.\n";
    return body.str();
}
}

// 층 진행 화면을 표시하고 다음 이동 방향을 결정한다.
FloorLoopResult FloorLoopScreen::Run(const Player& player, const ConsoleRenderer& renderer, const MenuInput& input) const
{
    if (player.floor >= 10)
    {
        return {GameState::Battle, std::nullopt};
    }

    if (player.floor == 1)
    {
        return {GameState::FloorLoop, PathChoice::Normal};
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
            return {GameState::FloorLoop, routeOptions[action.index].path};
        }

        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
        }
    }
}
