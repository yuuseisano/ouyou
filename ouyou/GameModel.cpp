#include "GameModel.h"
#include "Factory.h"
#include "Item.h"
#include <algorithm>
#include <random>

void GameModel::Initialize()
{
    // 再初期化時も前の戦闘を引き継がず、主人公と敵２体で開始する。
    mActors.clear();
    auto player = ActorFactory::Create("Player");
    // 初期アイテムはゲーム開始時だけ配り、再戦時には追加しない。
    player->AddItem(std::make_shared<Item>(ItemType::HP_Potion, "HPポーション", 30));
    player->AddItem(std::make_shared<Item>(ItemType::MP_Potion, "MPポーション", 20));
    AddActor(player);
    SpawnEnemies(2);
}

void GameModel::AddActor(std::shared_ptr<Actor> actor)
{
    if (actor) mActors.push_back(actor);
}

void GameModel::RemoveDeadActors()
{
    // 死亡した要素を末尾にまとめてから、一括で削除する。
    mActors.erase(std::remove_if(mActors.begin(), mActors.end(),
        [](const std::shared_ptr<Actor>& actor) { return actor->IsDead(); }), mActors.end());
}

std::vector<std::shared_ptr<Actor>> GameModel::SpawnEnemies(int count)
{
    static std::mt19937 rng(std::random_device{}());
    const char* types[] = { "Slime", "Goblin", "Skeleton", "Dragon" };
    // スライム、ゴブリン、スケルトン、ドラゴンの順に出現比率を指定する。
    std::discrete_distribution<int> distribution({ 50, 30, 15, 5 });
    std::vector<std::shared_ptr<Actor>> enemies;
    for (int i = 0; i < count; ++i)
    {
        auto enemy = ActorFactory::Create(types[distribution(rng)]);
        const auto player = GetPlayer();
        enemy->ScaleEnemyStats(player ? player->GetHeroStats()->level : 1);
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
    // 管理中の一覧は並べ替えず、生存者のコピーで行動順を決める。
    std::vector<std::shared_ptr<Actor>> order;
    for (const auto& actor : mActors)
        if (!actor->IsDead()) order.push_back(actor);
    std::sort(order.begin(), order.end(), [](const std::shared_ptr<Actor>& a, const std::shared_ptr<Actor>& b) {
        if (a->GetSpeed() != b->GetSpeed()) return a->GetSpeed() > b->GetSpeed();
        // 素早さが等しい場合は、名前で順序を決める。
        return a->GetName() < b->GetName();
    });
    return order;
}

void GameModel::UpdateEnemyDifficulty()
{
    const auto player = GetPlayer();
    if (!player || player->IsDead()) return;
    for (const auto& enemy : GetEnemies())
        enemy->ScaleEnemyStats(player->GetHeroStats()->level);
}
