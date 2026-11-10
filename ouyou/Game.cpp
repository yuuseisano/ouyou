#include "Game.h"
#include "GameController.h"
#include "GameView.h"
#include <iostream>

Game& Game::Instance()
{
    static Game instance;
    return instance;
}

void Game::Run()
{
    GameView view(std::cin, std::cout);
    GameController controller(mModel, view);
    controller.Run();
}

void Game::AddActor(std::shared_ptr<Actor> actor) { mModel.AddActor(actor); }
void Game::RemoveDeadActors() { mModel.RemoveDeadActors(); }