#pragma once

// Top-level game states.
enum class GameState
{
    Init,
    Title,
    JobSelect,
    FloorSelect,
    Battle,
    Event,
    Reward,
    Prep,
    Boss,
    GameOver,
    Clear,
    Exit
};

// Playable classes.
enum class JobClass
{
    Warrior,
    Mage
};

// Route choices offered on each floor.
enum class PathChoice
{
    Normal,
    Safe,
    Dangerous,
    Unknown
};

// Encounter categories.
enum class BattleType
{
    Normal,
    Elite,
    Event,
    Boss
};

// Results returned by the battle screen.
enum class BattleResult
{
    Victory,
    Defeat,
    Escape
};