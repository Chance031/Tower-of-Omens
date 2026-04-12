#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"

#include <optional>

// 직업 선택 화면의 선택 결과를 돌려준다.
class JobSelectScreen
{
public:
    std::optional<JobClass> Run(const ConsoleRenderer& renderer, const MenuInput& input) const;
};
