#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include "Actor.h"
#include "Factory.h"

class Game
{
public:
    // シングルトン取得
    static Game& Instance();

    // ゲームループ
    void Run();

    // アクター管理
    void AddActor(std::shared_ptr<Actor> actor);
    void RemoveDeadActors();

private:
    Game(); // private ctor
    ~Game() = default;

    // 非コピー/非代入
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    std::vector<std::shared_ptr<Actor>> mActors;
};

