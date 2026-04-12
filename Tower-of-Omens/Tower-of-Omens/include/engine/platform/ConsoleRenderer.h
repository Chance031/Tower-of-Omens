#pragma once

#include <string>
#include <vector>

// 콘솔 모드 설정과 프레임 출력만 담당하는 최소 렌더러다.
class ConsoleRenderer
{
public:
    ConsoleRenderer();
    ~ConsoleRenderer();

    bool Initialize();
    void Shutdown();
    void Present(const std::string& frame) const;
    std::string ComposeMenuFrame(
        const std::string& title,
        const std::string& body,
        const std::vector<std::string>& options,
        int selectedIndex) const;

private:
    void* m_stdoutHandle;
    unsigned long m_originalMode;
    bool m_isInitialized;
    mutable int m_lastRenderedLineCount;
};
