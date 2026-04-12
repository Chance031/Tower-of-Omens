# Tower-of-Omens

C++로 만드는 콘솔 로그라이크 RPG 프로젝트

## 기술 스택

[![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white)](https://isocpp.org/)

## 사용 AI

[![Claude](https://img.shields.io/badge/Claude-D97757?style=flat&logo=anthropic&logoColor=white)](https://claude.ai/)
[![ChatGPT](https://img.shields.io/badge/ChatGPT-74aa9c?style=flat&logo=openai&logoColor=white)](https://chat.openai.com/)

---

## 설계 문서

| 문서 | 설명 |
|--------|--------|
| [플로우차트 (draw.io로 열기)](https://app.diagrams.net/?url=https://raw.githubusercontent.com/Chance031/Tower-of-Omens/main/Tower%20of%20Omens.drawio) | 게임 전체 상태 흐름, 전투 시퀀스, 아이템·보상 흐름, 상태 다이어그램 (8페이지) |

---

## 개발 일지

<!-- LOG_START -->
| 날짜 | 섹션 | 작업 내용 |
|------|------|-----------|
| 2026-04-12 | 초기 설정 | 레포 생성, README·gitignore·MIT License 추가 |
| 2026-04-12 | 초기 설정 | 플로우차트(draw.io) 및 솔루션 파일(Tower-of-Omens.slnx) 추가 |
<!-- LOG_END -->

---

## 플레이 가이드

### 환경

- Windows (콘솔 앱)
- ANSI 색상 지원 터미널 권장 (Windows Terminal 등)
- Visual Studio 2022 (v17.x), 플랫폼: x64 / x86

### 빌드

Visual Studio 2022에서 `Tower-of-Omens/Tower-of-Omens.slnx` 열고 빌드 후 실행.

---

## 게임 개요

10층 구조의 탑을 오르며 몬스터를 처치하고 보상을 획득, 최종 보스를 쓰러뜨리는 턴제 로그라이크 RPG.

### 직업

| 직업 | 특징 |
|------|------|
| 전사 | 높은 HP·방어력, 근접 공격 중심 |
| 마법사 | 높은 마력, 스킬 의존형 |

### 층 구조

| 층 | 이벤트 |
|----|--------|
| 1–9층 | 몬스터 전투 → 보상 선택 → 정비 |
| 10층 | 보스 전투 |

### 전투 흐름

1. 선제 판정 (선제 여부 결정)
2. 플레이어 턴 — 공격 / 스킬 / 아이템 / 도주
3. 적 턴 — 일반 공격 / 특수 패턴
4. 승패 판정 → 보상 또는 게임 오버

### 보상 시스템

- 전투 승리 후 보상 카드 3장 중 1장 선택
- 보상 종류: 아이템, 장비, 스탯 강화, 스킬

### 아이템

| 분류 | 예시 |
|------|------|
| 소비 | 회복 포션, 마나 포션 |
| 장비 | 무기, 방어구 (전사/마법사 전용) |
| 스킬 | 직업별 액티브·패시브 |
