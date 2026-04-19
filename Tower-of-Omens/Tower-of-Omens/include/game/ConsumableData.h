#pragma once

#include "game/Player.h"

#include <string>
#include <vector>

// CSV에서 불러온 소모품 한 종류의 정의를 담는다.
struct ConsumableInfo
{
    std::string id;             // 고유 식별자 (예: "201")
    std::string name;           // 표시 이름
    std::string description;    // 설명 텍스트
    std::string type;           // 아이템 유형
    std::string jobRestriction; // 직업 제한 (비어있으면 제한 없음)
    std::string effect;         // 효과 키 (예: "hp", "mp", "maxHp")
    int value = 0;              // 효과 수치
    int buyPrice = 0;           // 구매 가격
    int sellPrice = 0;          // 판매 가격
};

// CSV 파일에서 전체 소모품 목록을 한 번 로드해 반환한다.
const std::vector<ConsumableInfo>& LoadConsumableCatalog();

// 플레이어가 보유한 소모품 목록을 반환한다.
std::vector<ConsumableInfo> BuildOwnedConsumables(const Player& player);

// 플레이어가 보유한 특정 소모품의 개수를 반환한다.
int GetConsumableCount(const Player& player, const std::string& id);

// 플레이어의 소모품 스택에 수량을 추가한다. 음수면 감소하며 0 미만으로는 내려가지 않는다.
void AddConsumable(Player& player, const std::string& id, int amount);

// 플레이어의 소모품을 지정한 수량만큼 소모한다. 성공 여부를 반환한다.
bool ConsumeConsumable(Player& player, const std::string& id, int amount);

// 소모품 효과를 플레이어에게 적용한다. 결과 요약을 summary에 채운다.
bool ApplyConsumableEffect(Player& player, const ConsumableInfo& item, bool inBattle, std::string& summary);
