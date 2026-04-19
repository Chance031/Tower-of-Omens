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

// 게임 전체 흐름을 소유하고 상태 전환을 관리하는 최상위 클래스다.
class Game
{
public:
    Game();

    // 렌더러 초기화 등 실행 전 준비를 수행한다.
    void Initialize();

    // 게임 상태 루프를 Exit 상태가 될 때까지 반복 실행한다.
    void Run();

private:
    GameState m_state;                    // 현재 게임 상태
    Player m_player;                      // 플레이어 데이터
    ConsoleRenderer m_renderer;           // 콘솔 렌더러
    EnemyFactory m_enemyFactory;          // 적 생성 팩토리
    BattleType m_pendingBattleType;       // 다음 전투 유형 (예약)
    PathChoice m_pendingPathChoice;       // 다음 전투 경로 (예약)
    Enemy m_lastEnemy;                    // 마지막으로 처치한 적
    BattleType m_lastBattleType;          // 마지막 전투 유형
    PathChoice m_lastResolvedPathChoice;  // 마지막으로 확정된 경로

    // 직업을 받아 플레이어를 초기 상태로 리셋한다.
    void StartRun(JobClass job);
    // 직업 클래스에 해당하는 한국어 이름을 반환한다.
    std::string JobName(JobClass job) const;
    // 경로 선택에 따라 전투 유형을 결정한다.
    BattleType DetermineBattleType(PathChoice path) const;
    // 레벨업 스탯 성장을 적용하고 결과 텍스트를 반환한다.
    std::string ApplyJobGrowth();
    // 전투 화면을 실행하고 결과에 따라 다음 상태를 반환한다.
    GameState RunEncounterState(
        BattleType battleType,
        BattleScreen& battleScreen,
        MessageScreen& messageScreen,
        const MenuInput& input);
    // 게임 오버 화면을 표시하고 재시작 또는 타이틀 상태를 반환한다.
    GameState RunGameOverScreen(const MenuInput& input);
    // 전투 보상을 처리하고 결과 요약을 반환한다.
    std::string ResolveBattleReward(const Enemy& enemy, BattleType battleType, PathChoice path, const MenuInput& input);
};
