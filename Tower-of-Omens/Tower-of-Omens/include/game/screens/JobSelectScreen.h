#pragma once

#include "engine/platform/ConsoleRenderer.h"
#include "engine/platform/MenuInput.h"
#include "game/Enums.h"

#include <optional>

// 직업 선택 화면에서 플레이어의 직업을 선택한다.
class JobSelectScreen
{
public:
    std::optional<JobClass> Run(const ConsoleRenderer& renderer, const MenuInput& input) const;
};
