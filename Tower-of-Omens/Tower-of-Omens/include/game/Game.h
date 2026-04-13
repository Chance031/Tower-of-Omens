#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enemy.h"
#include "game/EnemyFactory.h"
#include "game/Enums.h"
#include "game/Player.h"

#include <string>

class BattleScreen;
class MessageScreen;

// Owns the top-level game flow.
class Game
{
public:
    Game();

    // Prepares the game before running.
    void Initialize();

    // Runs the state loop until exit.
    void Run();

private:
    GameState m_state;
    Player m_player;
    ConsoleRenderer m_renderer;
    EnemyFactory m_enemyFactory;
    BattleType m_pendingBattleType;
    PathChoice m_pendingPathChoice;
    Enemy m_lastEnemy;
    BattleType m_lastBattleType;
    PathChoice m_lastResolvedPathChoice;

    void StartRun(JobClass job);
    std::string JobName(JobClass job) const;
    BattleType DetermineBattleType(PathChoice path) const;
    std::string ApplyJobGrowth();
    GameState RunEncounterState(
        BattleType battleType,
        BattleScreen& battleScreen,
        MessageScreen& messageScreen,
        const MenuInput& input);
    GameState RunGameOverScreen(const MenuInput& input);
    std::string ResolveBattleReward(const Enemy& enemy, BattleType battleType, PathChoice path, const MenuInput& input);
};
