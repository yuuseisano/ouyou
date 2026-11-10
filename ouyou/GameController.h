#pragma once
#include "GameModel.h"
#include "GameView.h"

// Controller: coordinates player commands, enemy actions, and turns.
class GameController
{
public:
    GameController(GameModel& model, GameView& view) : mModel(model), mView(view) {}
    void Run();
private:
    bool HandlePlayer(Actor& actor);
    void Attack(Actor& attacker, Actor& target);
    GameModel& mModel;
    GameView& mView;
};
