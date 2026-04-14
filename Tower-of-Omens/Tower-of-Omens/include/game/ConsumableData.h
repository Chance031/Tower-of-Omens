#pragma once

#include "game/Player.h"

#include <string>
#include <vector>

struct ConsumableInfo
{
    std::string id;
    std::string name;
    std::string description;
    std::string type;
    std::string jobRestriction;
    std::string effect;
    int value = 0;
    int buyPrice = 0;
    int sellPrice = 0;
};

const std::vector<ConsumableInfo>& LoadConsumableCatalog();
std::vector<ConsumableInfo> BuildOwnedConsumables(const Player& player);
int GetConsumableCount(const Player& player, const std::string& id);
void AddConsumable(Player& player, const std::string& id, int amount);
bool ConsumeConsumable(Player& player, const std::string& id, int amount);
bool ApplyConsumableEffect(Player& player, const ConsumableInfo& item, bool inBattle, std::string& summary);
