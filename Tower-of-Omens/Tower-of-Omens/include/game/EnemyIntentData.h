#pragma once

// CSV(enemy_base.csv)에서 로드한 적 행동 성향 데이터다.
struct EnemyIntentData
{
    int   enemyId      = 0;
    int   biasAttack   = 7;     // 공격 행동 가중치
    int   biasGuard    = 1;     // 수비 행동 가중치
    int   biasRecover  = 2;     // 회복 행동 가중치
    float thresholdHp  = 0.0f;  // 이 HP 비율 이하일 때 guard/recover 가중치 2배 적용
};

// 적이 이번 턴에 취할 행동 의도다.
// 효과 처리 규칙은 BattleScreen.cpp가 담당한다.
enum class EnemyIntent
{
    Attack,   // 공격
    Guard,    // 수비 (다음 플레이어 공격 피해 감소)
    Recover   // 회복 (Elite/Boss는 실제 HP 회복, Normal은 대기)
};
