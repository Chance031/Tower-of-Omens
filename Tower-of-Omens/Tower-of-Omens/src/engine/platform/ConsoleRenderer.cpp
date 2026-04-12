#include "engine/platform/ConsoleRenderer.h"

#include <Windows.h>

#include <iostream>
#include <sstream>

namespace
{
constexpr unsigned long kEnableVirtualTerminalProcessing = 0x0004;
}

ConsoleRenderer::ConsoleRenderer()
    : m_stdoutHandle(INVALID_HANDLE_VALUE),
      m_originalMode(0),
      m_isInitialized(false)
{
}

ConsoleRenderer::~ConsoleRenderer()
{
    Shutdown();
}

bool ConsoleRenderer::Initialize()
{
    if (m_isInitialized)
    {
        return true;
    }

    m_stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (m_stdoutHandle == INVALID_HANDLE_VALUE || m_stdoutHandle == nullptr)
    {
        return false;
    }

    unsigned long mode = 0;
    if (!GetConsoleMode(static_cast<HANDLE>(m_stdoutHandle), &mode))
    {
        return false;
    }

    m_originalMode = mode;
    mode |= kEnableVirtualTerminalProcessing;
    if (!SetConsoleMode(static_cast<HANDLE>(m_stdoutHandle), mode))
    {
        return false;
    }

    std::cout << "\x1B[?1049h\x1B[2J\x1B[H\x1B[?25l";
    std::cout.flush();
    m_isInitialized = true;
    return true;
}

void ConsoleRenderer::Shutdown()
{
    if (!m_isInitialized)
    {
        return;
    }

    std::cout << "\x1B[?25h\x1B[?1049l";
    std::cout.flush();
    SetConsoleMode(static_cast<HANDLE>(m_stdoutHandle), m_originalMode);
    m_isInitialized = false;
}

void ConsoleRenderer::Present(const std::string& frame) const
{
    std::cout << "\x1B[2J\x1B[H\x1B[?25l" << frame;
    std::cout.flush();
}

std::string ConsoleRenderer::ComposeMenuFrame(
    const std::string& title,
    const std::string& body,
    const std::vector<std::string>& options,
    int selectedIndex) const
{
    std::ostringstream frame;
    frame << '[' << title << "]\n";
    if (!body.empty())
    {
        frame << body;
        if (body.back() != '\n')
        {
            frame << '\n';
        }
        frame << '\n';
    }

    for (std::size_t i = 0; i < options.size(); ++i)
    {
        frame << ((static_cast<int>(i) == selectedIndex) ? "> " : "  ");
        frame << options[i] << '\n';
    }

    return frame.str();
}
