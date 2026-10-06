#include "Game.h"

int main()
{
    // シングルトン Game を実行
    Game::Instance().Run();
    return 0;
}
