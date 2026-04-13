#include "engine/platform/ConsoleRenderer.h"

#include <Windows.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
constexpr DWORD kEnableVirtualTerminalProcessing = 0x0004;

std::vector<std::string> SplitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        lines.push_back(line);
    }

    if (!text.empty() && text.back() == '\n')
    {
        lines.push_back("");
    }

    if (lines.empty())
    {
        lines.push_back("");
    }

    return lines;
}
}

struct ConsoleRenderer::Impl
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    DWORD originalMode = 0;
    bool isInitialized = false;
    int lastLineCount = 0;
};

// 콘솔 렌더러를 기본 상태로 생성한다.
ConsoleRenderer::ConsoleRenderer()
    : m_impl(std::make_unique<Impl>())
{
}

// 렌더러가 사용한 콘솔 상태를 정리한다.
ConsoleRenderer::~ConsoleRenderer()
{
    Shutdown();
}

// ANSI 제어 시퀀스를 사용할 수 있도록 콘솔을 초기화한다.
bool ConsoleRenderer::Initialize()
{
    if (m_impl->isInitialized)
    {
        return true;
    }

    m_impl->handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (m_impl->handle == INVALID_HANDLE_VALUE || m_impl->handle == nullptr)
    {
        return false;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(m_impl->handle, &mode))
    {
        return false;
    }

    m_impl->originalMode = mode;
    mode |= kEnableVirtualTerminalProcessing;
    if (!SetConsoleMode(m_impl->handle, mode))
    {
        return false;
    }

    std::cout << "\x1B[?1049h\x1B[2J\x1B[H\x1B[?25l";
    std::cout.flush();
    m_impl->isInitialized = true;
    return true;
}

// 초기화 과정에서 바꾼 콘솔 상태를 원래대로 되돌린다.
void ConsoleRenderer::Shutdown()
{
    if (!m_impl->isInitialized)
    {
        return;
    }

    std::cout << "\x1B[?25h\x1B[?1049l";
    std::cout.flush();
    SetConsoleMode(m_impl->handle, m_impl->originalMode);
    m_impl->isInitialized = false;
    m_impl->lastLineCount = 0;
}

// 전달받은 프레임 문자열을 현재 콘솔 화면에 출력한다.
void ConsoleRenderer::Present(const std::string& frame) const
{
    const std::vector<std::string> lines = SplitLines(frame);

    std::cout << "\x1B[H\x1B[?25l";

    for (int i = 0; i < static_cast<int>(lines.size()); ++i)
    {
        std::cout << "\x1B[2K" << lines[i];
        if (i + 1 < static_cast<int>(lines.size()))
        {
            std::cout << '\n';
        }
    }

    for (int i = static_cast<int>(lines.size()); i < m_impl->lastLineCount; ++i)
    {
        std::cout << "\n\x1B[2K";
    }

    std::cout << "\x1B[H";
    std::cout.flush();
    m_impl->lastLineCount = static_cast<int>(lines.size());
}

// 선택 메뉴 화면을 한 프레임 문자열로 조합한다.
std::string ConsoleRenderer::ComposeMenuFrame(
    const std::string& title,
    const std::string& body,
    const std::vector<std::string>& options,
    int selectedIndex) const
{
    const std::vector<std::string> bodyLines = SplitLines(body);
    std::ostringstream frame;

    frame << "============================================================\n";
    frame << "TOWER OF OMENS :: " << title << '\n';
    frame << "------------------------------------------------------------\n";

    for (const std::string& line : bodyLines)
    {
        frame << line << '\n';
    }

    if (!options.empty())
    {
        frame << "------------------------------------------------------------\n";
        for (std::size_t i = 0; i < options.size(); ++i)
        {
            frame << ((static_cast<int>(i) == selectedIndex) ? "> " : "  ");
            frame << options[i] << '\n';
        }
    }

    frame << "============================================================\n";
    return frame.str();
}
