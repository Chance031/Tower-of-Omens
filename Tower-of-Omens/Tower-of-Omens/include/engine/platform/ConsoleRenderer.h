#pragma once

#include <memory>
#include <string>
#include <vector>

// 콘솔 렌더링과 메뉴 UI 출력을 담당하는 클래스다.
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
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
