#pragma once

#include "game/EnemyIntentData.h"

#include <string>
#include <unordered_map>

// enemy_base.csv를 파싱해 enemy_id → EnemyIntentData 맵을 반환한다.
// 해당 id가 없으면 호출부에서 기본값(biasAttack=7, biasGuard=1, biasRecover=2)을 사용한다.
std::unordered_map<int, EnemyIntentData> LoadEnemyIntents(const std::string& csvPath);

// intentMap에서 enemyId에 해당하는 데이터를 찾아 반환한다.
// id가 없으면 기본값 EnemyIntentData를 반환한다.
EnemyIntentData FindIntentData(
    const std::unordered_map<int, EnemyIntentData>& intentMap,
    int enemyId);

// 가중치와 D20 롤을 기반으로 적의 이번 턴 행동 의도를 결정한다.
// hp 비율이 thresholdHp 이하일 때 guard/recover 가중치가 2배 적용된다.
EnemyIntent DecideEnemyIntent(
    const EnemyIntentData& data,
    int currentHp,
    int maxHp,
    int d20Roll);
