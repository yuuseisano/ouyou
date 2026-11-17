#pragma once
#include "GameModel.h"

// 既存の起動方法を保ち、モデル・ビュー・コントローラーを組み立てる窓口。
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