#pragma once
#include "GameModel.h"

// Application facade: keeps the existing entry point compatible.
class Game
{
public:
    static Game& Instance();
    void Run();
    void AddActor(std::shared_ptr<Actor> actor);
    void RemoveDeadActors();
private:
    Game() = default;
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    GameModel mModel;
};