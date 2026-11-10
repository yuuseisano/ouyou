#pragma once
#include "Actor.h"
#include <memory>
#include <vector>

// Model: owns the actors and rules; has no console dependency.
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
