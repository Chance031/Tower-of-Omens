#pragma once

#include "game/Enums.h"

#include <algorithm>
#include <string>
#include <vector>

struct ConsumableStack
{
    std::string id;
    int count = 0;
};

// 플레이어의 모든 상태를 보관하는 구조체다.
struct Player
{
    std::string name;                    // 직업 이름
    JobClass job = JobClass::Warrior;    // 선택한 직업

    // --- 진행 상태 ---
    int level = 1;       // 현재 레벨
    int statPoints = 0;  // 미배분 스탯 포인트
    int floor = 1;       // 현재 층
    int gold = 0;        // 보유 골드

    // --- 현재 자원 ---
    int hp = 0;  // 현재 HP
    int mp = 0;  // 현재 MP

    // --- 기본 스탯 (레벨업/이벤트로 증가) ---
    int strength = 0;     // STR: 전사 ATK·HP 기여, 스킬 판정
    int agility = 0;      // AGI: 전사 ATK·DEF 기여, 명중/도주 판정
    int intelligence = 0; // INT: 마법사 ATK·MP 기여, 명중 판정
    int spirit = 0;       // MND: HP·MP·DEF 기여, 전투 후 회복

    // --- 파생 스탯 (RefreshDerivedStats로 재계산) ---
    int atk = 0;    // 최종 공격력
    int def = 0;    // 최종 방어력
    int maxHp = 0;  // 최대 HP
    int maxMp = 0;  // 최대 MP

    // --- 보너스 (장비·유물·이벤트 누적) ---
    int bonusAttackPower = 0; // ATK 추가 보너스
    int bonusDefense = 0;     // DEF 추가 보너스
    int bonusMaxHp = 0;       // 최대 HP 추가 보너스
    int bonusMaxMp = 0;       // 최대 MP 추가 보너스

    // --- 상태이상 (남은 턴 수) ---
    int burnTurns = 0;    // 화상: 매 턴 HP 감소
    int wetTurns = 0;     // 습기: DEF 감소
    int bindTurns = 0;    // 속박: 도주 불가
    int staggerTurns = 0; // 경직: 행동 불가

    // --- 전투 임시 상태 ---
    int nextAttackMultiplier = 1; // 다음 공격의 배율 (아이템 사용 시 설정)

    // --- 장비 (장착 슬롯 + 예비 슬롯) ---
    std::string weaponName;
    int weaponAtkBonus = 0;
    std::string armorName;
    int armorDefBonus = 0;
    std::string bagWeaponName;
    int bagWeaponAtkBonus = 0;
    std::string bagArmorName;
    int bagArmorDefBonus = 0;

    // --- 소모품 및 유물 ---
    std::vector<ConsumableStack> consumables; // 보유 소모품 스택
    std::vector<std::string> relicNames;      // 획득한 유물 이름 목록
};

// 기본 스탯과 장비 보너스를 기반으로 ATK/DEF/maxHp/maxMp를 재계산한다.
// refillResources가 true면 HP/MP를 최대치로 채운다.
inline void RefreshDerivedStats(Player& player, bool refillResources = false)
{
    const int previousMaxHp = std::max(1, player.maxHp);
    const int previousMaxMp = std::max(0, player.maxMp);
    const int preservedHp = player.hp;
    const int preservedMp = player.mp;

    const int baseHp = 40 + player.strength * 4 + player.spirit * 2;
    const int baseMp = 10 + player.intelligence * 3 + player.spirit * 2;
    const int baseAttack = (player.job == JobClass::Warrior)
        ? (4 + player.strength + (player.agility / 2))
        : (4 + player.intelligence + (player.spirit / 2));
    const int baseDefense = ((player.job == JobClass::Warrior) ? 4 : 1) + (player.agility / 2) + (player.spirit / 4);

    player.maxHp = std::max(1, baseHp + player.bonusMaxHp);
    player.maxMp = std::max(0, baseMp + player.bonusMaxMp);
    player.atk = std::max(1, baseAttack + player.weaponAtkBonus + player.bonusAttackPower);
    player.def = std::max(0, baseDefense + player.armorDefBonus + player.bonusDefense);

    if (refillResources)
    {
        player.hp = player.maxHp;
        player.mp = player.maxMp;
        return;
    }

    if (previousMaxHp > 0)
    {
        player.hp = std::clamp((preservedHp * player.maxHp) / previousMaxHp, 0, player.maxHp);
    }
    else
    {
        player.hp = player.maxHp;
    }

    if (previousMaxMp > 0)
    {
        player.mp = std::clamp((preservedMp * player.maxMp) / previousMaxMp, 0, player.maxMp);
    }
    else
    {
        player.mp = player.maxMp;
    }
}
