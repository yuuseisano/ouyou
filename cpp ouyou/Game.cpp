#include "Game.h"
#include "DataTable.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include <string>
#include <sstream>
#include <limits>
#include <random>
#include <vector>

namespace
{
    // 入力を安全に整数へ変換し、範囲チェックするヘルパー
    int ReadIntInRange(const std::string& prompt, int minVal, int maxVal)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line))
            {
                // 入力ストリームが閉じられた場合は最小値を返す
                return minVal;
            }

            std::istringstream iss(line);
            int v;
            if ((iss >> v) && v >= minVal && v <= maxVal)
            {
                return v;
            }
            std::cout << "無効な入力です。 " << minVal << "?" << maxVal << " の数字を入力してください。\n";
        }
    }

    // ランダム生成用ユーティリティ（重み付き）
    std::mt19937& GetRng()
    {
        static std::mt19937 rng(static_cast<unsigned int>(std::random_device{}()));
        return rng;
    }

    // ランダムで敵を生成して返す (重み付き: Slime:50, Goblin:30, Skeleton:15, Dragon:5)
    std::vector<std::shared_ptr<Actor>> SpawnRandomEnemies(int count)
    {
        const std::vector<std::string> types = { "Slime", "Goblin", "Skeleton", "Dragon" };
        const std::vector<int> weights = { 50, 30, 15, 5 };
        std::discrete_distribution<int> dist(weights.begin(), weights.end());

        std::vector<std::shared_ptr<Actor>> result;
        result.reserve(count);
        for (int i = 0; i < count; ++i)
        {
            int idx = dist(GetRng());
            result.push_back(ActorFactory::Create(types[idx]));
        }
        return result;
    }
}

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
    // プレイヤー生成
    auto player = ActorFactory::Create("Player");
    AddActor(player);

    // 敵をランダムに2体スポーン
    auto initialEnemies = SpawnRandomEnemies(2);
    for (auto& e : initialEnemies)
    {
        AddActor(e);
        std::cout << e->GetName() << " が現れた！\n";
    }

    std::cout << "=== コンソールRPG 開始 ===\n";
    std::cout << "プレイヤー操作：攻撃・ステータス確認・待機・終了が選べます。\n";

    int turn = 1;
    bool quitRequested = false;
    while (true)
    {
        std::cout << "\n--- ターン " << turn << " ---\n";

        // ループ中に新しいアクターが追加されても、そのターンの処理は追加前のアクターのみ行う
        size_t actorsToProcess = mActors.size();

        for (size_t i = 0; i < actorsToProcess; ++i)
        {
            // 安全のため、リムーブされたりして空になっている場合のチェック
            if (i >= mActors.size()) break;

            auto& actor = mActors[i];
            if (actor->IsDead()) continue;

            if (actor->GetType() == "Player")
            {
                // プレイヤー入力メニュー（敵追加は削除）
                std::cout << "\nあなたの番: " << actor->GetName() << " (HP: " << actor->GetHp() << ")\n";
                std::cout << "1) 攻撃  2) ステータス表示  3) 待機  4) 終了\n";
                int choice = ReadIntInRange("選択> ", 1, 4);

                if (choice == 1)
                {
                    // 攻撃対象を列挙
                    std::vector<std::shared_ptr<Actor>> enemies;
                    for (auto& a : mActors)
                    {
                        if (a->GetType() != "Player" && !a->IsDead())
                            enemies.push_back(a);
                    }

                    if (enemies.empty())
                    {
                        std::cout << "攻撃対象がいません。\n";
                    }
                    else
                    {
                        std::cout << "攻撃対象：\n";
                        for (size_t idx = 0; idx < enemies.size(); ++idx)
                        {
                            std::cout << (idx + 1) << ") " << enemies[idx]->GetName() << " (HP: " << enemies[idx]->GetHp() << ")\n";
                        }
                        int t = ReadIntInRange("対象番号> ", 1, static_cast<int>(enemies.size()));
                        actor->Attack(*enemies[t - 1]);
                    }
                }
                else if (choice == 2)
                {
                    std::cout << "=== ステータス ===\n";
                    for (auto& a : mActors)
                    {
                        std::cout << a->GetName() << " [" << a->GetType() << "] HP:" << a->GetHp()
                                  << " ATK:" << a->GetAttack() << " DEF:" << a->GetDefense()
                                  << (a->IsDead() ? " (倒)" : "") << "\n";
                    }
                }
                else if (choice == 3)
                {
                    std::cout << actor->GetName() << " は様子を見ている。\n";
                }
                else if (choice == 4)
                {
                    std::cout << "プレイヤーがゲーム終了を要求しました。\n";
                    quitRequested = true;
                    break;
                }
            }
            else
            {
                // 敵のAI：最も近いプレイヤーを攻撃
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

            // プレイヤーが要求した終了ならループ抜け
            if (quitRequested) break;
        }

        RemoveDeadActors();

        // 終了判定: プレイヤーが生存しているか、敵が全滅しているか、またはプレイヤー終了要求
        bool playerAlive = false;
        bool anyEnemyAlive = false;
        for (auto& a : mActors)
        {
            if (a->GetType() == "Player" && !a->IsDead()) playerAlive = true;
            if (a->GetType() != "Player" && !a->IsDead()) anyEnemyAlive = true;
        }

        if (quitRequested)
        {
            std::cout << "\nプレイヤーによりゲームを終了します。\n";
            break;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::cout << "=== ゲーム終了 ===\n";
}