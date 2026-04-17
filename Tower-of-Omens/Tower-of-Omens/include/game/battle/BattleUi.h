#pragma once

#include "game/ConsumableData.h"
#include "game/Enemy.h"
#include "game/EnemyIntentLoader.h"
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
inline std::string BattleTypeName(BattleType battleType)
{
    switch (battleType)
    {
    case BattleType::Normal:
        return "일반 전투";
    case BattleType::Elite:
        return "엘리트 전투";
    case BattleType::Event:
        return "이벤트";
    case BattleType::Boss:
        return "보스 전투";
    }

    return "전투";
}

inline std::string PassiveName(JobClass job)
{
    return (job == JobClass::Warrior) ? "불굴" : "마력 순환";
}

inline std::string PassiveDescription(JobClass job)
{
    return (job == JobClass::Warrior)
        ? "받는 피해가 항상 2 감소한다."
        : "행동 후 MP를 3 회복한다.";
}

inline bool HasObservationRelic(const Player& player)
{
    return std::find(player.relicNames.begin(), player.relicNames.end(), "관찰 유물") != player.relicNames.end() ||
        std::find(player.relicNames.begin(), player.relicNames.end(), "관찰의 눈") != player.relicNames.end();
}

inline std::string EnemyIntentName(EnemyIntent intent)
{
    switch (intent)
    {
    case EnemyIntent::Attack:
        return "공격";
    case EnemyIntent::Guard:
        return "방어";
    case EnemyIntent::Recover:
        return "회복";
    }

    return "알 수 없음";
}

inline std::string EnemyIntentDescription(EnemyIntent intent)
{
    switch (intent)
    {
    case EnemyIntent::Attack:
        return "다음 턴에 플레이어를 노리고 공격을 준비 중이다.";
    case EnemyIntent::Guard:
        return "다음 턴에 몸을 웅크리고 피해를 줄일 생각이다.";
    case EnemyIntent::Recover:
        return "다음 턴에 숨을 고르며 체력을 회복하려 한다.";
    }

    return "의도를 읽을 수 없다.";
}

inline std::string MakeBar(int current, int maximum, int width, char filled, char empty)
{
    if (maximum <= 0)
    {
        return std::string(width, empty);
    }

    const int clampedCurrent = std::max(0, std::min(current, maximum));
    const int filledCount = (clampedCurrent * width) / maximum;
    return std::string(filledCount, filled) + std::string(width - filledCount, empty);
}

inline bool HasAnyStatus(const Player& player)
{
    return player.burnTurns > 0 || player.wetTurns > 0 || player.bindTurns > 0 || player.staggerTurns > 0;
}

inline bool HasAnyStatus(const EnemyStatusState& status)
{
    return status.burnTurns > 0 || status.wetTurns > 0 || status.bindTurns > 0 || status.staggerTurns > 0;
}

inline std::string ComposePlayerStatusText(const Player& player)
{
    if (!HasAnyStatus(player))
    {
        return "없음";
    }

    std::ostringstream body;
    bool first = true;
    if (player.burnTurns > 0)
    {
        body << (first ? "" : ", ") << "화상 " << player.burnTurns << "턴";
        first = false;
    }
    if (player.wetTurns > 0)
    {
        body << (first ? "" : ", ") << "습기 " << player.wetTurns << "턴";
        first = false;
    }
    if (player.bindTurns > 0)
    {
        body << (first ? "" : ", ") << "속박 " << player.bindTurns << "턴";
        first = false;
    }
    if (player.staggerTurns > 0)
    {
        body << (first ? "" : ", ") << "경직 " << player.staggerTurns << "턴";
    }
    return body.str();
}

inline std::string ComposeEnemyStatusText(const EnemyStatusState& status)
{
    if (!HasAnyStatus(status))
    {
        return "없음";
    }

    std::ostringstream body;
    bool first = true;
    if (status.burnTurns > 0)
    {
        body << (first ? "" : ", ") << "화상 " << status.burnTurns << "턴";
        first = false;
    }
    if (status.wetTurns > 0)
    {
        body << (first ? "" : ", ") << "습기 " << status.wetTurns << "턴";
        first = false;
    }
    if (status.bindTurns > 0)
    {
        body << (first ? "" : ", ") << "속박 " << status.bindTurns << "턴";
        first = false;
    }
    if (status.staggerTurns > 0)
    {
        body << (first ? "" : ", ") << "경직 " << status.staggerTurns << "턴";
    }
    return body.str();
}

inline EnemyIntent RollEnemyIntent(
    const Enemy& enemy,
    const std::unordered_map<int, EnemyIntentData>& intentMap,
    int enemyHp,
    BattleType battleType)
{
    EnemyIntentData intentData = FindIntentData(intentMap, enemy.id);

    if (battleType == BattleType::Boss)
    {
        intentData.biasGuard += 1;
        intentData.biasRecover += 1;
    }
    else if (battleType == BattleType::Elite)
    {
        intentData.biasAttack += 1;
    }

    return DecideEnemyIntent(intentData, enemyHp, enemy.hp, RollDie(1, 20));
}

inline std::vector<SkillDefinition> BuildSkillList(JobClass job, int level)
{
    std::vector<SkillDefinition> skills;

    if (job == JobClass::Warrior)
    {
        skills.push_back({"강철 태세", "MP 8 소모. 피해를 주고 이번 턴 방어를 강화한다.", 8, 5, true});
        if (level >= 5)
        {
            skills.push_back({"파쇄 돌격", "MP 14 소모. 더 큰 피해를 주고 자세를 다잡는다.", 14, 12, true});
        }
    }
    else
    {
        skills.push_back({"마력 폭발", "MP 14 소모. 강한 마법 피해를 준다.", 14, 16, false});
        if (level >= 5)
        {
            skills.push_back({"운석 낙하", "MP 20 소모. 압도적인 마법 피해를 준다.", 20, 24, false});
        }
    }

    return skills;
}

inline std::vector<ItemDefinition> BuildItemList(const Player& player)
{
    std::vector<ItemDefinition> items;
    for (const ConsumableInfo& consumable : BuildOwnedConsumables(player))
    {
        items.push_back({consumable.name, consumable.description, GetConsumableCount(player, consumable.id)});
    }
    return items;
}

inline std::string ActionDescription(JobClass job, int level, int actionIndex)
{
    switch (actionIndex)
    {
    case 0:
        return "d20 판정으로 명중을 확인한 뒤 적을 공격한다.";
    case 1:
        return (job == JobClass::Warrior)
            ? ((level >= 5) ? "사용 가능한 전투 기술을 선택한다. 강철 태세와 파쇄 돌격을 사용할 수 있다." : "사용 가능한 전투 기술을 선택한다. 현재는 강철 태세를 사용할 수 있다.")
            : ((level >= 5) ? "사용 가능한 마법 기술을 선택한다. 마력 폭발과 운석 낙하를 사용할 수 있다." : "사용 가능한 마법 기술을 선택한다. 현재는 마력 폭발을 사용할 수 있다.");
    case 2:
        return "보유 중인 소모품 목록을 열어 하나를 선택한다. 없는 아이템은 표시되지 않는다.";
    case 3:
        return "방어 자세를 취해 받는 피해를 줄이고 적의 명중을 흔든다.";
    case 4:
        return "d20 + 민첩 보정으로 전투에서 이탈을 시도한다.";
    }

    return "행동을 선택한다.";
}

inline std::string ComposeLogText(const std::vector<std::string>& logs)
{
    std::ostringstream stream;
    for (const std::string& line : logs)
    {
        stream << "- " << line << '\n';
    }

    return stream.str();
}

inline std::string ComposeStatusHeadline(const Player& player)
{
    std::ostringstream body;
    body << "[현재 상태] HP " << player.hp << '/' << player.maxHp;
    body << " | MP " << player.mp << '/' << player.maxMp;
    body << " | 회복약 " << GetConsumableCount(player, "201");
    body << " | 마나약 " << GetConsumableCount(player, "203");
    if (player.nextAttackMultiplier > 1)
    {
        body << " | 다음 공격 x" << player.nextAttackMultiplier;
    }
    body << '\n';
    return body.str();
}

inline std::string ComposeBattleTitle(const Player& player, const std::string& baseTitle)
{
    std::ostringstream title;
    title << baseTitle << " | HP " << player.hp << '/' << player.maxHp;
    title << " | MP " << player.mp << '/' << player.maxMp;
    return title.str();
}

inline std::string ComposePlayerPanel(const Player& player)
{
    std::ostringstream body;
    body << "[플레이어 패널]\n";
    body << player.name << " | 층 " << player.floor << " | Lv " << player.level << '\n';
    body << "HP [" << MakeBar(player.hp, player.maxHp, 20, '#', '.') << "] " << player.hp << '/' << player.maxHp << '\n';
    body << "MP [" << MakeBar(player.mp, player.maxMp, 20, '@', '.') << "] " << player.mp << '/' << player.maxMp << '\n';
    body << "STR " << player.strength << " | AGI " << player.agility;
    body << " | INT " << player.intelligence << " | MND " << player.spirit << '\n';
    body << "방어력 " << player.def << " | GOLD " << player.gold << '\n';
    body << "근접 보정 " << StatModifier(player.strength) << " | 명중/도주 보정 " << StatModifier(player.agility)
        << " | 마법 보정 " << StatModifier(player.intelligence) << " | 회복 보정 " << StatModifier(player.spirit) << '\n';
    body << "상태이상 " << ComposePlayerStatusText(player) << '\n';
    body << "회복약 " << GetConsumableCount(player, "201") << " | 마나약 " << GetConsumableCount(player, "203") << '\n';
    body << "패시브 " << PassiveName(player.job) << " - " << PassiveDescription(player.job) << '\n';
    return body.str();
}

inline std::string ComposeEnemyPanel(
    const Player& player,
    const Enemy& enemy,
    int enemyHp,
    BattleType battleType,
    EnemyIntent nextIntent,
    const EnemyStatusState& enemyStatus)
{
    std::ostringstream body;
    body << "[적 패널]\n";
    body << enemy.name << " | " << BattleTypeName(battleType) << '\n';
    body << "HP [" << MakeBar(enemyHp, enemy.hp, 20, '#', '.') << "] " << enemyHp << '/' << enemy.hp << '\n';
    body << "ATK " << enemy.atk << " | 보상 " << enemy.goldReward << " Gold\n";
    body << "기본 회피 난도 " << BaseAttackDifficulty(battleType) << '\n';
    body << "상태이상 " << ComposeEnemyStatusText(enemyStatus) << '\n';
    if (HasObservationRelic(player))
    {
        body << "다음 행동: " << EnemyIntentName(nextIntent) << "\n";
        body << EnemyIntentDescription(nextIntent) << '\n';
    }
    else if (battleType == BattleType::Boss)
    {
        body << "심연이 꿈틀거린다. 물러설 곳은 없다.\n";
    }
    else if (battleType == BattleType::Elite)
    {
        body << "보통 적보다 날카로운 기세가 느껴진다.\n";
    }
    else
    {
        body << "숨을 고르고 적의 움직임을 살핀다.\n";
    }

    return body.str();
}

inline std::string ComposeBattleBody(
    const Player& player,
    const Enemy& enemy,
    int enemyHp,
    BattleType battleType,
    EnemyIntent nextIntent,
    const EnemyStatusState& enemyStatus,
    int selected,
    const std::vector<std::string>& battleLogs)
{
    std::ostringstream body;
    body << ComposeStatusHeadline(player);
    body << ComposePlayerPanel(player) << '\n';
    body << "------------------------------------------------------------\n";
    body << ComposeEnemyPanel(player, enemy, enemyHp, battleType, nextIntent, enemyStatus) << '\n';
    body << "------------------------------------------------------------\n";
    body << "[현재 행동]\n";
    body << ActionDescription(player.job, player.level, selected) << "\n\n";
    body << "[전투 로그]\n";
    body << ComposeLogText(battleLogs);
    return body.str();
}

inline std::string ComposeSkillMenuBody(const Player& player, const SkillDefinition& skill)
{
    std::ostringstream body;
    body << ComposeStatusHeadline(player);
    body << "[선택한 스킬]\n";
    body << skill.name << '\n';
    body << skill.description << '\n';
    body << "필요 MP: " << skill.mpCost << "\n\n";
    body << ComposePlayerPanel(player);
    body << "ESC를 누르면 전투 메뉴로 돌아간다.\n";
    return body.str();
}

inline std::string ComposeItemMenuBody(const Player& player, const ItemDefinition& item)
{
    std::ostringstream body;
    body << ComposeStatusHeadline(player);
    body << "[선택한 아이템]\n";
    body << item.name << " | 보유 수량: " << item.count << '\n';
    body << item.description << "\n\n";
    body << ComposePlayerPanel(player);
    body << "ESC를 누르면 전투 메뉴로 돌아간다.\n";
    return body.str();
}
}
