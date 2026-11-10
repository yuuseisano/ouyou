#include "GameModel.h"
#include "Factory.h"
#include <algorithm>
#include <random>

void GameModel::Initialize()
{
    mActors.clear();
    AddActor(ActorFactory::Create("Player"));
    SpawnEnemies(2);
}

void GameModel::AddActor(std::shared_ptr<Actor> actor)
{
    if (actor) mActors.push_back(actor);
}

void GameModel::RemoveDeadActors()
{
    mActors.erase(std::remove_if(mActors.begin(), mActors.end(),
        [](const std::shared_ptr<Actor>& actor) { return actor->IsDead(); }), mActors.end());
}

std::vector<std::shared_ptr<Actor>> GameModel::SpawnEnemies(int count)
{
    static std::mt19937 rng(std::random_device{}());
    const char* types[] = { "Slime", "Goblin", "Skeleton", "Dragon" };
    std::discrete_distribution<int> distribution({ 50, 30, 15, 5 });
    std::vector<std::shared_ptr<Actor>> enemies;
    for (int i = 0; i < count; ++i)
    {
        auto enemy = ActorFactory::Create(types[distribution(rng)]);
        AddActor(enemy);
        enemies.push_back(enemy);
    }
    return enemies;
}

std::vector<std::shared_ptr<Actor>> GameModel::GetEnemies() const
{
    std::vector<std::shared_ptr<Actor>> enemies;
    for (const auto& actor : mActors)
        if (actor->GetType() != "Player" && !actor->IsDead()) enemies.push_back(actor);
    return enemies;
}

std::shared_ptr<Actor> GameModel::GetPlayer() const
{
    for (const auto& actor : mActors)
        if (actor->GetType() == "Player") return actor;
    return nullptr;
}

std::vector<std::shared_ptr<Actor>> GameModel::GetTurnOrder() const
{
    std::vector<std::shared_ptr<Actor>> order;
    for (const auto& actor : mActors)
        if (!actor->IsDead()) order.push_back(actor);
    std::sort(order.begin(), order.end(), [](const std::shared_ptr<Actor>& a, const std::shared_ptr<Actor>& b) {
        if (a->GetSpeed() != b->GetSpeed()) return a->GetSpeed() > b->GetSpeed();
        return a->GetName() < b->GetName();
    });
    return order;
}
