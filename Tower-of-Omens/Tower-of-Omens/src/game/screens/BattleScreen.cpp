#include "game/screens/BattleScreen.h"
#include "game/battle/BattleCalculations.h"
#include "game/battle/BattleStatus.h"
#include "game/battle/BattleTypes.h"
#include "game/battle/BattleUi.h"
#include "game/ConsumableData.h"
#include "game/CsvUtils.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
using battle::BaseAttackDifficulty;
using battle::ComputeEnemyDamage;
using battle::ComputePlayerDamage;
using battle::D20Check;
using battle::EnemyStatusPattern;
using battle::EnemyStatusState;
using battle::FormatD20Check;
using battle::ItemDefinition;
using battle::MakeD20Check;
using battle::PlayerAccuracyStat;
using battle::RandomPercent;
using battle::RecoveryAmountFromSpirit;
using battle::RollDie;
using battle::SkillDefinition;
using battle::StatModifier;
using battle::StatusBurnDamage;
using battle::TryApplyEnemyPatterns;
using battle::ApplyEnemyStatus;
using battle::ApplyPlayerStatus;
using battle::BuildItemList;
using battle::BuildSkillList;
using battle::ComposeBattleBody;
using battle::ComposeBattleTitle;
using battle::ComposeItemMenuBody;
using battle::ComposePlayerStatusText;
using battle::ComposeSkillMenuBody;
using battle::DecayEnemyStatuses;
using battle::DecayPlayerStatuses;
using battle::HasAnyStatus;
using battle::RollEnemyIntent;

void PushBattleLog(std::vector<std::string>& logs, const std::string& line);

void PushBattleLog(std::vector<std::string>& logs, const std::string& line)
{
    logs.push_back(line);

    const std::size_t maxLogCount = 5;
    if (logs.size() > maxLogCount)
    {
        logs.erase(logs.begin());
    }
}
}

BattleResult BattleScreen::Run(
    Player& player,
    const Enemy& enemy,
    BattleType battleType,
    const std::unordered_map<int, EnemyIntentData>& intentMap,
    const ConsoleRenderer& renderer,
    const MenuInput& input) const
{
    const std::vector<std::string> options = {"공격", "스킬", "아이템", "방어", "도주"};
    int selected = 0;
    int enemyHp = enemy.hp;
    int turnCount = 1;
    std::vector<std::string> battleLogs;
    EnemyIntent pendingEnemyIntent = RollEnemyIntent(enemy, intentMap, enemyHp, battleType);
    EnemyStatusState enemyStatus;

    if (battleType == BattleType::Boss)
    {
        PushBattleLog(battleLogs, "10층 최상부에서 심연의 징조가 모습을 드러냈다.");
        PushBattleLog(battleLogs, "물러설 곳은 없다. 이 전투의 승패가 탐험의 끝을 결정한다.");
    }
    else
    {
        PushBattleLog(battleLogs, "전투가 시작되었다.");
    }

    if (HasAnyStatus(player))
    {
        PushBattleLog(battleLogs, "현재 상태이상: " + ComposePlayerStatusText(player));
    }

    for (;;)
    {
        if (player.staggerTurns > 0)
        {
            PushBattleLog(battleLogs, "경직으로 인해 이번 턴 행동할 수 없다.");
            player.staggerTurns = 0;

            if (pendingEnemyIntent == EnemyIntent::Recover)
            {
                const int recoverAmount = (battleType == BattleType::Boss) ? 18 : 12;
                const int previousHp = enemyHp;
                enemyHp = std::min(enemy.hp, enemyHp + recoverAmount);
                PushBattleLog(battleLogs, enemy.name + "이(가) 몸을 추슬러 HP를 " + std::to_string(enemyHp - previousHp) + " 회복했다.");
            }
            else if (pendingEnemyIntent == EnemyIntent::Guard)
            {
                PushBattleLog(battleLogs, enemy.name + "이(가) 방어 자세를 취했다. 다음 턴에는 피해가 줄어들 수 있다.");
            }
            else if (enemyStatus.staggerTurns > 0)
            {
                PushBattleLog(battleLogs, enemy.name + "도 경직으로 행동하지 못했다.");
                enemyStatus.staggerTurns = 0;
            }
            else
            {
                const int defensePenalty = (player.wetTurns > 0) ? 4 : 0;
                const int defenseValue = std::max(0, player.def - defensePenalty);
                const int target = 11;
                const D20Check enemyCheck = MakeD20Check(player.agility, target);
                if (!enemyCheck.success)
                {
                    PushBattleLog(battleLogs, "적의 공격이 빗나갔다. " + FormatD20Check(enemyCheck));
                }
                else
                {
                    int enemyDamage = ComputeEnemyDamage(enemy, defenseValue);
                    if (player.job == JobClass::Warrior)
                    {
                        enemyDamage = std::max(1, enemyDamage - 2);
                    }
                    player.hp = std::max(0, player.hp - enemyDamage);
                    PushBattleLog(battleLogs, "적의 공격 적중: " + FormatD20Check(enemyCheck));
                    PushBattleLog(battleLogs, "적의 반격으로 " + std::to_string(enemyDamage) + "의 피해를 입었다.");
                }
            }

            TryApplyEnemyPatterns(
                player,
                enemy,
                battleType,
                enemyHp,
                turnCount,
                [&battleLogs](const std::string& line)
                {
                    PushBattleLog(battleLogs, line);
                });

            if (player.burnTurns > 0)
            {
                const int burnDamage = StatusBurnDamage(battleType);
                player.hp = std::max(0, player.hp - burnDamage);
                PushBattleLog(battleLogs, "화상으로 " + std::to_string(burnDamage) + "의 피해를 입었다.");
            }
            if (enemyStatus.burnTurns > 0 && enemyHp > 0)
            {
                const int burnDamage = StatusBurnDamage(battleType);
                enemyHp = std::max(0, enemyHp - burnDamage);
                PushBattleLog(battleLogs, enemy.name + "이(가) 화상으로 " + std::to_string(burnDamage) + "의 피해를 입었다.");
            }

            DecayPlayerStatuses(player);
            DecayEnemyStatuses(enemyStatus);

            if (player.hp <= 0)
            {
                PushBattleLog(battleLogs, "플레이어가 쓰러졌다.");
                return BattleResult::Defeat;
            }
            if (enemyHp <= 0)
            {
                PushBattleLog(battleLogs, enemy.name + "을(를) 쓰러뜨렸다.");
                return BattleResult::Victory;
            }

            pendingEnemyIntent = RollEnemyIntent(enemy, intentMap, enemyHp, battleType);
            ++turnCount;
            continue;
        }

        renderer.Present(renderer.ComposeMenuFrame(
            ComposeBattleTitle(player, "전투"),
            ComposeBattleBody(player, enemy, enemyHp, battleType, pendingEnemyIntent, enemyStatus, selected, battleLogs),
            options,
            selected));

        const MenuAction action = input.ReadMenuSelection(selected, static_cast<int>(options.size()));
        if (action.type == MenuResultType::Move)
        {
            selected = action.index;
            continue;
        }

        if (action.type == MenuResultType::Cancel)
        {
            if (battleType == BattleType::Boss)
            {
                PushBattleLog(battleLogs, "보스전에서는 도망칠 수 없다.");
                continue;
            }
            if (player.bindTurns > 0)
            {
                PushBattleLog(battleLogs, "속박 상태라 도주할 수 없다.");
                continue;
            }
            const D20Check fleeCheck = MakeD20Check(player.agility, 11);
            if (fleeCheck.success)
            {
                PushBattleLog(battleLogs, "도주 성공: " + FormatD20Check(fleeCheck));
                return BattleResult::Escape;
            }

            PushBattleLog(battleLogs, "도주 실패: " + FormatD20Check(fleeCheck));
            continue;
        }

        if (action.index == 4)
        {
            if (battleType == BattleType::Boss)
            {
                PushBattleLog(battleLogs, "심연의 징조가 퇴로를 막았다.");
                continue;
            }
            if (player.bindTurns > 0)
            {
                PushBattleLog(battleLogs, "속박 상태라 도주할 수 없다.");
                continue;
            }

            const D20Check fleeCheck = MakeD20Check(player.agility, 11);
            if (fleeCheck.success)
            {
                PushBattleLog(battleLogs, "도주 성공: " + FormatD20Check(fleeCheck));
                return BattleResult::Escape;
            }

            PushBattleLog(battleLogs, "도주 실패: " + FormatD20Check(fleeCheck));
            continue;
        }

        bool guarded = false;
        bool enemyGuarded = (pendingEnemyIntent == EnemyIntent::Guard);
        int playerDamage = 0;
        bool performedAction = false;

        switch (action.index)
        {
        case 0:
        {
            performedAction = true;
            const int target = BaseAttackDifficulty(battleType) + (enemyGuarded ? 2 : 0);
            const D20Check hitCheck = MakeD20Check(PlayerAccuracyStat(player), target);
            if (!hitCheck.success)
            {
                player.nextAttackMultiplier = 1;
                PushBattleLog(battleLogs, "공격이 빗나갔다. " + FormatD20Check(hitCheck));
                break;
            }

            const int enemyDefense = std::max(0, (enemyGuarded ? (enemy.atk / 2) : std::max(1, enemy.atk / 3)) - (enemyStatus.wetTurns > 0 ? 4 : 0));
            playerDamage = ComputePlayerDamage(player, enemyDefense);
            playerDamage *= std::max(1, player.nextAttackMultiplier);
            player.nextAttackMultiplier = 1;
            enemyHp = std::max(0, enemyHp - playerDamage);
            PushBattleLog(battleLogs, "공격 적중: " + FormatD20Check(hitCheck));
            PushBattleLog(battleLogs, "공격이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.");
            break;
        }

        case 1:
        {
            const std::vector<SkillDefinition> skills = BuildSkillList(player.job, player.level);
            int skillSelected = 0;

            for (;;)
            {
                std::vector<std::string> skillOptions;
                for (const SkillDefinition& skill : skills)
                {
                    skillOptions.push_back(skill.name);
                }

                renderer.Present(renderer.ComposeMenuFrame(
                    ComposeBattleTitle(player, "스킬 선택"),
                    ComposeSkillMenuBody(player, skills[skillSelected]),
                    skillOptions,
                    skillSelected));

                const MenuAction skillAction = input.ReadMenuSelection(skillSelected, static_cast<int>(skillOptions.size()));
                if (skillAction.type == MenuResultType::Move)
                {
                    skillSelected = skillAction.index;
                    continue;
                }

                if (skillAction.type == MenuResultType::Cancel)
                {
                    break;
                }

                const SkillDefinition& skill = skills[skillAction.index];
                performedAction = true;

                if (player.mp < skill.mpCost)
                {
                    PushBattleLog(battleLogs, "MP가 부족해 " + skill.name + "을(를) 사용할 수 없다.");
                    break;
                }

                player.mp -= skill.mpCost;
                guarded = skill.grantsGuard;
                const int skillTarget = BaseAttackDifficulty(battleType) + 1 + (enemyGuarded ? 2 : 0);
                const D20Check skillCheck = MakeD20Check(
                    (player.job == JobClass::Warrior) ? player.strength : player.intelligence,
                    skillTarget,
                    (player.job == JobClass::Mage) ? 1 : 0);
                if (!skillCheck.success)
                {
                    player.nextAttackMultiplier = 1;
                    PushBattleLog(battleLogs, skill.name + " 실패: " + FormatD20Check(skillCheck));
                    break;
                }

                const int enemyDefense = std::max(0, (enemyGuarded ? (enemy.atk / 2) : std::max(0, enemy.atk / 6)) - (enemyStatus.wetTurns > 0 ? 4 : 0));
                playerDamage = ComputePlayerDamage(player, enemyDefense, skill.attackBonus);
                playerDamage *= std::max(1, player.nextAttackMultiplier);
                player.nextAttackMultiplier = 1;
                enemyHp = std::max(0, enemyHp - playerDamage);
                PushBattleLog(battleLogs, skill.name + " 성공: " + FormatD20Check(skillCheck));
                PushBattleLog(battleLogs, skill.name + "이 적중해 " + std::to_string(playerDamage) + "의 피해를 주었다.");
                if (skill.name == "강철 태세")
                {
                    const D20Check wetCheck = MakeD20Check(player.strength, 11);
                    if (wetCheck.success)
                    {
                        ApplyEnemyStatus(enemyStatus, "습기", 2);
                        PushBattleLog(battleLogs, "습기 부여 성공: " + FormatD20Check(wetCheck));
                    }
                }
                else if (skill.name == "파쇄 돌격")
                {
                    const D20Check staggerCheck = MakeD20Check(player.strength, 12);
                    if (staggerCheck.success)
                    {
                        ApplyEnemyStatus(enemyStatus, "경직", 1);
                        PushBattleLog(battleLogs, "경직 부여 성공: " + FormatD20Check(staggerCheck));
                    }
                }
                else if (skill.name == "마력 폭발")
                {
                    const D20Check burnCheck = MakeD20Check(player.intelligence, 11);
                    if (burnCheck.success)
                    {
                        ApplyEnemyStatus(enemyStatus, "화상", 3);
                        PushBattleLog(battleLogs, "화상 부여 성공: " + FormatD20Check(burnCheck));
                    }
                }
                else if (skill.name == "운석 낙하")
                {
                    const D20Check bindCheck = MakeD20Check(player.intelligence, 12);
                    if (bindCheck.success)
                    {
                        ApplyEnemyStatus(enemyStatus, "속박", 2);
                        PushBattleLog(battleLogs, "속박 부여 성공: " + FormatD20Check(bindCheck));
                    }
                }
                if (skill.grantsGuard)
                {
                    PushBattleLog(battleLogs, "스킬의 여파로 자세를 가다듬고 방어를 강화했다.");
                }
                break;
            }

            if (!performedAction)
            {
                continue;
            }
            break;
        }

        case 2:
        {
            const std::vector<ItemDefinition> items = BuildItemList(player);
            if (items.empty())
            {
                PushBattleLog(battleLogs, "전투 중 사용할 수 있는 아이템이 없다.");
                continue;
            }

            int itemSelected = 0;

            for (;;)
            {
                std::vector<std::string> itemOptions;
                for (const ItemDefinition& item : items)
                {
                    itemOptions.push_back(item.name);
                }

                renderer.Present(renderer.ComposeMenuFrame(
                    ComposeBattleTitle(player, "아이템 선택"),
                    ComposeItemMenuBody(player, items[itemSelected]),
                    itemOptions,
                    itemSelected));

                const MenuAction itemAction = input.ReadMenuSelection(itemSelected, static_cast<int>(itemOptions.size()));
                if (itemAction.type == MenuResultType::Move)
                {
                    itemSelected = itemAction.index;
                    continue;
                }

                if (itemAction.type == MenuResultType::Cancel)
                {
                    break;
                }

                const std::vector<ConsumableInfo> ownedConsumables = BuildOwnedConsumables(player);
                const ConsumableInfo& chosenConsumable = ownedConsumables[itemAction.index];
                performedAction = true;

                std::string itemSummary;
                if (!ApplyConsumableEffect(player, chosenConsumable, true, itemSummary))
                {
                    PushBattleLog(battleLogs, itemSummary);
                    break;
                }

                ConsumeConsumable(player, chosenConsumable.id, 1);
                PushBattleLog(battleLogs, itemSummary);
                break;
            }

            if (!performedAction)
            {
                continue;
            }
            break;
        }

        case 3:
            performedAction = true;
            guarded = true;
            PushBattleLog(battleLogs, "방어 자세를 취했다.");
            break;

        default:
            break;
        }

        if (enemyGuarded && enemyHp > 0)
        {
            PushBattleLog(battleLogs, enemy.name + "이(가) 충격을 줄일 자세를 갖추고 있었다.");
        }

        if (enemyHp <= 0)
        {
            const D20Check recoveryCheck = MakeD20Check(player.spirit, 11);
            if (recoveryCheck.success)
            {
                const int hpRecovery = std::min(player.maxHp - player.hp, RecoveryAmountFromSpirit(player, true));
                const int mpRecovery = std::min(player.maxMp - player.mp, RecoveryAmountFromSpirit(player, false));
                player.hp += std::max(0, hpRecovery);
                player.mp += std::max(0, mpRecovery);
                PushBattleLog(battleLogs, "자연 회복 성공: " + FormatD20Check(recoveryCheck));
                if (hpRecovery > 0 || mpRecovery > 0)
                {
                    PushBattleLog(battleLogs, "전투 후 HP " + std::to_string(hpRecovery) + ", MP " + std::to_string(mpRecovery) + " 회복.");
                }
                if (HasAnyStatus(player))
                {
                    player.burnTurns = std::max(0, player.burnTurns - 1);
                    player.wetTurns = std::max(0, player.wetTurns - 1);
                    player.bindTurns = std::max(0, player.bindTurns - 1);
                    player.staggerTurns = 0;
                    PushBattleLog(battleLogs, "자연 회복으로 상태이상이 다소 완화되었다.");
                }
            }
            else
            {
                PushBattleLog(battleLogs, "자연 회복 실패: " + FormatD20Check(recoveryCheck));
            }

            PushBattleLog(battleLogs, enemy.name + "을(를) 쓰러뜨렸다.");
            renderer.Present(renderer.ComposeMenuFrame(
                ComposeBattleTitle(player, "전투 승리"),
                ComposeBattleBody(player, enemy, enemyHp, battleType, pendingEnemyIntent, enemyStatus, selected, battleLogs),
                options,
                selected));
            return BattleResult::Victory;
        }

        if (pendingEnemyIntent == EnemyIntent::Recover)
        {
            const int recoverAmount = (battleType == BattleType::Boss) ? 18 : 12;
            const int previousHp = enemyHp;
            enemyHp = std::min(enemy.hp, enemyHp + recoverAmount);
            PushBattleLog(battleLogs, enemy.name + "이(가) 몸을 추슬러 HP를 " + std::to_string(enemyHp - previousHp) + " 회복했다.");
        }
        else if (pendingEnemyIntent == EnemyIntent::Guard)
        {
            PushBattleLog(battleLogs, enemy.name + "이(가) 방어 자세를 취했다. 다음 턴에는 피해가 줄어들 수 있다.");
        }
        else
        {
            const int defensePenalty = (player.wetTurns > 0) ? 4 : 0;
            const int defenseValue = std::max(0, (guarded ? player.def + 6 : player.def) - defensePenalty);
            const int target = guarded ? 13 : 11;
            const D20Check enemyCheck = MakeD20Check(player.agility, target);
            if (!enemyCheck.success)
            {
                PushBattleLog(battleLogs, "적의 공격이 빗나갔다. " + FormatD20Check(enemyCheck));
            }
            else
            {
                int enemyDamage = ComputeEnemyDamage(enemy, defenseValue);

                if (player.job == JobClass::Warrior)
                {
                    enemyDamage = std::max(1, enemyDamage - 2);
                }

                player.hp = std::max(0, player.hp - enemyDamage);

                if (guarded)
                {
                    PushBattleLog(battleLogs, "적의 공격 적중: " + FormatD20Check(enemyCheck));
                    PushBattleLog(battleLogs, "적의 반격을 받아 " + std::to_string(enemyDamage) + "의 피해를 입었지만 방어로 충격을 줄였다.");
                }
                else
                {
                    PushBattleLog(battleLogs, "적의 공격 적중: " + FormatD20Check(enemyCheck));
                    PushBattleLog(battleLogs, "적의 반격으로 " + std::to_string(enemyDamage) + "의 피해를 입었다.");
                }
            }
        }

        TryApplyEnemyPatterns(
            player,
            enemy,
            battleType,
            enemyHp,
            turnCount,
            [&battleLogs](const std::string& line)
            {
                PushBattleLog(battleLogs, line);
            });

        if (player.burnTurns > 0)
        {
            const int burnDamage = StatusBurnDamage(battleType);
            player.hp = std::max(0, player.hp - burnDamage);
            PushBattleLog(battleLogs, "화상으로 " + std::to_string(burnDamage) + "의 피해를 입었다.");
        }
        if (enemyStatus.burnTurns > 0 && enemyHp > 0)
        {
            const int burnDamage = StatusBurnDamage(battleType);
            enemyHp = std::max(0, enemyHp - burnDamage);
            PushBattleLog(battleLogs, enemy.name + "이(가) 화상으로 " + std::to_string(burnDamage) + "의 피해를 입었다.");
        }

        DecayPlayerStatuses(player);
        DecayEnemyStatuses(enemyStatus);

        if (player.job == JobClass::Mage && performedAction)
        {
            const int recoveredMp = std::min(3, player.maxMp - player.mp);
            if (recoveredMp > 0)
            {
                player.mp += recoveredMp;
                PushBattleLog(battleLogs, "마력 순환으로 MP를 " + std::to_string(recoveredMp) + " 회복했다.");
            }
        }

        if (player.hp <= 0)
        {
            PushBattleLog(battleLogs, "플레이어가 쓰러졌다.");
            return BattleResult::Defeat;
        }

        if (enemyHp <= 0)
        {
            PushBattleLog(battleLogs, enemy.name + "을(를) 쓰러뜨렸다.");
            return BattleResult::Victory;
        }

        pendingEnemyIntent = RollEnemyIntent(enemy, intentMap, enemyHp, battleType);
        ++turnCount;
    }
}
