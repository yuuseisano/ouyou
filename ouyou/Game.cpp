#include "Game.h"
#include "DataTable.h"
#include <chrono>
#include <thread>

Game& Game::Instance()
{
    static Game instance;
    return instance;
}

Game::Game()
{
    // 初期データテーブルは Factory が使うためここでは何もしない
}

void Game::AddActor(std::shared_ptr<Actor> actor)
{
    mActors.push_back(actor);
}

void Game::RemoveDeadActors()
{
    mActors.erase(
        std::remove_if(mActors.begin(), mActors.end(),
                       [](const std::shared_ptr<Actor>& a) { return a->IsDead(); }),
        mActors.end());
}

void Game::Run()
{
    // サンプルの生成: プレイヤーと2体の敵
    auto player = ActorFactory::Create("Player");
    auto enemy1 = ActorFactory::Create("Slime");
    auto enemy2 = ActorFactory::Create("Goblin");

    AddActor(player);
    AddActor(enemy1);
    AddActor(enemy2);

    std::cout << "=== コンソールRPG 開始 ===\n";

    int turn = 1;
    while (true)
    {
        std::cout << "\n--- ターン " << turn << " ---\n";

        // 各アクターのターン処        for (auto& actor : mActors)
        for (size_t i = 0; i < mActors.size(); ++i)
        {
            auto& actor = mActors[i];
            if (actor->IsDead()) continue;

            // シンプルな AI / 控えの行動: プレイヤーは最初の敵を攻撃、敵はプレイヤーを攻撃
            if (actor->GetType() == "Player")
            {
                // 対象を探す
                std::shared_ptr<Actor> target = nullptr;
                for (auto& a : mActors)
                {
                    if (a != actor && !a->IsDead()) { target = a; break; }
                }
                if (target)
                {
                    actor->Attack(*target);
                }
            }
            else
            {
                // 敵はプレイヤーを攻撃
                std::shared_ptr<Actor> playerTarget = nullptr;
                for (auto& a : mActors)
                {
                    if (a->GetType() == "Player" && !a->IsDead()) { playerTarget = a; break; }
                }
                if (playerTarget)
                {
                    actor->Attack(*playerTarget);
                }
            }

            // 各ターンごとに状態更新
            actor->Update();
        }

        RemoveDeadActors();

        // 終了判定: プレイヤーが生存しているか、敵が全滅しているか
        bool playerAlive = false;
        bool anyEnemyAlive = false;
        for (auto& a : mActors)
        {
            if (a->GetType() == "Player" && !a->IsDead()) playerAlive = true;
            if (a->GetType() != "Player" && !a->IsDead()) anyEnemyAlive = true;
        }

        if (!playerAlive)
        {
            std::cout << "\nあなたは敗北しました...\n";
            break;
        }
        if (!anyEnemyAlive)
        {
            std::cout << "\n敵を全滅させました！勝利！\n";
            break;
        }

        ++turn;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "=== ゲーム終了 ===\n";
}
