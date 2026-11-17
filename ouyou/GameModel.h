#pragma once
#include "Actor.h"
#include <memory>
#include <vector>

// アクターとゲームのルールを管理する。コンソールへの入出力は行わない。
class GameModel
{
public:
    void Initialize();
    void AddActor(std::shared_ptr<Actor> actor);
    void RemoveDeadActors();
    std::vector<std::shared_ptr<Actor>> SpawnEnemies(int count);
    std::vector<std::shared_ptr<Actor>> GetEnemies() const;
    std::vector<std::shared_ptr<Actor>> GetTurnOrder() const;
    std::shared_ptr<Actor> GetPlayer() const;
    const std::vector<std::shared_ptr<Actor>>& GetActors() const { return mActors; }
private:
    std::vector<std::shared_ptr<Actor>> mActors;
};
