#pragma once

#include <string>

namespace battle
{
// 전투에서 사용할 스킬의 정의를 담는다.
struct SkillDefinition
{
    std::string name;        // 스킬 이름
    std::string description; // 설명 텍스트
    int mpCost = 0;          // 사용 MP 비용
    int attackBonus = 0;     // 추가 공격력 보정
    bool grantsGuard = false;// 사용 시 방어 자세 부여 여부
};

// 전투 아이템 메뉴에 표시할 아이템 정보를 담는다.
struct ItemDefinition
{
    std::string name;        // 아이템 이름
    std::string description; // 설명 텍스트
    int count = 0;           // 보유 수량
};

// d20 판정 한 번의 결과를 담는다.
struct D20Check
{
    int roll = 0;            // 주사위 눈 (1~20)
    int modifier = 0;        // 스탯에서 계산된 수정치
    int situationalBonus = 0;// 상황 보너스
    int total = 0;           // 최종 합계
    int target = 0;          // 성공 기준값
    bool success = false;    // 성공 여부
};

// 전투 중 적에게 부여된 상태이상의 남은 턴 수를 추적한다.
struct EnemyStatusState
{
    int burnTurns = 0;    // 화상 지속 턴
    int wetTurns = 0;     // 습기 지속 턴
    int bindTurns = 0;    // 속박 지속 턴
    int staggerTurns = 0; // 경직 지속 턴
};
}
