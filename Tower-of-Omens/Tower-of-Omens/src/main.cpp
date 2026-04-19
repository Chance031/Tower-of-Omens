#include "game/Game.h"

// 게임 전체를 초기화하고 메인 루프를 실행한다.
int main()
{
    Game game;
    game.Initialize();
    game.Run();
    return 0;
}
