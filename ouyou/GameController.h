#pragma once
#include "GameModel.h"
#include "GameView.h"

// プレイヤーの操作と敵の行動を振り分け、ターンと戦闘の進行を制御する。
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
