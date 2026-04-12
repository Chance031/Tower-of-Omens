#pragma once

#include <string>

// 콘솔 모드 설정과 프레임 출력만 담당하는 최소 렌더러다.
class ConsoleRenderer
{
public:
    ConsoleRenderer();
    ~ConsoleRenderer();

    bool Initialize();
    void Shutdown();
    void Present(const std::string& frame) const;

private:
    void* m_stdoutHandle;
    unsigned long m_originalMode;
    bool m_isInitialized;
};
