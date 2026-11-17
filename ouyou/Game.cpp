#include "Game.h"
#include "GameController.h"
#include "GameView.h"
#include <iostream>

Game& Game::Instance()
{
    // 初回に生成したゲームを、以後の呼び出しでも共有する。
    static Game instance;
    return instance;
}

void Game::Run()
{
    // 標準入出力を使うビューと、モデルを操作するコントローラーを接続する。
    GameView view(std::cin, std::cout);
    GameController controller(mModel, view);
    controller.Run();
}

void Game::AddActor(std::shared_ptr<Actor> actor) { mModel.AddActor(actor); }
void Game::RemoveDeadActors() { mModel.RemoveDeadActors(); }